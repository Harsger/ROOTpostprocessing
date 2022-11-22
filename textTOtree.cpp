#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){
    
    if( argc < 2 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

    TString filename = argv[1] ;

    vector< vector<string> > parameter = getInput( filename.Data() );
    
    string neverUse = "neverUseThisPhrase" ;
    string directorySpecifierEnding[3] = {
        neverUse , neverUse , neverUse
    };
    
    map< unsigned int , vector<TString> > columnTOspecifier ;
    
    unsigned int maxColumn = 0 ;
    
    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//            cout << " comment line " << r << endl ;
            continue ;
        }

        if( 
            parameter.at(r).at(0).compare("FILE") == 0 
            &&  
            parameter.at(r).size() > 3
        ){ 
            directorySpecifierEnding[0] = parameter.at(r).at(1) ;
            directorySpecifierEnding[1] = parameter.at(r).at(2) ;
            directorySpecifierEnding[2] = parameter.at(r).at(3) ;
            continue ;
        }
        
        if( parameter.at(r).size() > 2 ){
            
            TString indexString = parameter.at(r).at(0) ;
            if( ! indexString.IsDec() ) continue ;
            
            unsigned int columnIndex = atoi( indexString.Data() ) ;
            
            if( 
                columnTOspecifier.find( columnIndex )
                != 
                columnTOspecifier.end() 
            ){
                cout << " ERROR : multiple usage of column-index " 
                     << columnIndex 
                     << " => EXIT " << endl ;
                return 2 ;
            }
            
            columnTOspecifier[ columnIndex ] = {
                parameter.at(r).at(1) ,
                parameter.at(r).at(2)
            } ;
            
            if( columnIndex+1 > maxColumn ) 
                maxColumn = columnIndex+1 ;
            
        }

    }
    
    if( maxColumn < 1 || columnTOspecifier.empty() ){
        cout << " ERROR : no column-specification found => EXIT " << endl ;
        return 3 ;
    } 
    
    vector<TString> dataFileNames ;
    
    TSystemDirectory directory( 
                                directorySpecifierEnding[0].c_str() , 
                                directorySpecifierEnding[0].c_str() 
                            ); 
    TList * fileList = directory.GetListOfFiles(); 
    
    if( !fileList ){
        cout << " ERROR : directory \"" 
             << directorySpecifierEnding[0] 
             << "\" not found => EXIT " << endl ;
        return 4 ;
    }
    
    TSystemFile * readfile ; 
    TIter next( fileList ) ;
    TString readname ;
    
    while( ( readfile = (TSystemFile*)next() ) ){ 
        
        readname = readfile->GetName() ; 
        
        if( 
            !( readname.Contains( directorySpecifierEnding[1].c_str() ) ) ||
            !( readname.EndsWith( directorySpecifierEnding[2].c_str() ) )
        ) continue;
        
        dataFileNames.push_back( readname ) ;
        
    }
    
    if( dataFileNames.size() < 1 ){
        cout << " ERROR : no files found in \""
                << filename << "\" => EXIT " << endl ;
        return 5 ;
    }

    TString name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
    if( outfile == NULL ){
        cout << " ERROR : can not create output \"" 
             <<  name << "\" => EXIT " << endl ;
        return 6 ;
    }
    
    TTree * data ;
    
    map< unsigned int , int >     leaf_int ;
    map< unsigned int , double >  leaf_double ;
    map< unsigned int , TString > leaf_string ;
    
    data = new TTree( "data" , "data" );
    
    for( auto c : columnTOspecifier ){
        
        if( c.second.at(1).EqualTo("int") ){
            leaf_int[c.first] = 0 ;
            data->Branch( c.second.at(0) , &leaf_int[c.first] ) ;
        }
        else if( c.second.at(1).EqualTo("double") ){
            leaf_double[c.first] = 0. ;
            data->Branch( c.second.at(0) , &leaf_double[c.first] ) ;
        }
        else if( c.second.at(1).EqualTo("string") ){
            leaf_string[c.first] = "" ;
            data->Branch( c.second.at(0) , &leaf_string[c.first] ) ;
        }
        
    }
    
    unsigned int nFiles = dataFileNames.size() ;
    
    for(unsigned int f=0; f<nFiles; f++){
        
        readname = directorySpecifierEnding[0] ;
        if( ! readname.EndsWith("/") ) readname += "/" ;
        readname += dataFileNames.at(f) ;
        
        cout << " " << f+1 << "/" << nFiles << " : " << dataFileNames.at(f) ;
        cout << flush ;
        
        vector< vector<string> > textData 
            = getInput( readname.Data() ) ;
            
        unsigned int rows = textData.size() ;
        
        cout << " => # rows : " << rows << endl ;
        
        for(unsigned int r=0; r<rows; r++){
            
            if( textData.at(r).size() < maxColumn ) continue;
            
            for( auto &i : leaf_int ){
                i.second = atoi( textData.at(r).at(i.first).c_str() ) ;
            }
            
            for( auto &d : leaf_double ){
                d.second = atof( textData.at(r).at(d.first).c_str() ) ;
            }
            
            for( auto &s : leaf_string ){
                s.second = textData.at(r).at(s.first) ;
            }
            
            data->Fill();
            
        }
        
    }
    
    outfile->cd();
    
    cout << " writing ... " ;
    
    data->Write();
    
    cout << " closing ... " ;
    
    outfile->Close();
    
    cout << " done " << endl ;
    
    return 0 ;
    
}
