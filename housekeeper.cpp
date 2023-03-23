#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){
    
    if( argc < 2 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

    plotOptions( true ) ;

    TString filename = argv[1] ;
    
    TString outname = filename ;
    
    TString nameDefault = "logs.root" ;
    
    TString readname ;
    
    vector<TString> dataFileNames ;
    
    if( outname.EndsWith(".dat") ){
        dataFileNames.push_back( filename ) ;
        outname = outname.ReplaceAll( ".dat" , ".root" ) ;
        if( outname.Contains("/") ){
            TString prefix = outname( 0 , outname.Last('/') ) ;
            TString suffix = outname( outname.Last('/')+1 , outname.Sizeof() ) ;
            if( prefix.EndsWith("/logs") ){
                prefix = prefix.ReplaceAll( "/logs" , "" ) ;
                prefix = prefix( prefix.Last('/')+1 , prefix.Sizeof() ) ;
                outname = prefix ;
                outname += "_" ;
                outname += suffix ;
            }
            else outname = suffix ;
        }
    }
    else{
        if( outname.EndsWith("/") )
            outname = outname( 0 , outname.Length()-1 ) ;
        if( outname.EndsWith("logs") )
            outname = outname( 0 , outname.Length()-4 ) ;
        if( outname.EndsWith("/") )
            outname = outname( 0 , outname.Length()-1 ) ;
        if( outname.Length() == 0 ) outname = nameDefault ;
        else if( outname.Contains("/") ){
            outname 
                = outname( outname.Last('/')+1 , outname.Sizeof() ) ;
            outname += ".root" ;
        }
        else outname += ".root" ;
    }
    
    if( dataFileNames.size() < 1 ){
    
        TSystemDirectory mainDir( filename , filename ); 
        TList * files = mainDir.GetListOfFiles(); 
        
        if( !files ){
            cout << " ERROR : directory \"" 
                 << filename << "\" not found => EXIT " << endl ;
            return 2 ;
        }
        
        TSystemFile * readfile ; 
        TIter next( files ) ;
        
        while( ( readfile = (TSystemFile*)next() ) ){ 
            
            readname = readfile->GetName() ; 
            
            if( 
                !( readname.BeginsWith("housekeeping_") ) ||
                !( readname.EndsWith(".dat") )
            ) continue;
            
            dataFileNames.push_back( readname ) ;
            
        }
        
        if( dataFileNames.size() < 1 ){
            cout << " ERROR : no files found in \""
                 << filename << "\" => EXIT " << endl ;
            return 3 ;
        }
        
    }
    
    TString name ;
    map<TString,TGraph*> plots ;
    map<TString,bool> setAxisTitle ;
    map<string,bool> options = {
        { "show"  , false } ,
        { "print" , false } ,
        { "write" , false }
    } ;
    
    if( argc > 2 ){
        for(unsigned int a=2; a<argc; a++){
            if( options.find( argv[a] ) != options.end() ){
                options[argv[a]] = true ;
                continue ;
            }
            plots[ argv[a] ] = new TGraph() ;
            plots[ argv[a] ]->SetName(  argv[a] ) ;
            plots[ argv[a] ]->SetTitle( argv[a] ) ;
            setAxisTitle[ argv[a] ] = true ;
        }
    }
    
    TFile * outfile ;
    TTree * data ;
    unsigned int unixtime ;
    TString      quantity ;
    TString      specifier ;
    double       value ;
    TString      unit ;
    
    if( options["write"] ){

        outfile = new TFile( outname , "RECREATE" ) ;
        if( outfile == NULL ){
            cout << " ERROR : can not create output \""
                <<  outname << "\" => EXIT " << endl ;
        }

        data = new TTree( "data" , "housekeeping" ) ;
        data->Branch( "unixtime"  , &unixtime  ) ;
        data->Branch( "quantity"  , &quantity  ) ;
        data->Branch( "specifier" , &specifier ) ;
        data->Branch( "value"     , &value     ) ;
        data->Branch( "unit"      , &unit      ) ;
    
    }
    
    unsigned int nFiles = dataFileNames.size() ;
    vector<TString> foundSpecifier ;
    TString specNquant ;
    bool fillTree = options["write"] ;
    bool fillGraphs =
        ( options["show"] || options["print"] || options["write"] )
        && plots.size() > 0 && nFiles == 1 ;
    
    for(unsigned int f=0; f<nFiles; f++){
        
        if( nFiles == 1 ) readname = dataFileNames.at(f) ;
        else{
            readname = filename ;
            if( ! readname.EndsWith("/") ) readname += "/" ;
            readname += dataFileNames.at(f) ;
            cout << " " << f+1 << "/" << nFiles ;
            cout << " : " << dataFileNames.at(f) ;
            cout << flush ;
        }
        
        vector< vector<string> > textData 
            = getInput( readname.Data() ) ;
            
        unsigned int rows = textData.size() ;
        
        cout << " => # rows : " << rows << endl ;
        
        
        for(unsigned int r=0; r<rows; r++){
            
            if( textData.at(r).size() < 5 ) continue;
            
            unixtime  = (unsigned int)( atof( textData.at(r).at(0).c_str() ) ) ;
            quantity  = textData.at(r).at(1) ;
            specifier = textData.at(r).at(2) ;
            value     = atof( textData.at(r).at(3).c_str() ) ;
            unit      = textData.at(r).at(4) ;
            
            if( fillTree ) data->Fill() ;
            
            specNquant = specifier ;
            specNquant += " " ;
            specNquant += quantity ;
            
            if( find( 
                        foundSpecifier.begin() , 
                        foundSpecifier.end() , 
                        specNquant
                    ) 
                == 
                foundSpecifier.end() 
            ){
                foundSpecifier.push_back( specNquant ) ;
            }
            
            if( fillGraphs ){

                specNquant = specNquant.ReplaceAll( " " , "" ) ;
                if( plots.find( specNquant ) == plots.end() ) continue ;
                plots[ specNquant ]->SetPoint(
                                                plots[ specNquant ]->GetN() ,
                                                unixtime ,
                                                value
                                            ) ;
                if( setAxisTitle[ specNquant ] ){
                    name = specifier ;
                    name += " " ;
                    name += quantity ;
                    name += " (" ;
                    name += unit ;
                    name += ")" ;
                    plots[ specNquant ]->GetYaxis()->SetTitle( name ) ;
                    setAxisTitle[ specNquant ] = false ;
                }
            }
            
        }
        
    }
    
    std::sort( foundSpecifier.begin() , foundSpecifier.end() ) ;
    cout << " specifier quantity " << endl ;
    for(unsigned int s=0; s<foundSpecifier.size(); s++)
        cout << " " << foundSpecifier.at(s) << endl ;
    cout << endl ;
    
    if( options["write"] ){
        cout << " writing ... " ;
        outfile->cd() ;
        data->Write() ;
        if( !fillGraphs ){
            outfile->Close() ;
            cout << " closed ... " ;
        }
    }
    
    if( fillGraphs ){
    
        TApplication app("app", &argc, argv) ; 
        
        TCanvas * can = new TCanvas( "can" , "can" , 1400 , 400 ) ;
        
        for( auto p : plots ){
            
            if( p.second->GetN() < 1 ){
                p.second->Delete() ;
                continue ;
            }

            can->SetName(  p.first ) ;
            can->SetTitle( p.first ) ;
            
            unsigned int dayStartTime = 
                (unsigned int)( 
                    TMath::MaxElement( p.second->GetN() , p.second->GetX() )
                ) / secondsPER["d"] ;
            dayStartTime *= secondsPER["d"] ;
            time_t startingTime = dayStartTime ;
            char dateArray[11] ;
            char dateFormat[] = "%d.%m.%Y" ;
            strftime(
                        dateArray , sizeof( dateArray ) ,
                        dateFormat , gmtime( &startingTime )
                    ) ;
            dayStartTime -= 7200 ;
            
            p.second->SetMarkerStyle(7) ;
            p.second->GetXaxis()->SetTimeDisplay(1) ;
            p.second->GetXaxis()->SetTimeFormat("%H:%M%F1970-01-01 00:00:00") ;
//             p.second->GetXaxis()->SetTimeOffset( 0 , "GMT" ) ;
            p.second->GetXaxis()->SetNdivisions(515);
            p.second->GetXaxis()->SetRangeUser(
                dayStartTime ,
                dayStartTime + secondsPER["d"]
            );
            p.second->GetXaxis()->SetTitle( dateArray ) ;
            
            if( options["write"] ){
                outfile->cd() ;
                p.second->Write() ;
            }

            p.second->Draw("AP") ;
            
            gPad->SetGridx() ;
            gPad->SetGridy() ;
            
            if( options["show"] ){
                if( options["print"] ) showing() ;
                else                   padWaiting() ;
            }

            if( options["print"] ){
                name = outname ;
                name = name.ReplaceAll( ".root" , "_" ) ;
                name += p.first ;
                name += ".pdf" ;
                gPad->Print( name ) ;
            }

            p.second->Delete() ;
            
        }
        
        if( options["write"] ){
            outfile->Close() ;
            cout << " closed ... " ;
        }

    }
    
    cout << " done " << endl ;
    
    return 0 ;
    
}
