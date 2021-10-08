#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ){ 
        cout << " usage : " << argv[0] << endl ;
        cout << " append either single text-file-name " << endl ;
        cout << " or : <file1> <graph1> <file2> <graph2> " << endl ;
        return 1 ;
    }
    
    plotOptions() ;

    TString filename = argv[1] ;
    TString filesNgraphs[2][2] ;
    TString outname ;
    TString name ;

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };

    unsigned int count = 0 ;
    
    if( argc > 4 && filename.EndsWith(".root") ){
        filesNgraphs[0][0] = argv[1] ;
        filesNgraphs[0][1] = argv[2] ;
        filesNgraphs[1][0] = argv[3] ;
        filesNgraphs[1][1] = argv[4] ;
        outname = filesNgraphs[0][0] ;
        outname += "_" ;
        outname += filesNgraphs[0][1] ;
        outname += "_VS_" ;
        outname += filesNgraphs[1][0] ;
        outname += "_" ;
        outname = filesNgraphs[1][1] ;
        outname.ReplaceAll( ".root" , "" ) ;
        outname += ".root" ;
    }
    else{
        
        vector< vector<string> > parameter = getInput( filename.Data() ) ;
        
        for(unsigned int r=0; r<parameter.size(); r++){

            if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//                 cout << " comment line " << r << endl ;
                continue ;
            }

            int specifier = -1 ;

            if( parameter.at(r).at(0).compare("FILE") == 0  ) 
                specifier = 0 ;
            else if( parameter.at(r).at(0).compare("GRAPH") == 0  ) 
                specifier = 1 ;

            if( specifier > -1 && parameter.at(r).size() > 2 ){
                if( specifier < 2 ){
                    preNsuffix[specifier][0] = parameter.at(r).at(1) ;
                    preNsuffix[specifier][1] = parameter.at(r).at(2) ;
                }
                continue ;
            }
            
            if( parameter.at(r).size() < 2 ) continue ;
            
            if( count > 1 ) continue ;
            
            filesNgraphs[count][0] = parameter.at(r).at(0) ;
            filesNgraphs[count][1] = parameter.at(r).at(1) ;
            
            count++ ;
            
        }
        
        if( count != 2 ){
            cout << " ERROR : parameter-file not in correct format " << endl ;
            return 3 ;
        }
        
        outname = filename ;
        if( outname.Contains(".") ) 
            outname = outname( 0 , outname.Last('.') ) ;
        outname += ".root" ;
        if( outname.Contains("/") ) 
            outname = outname( outname.Last('/')+1 , outname.Sizeof() ) ;
        
    }

    for(unsigned int t=0; t<2; t++){
        for(unsigned int f=0; f<2; f++){
            if( 
                preNsuffix[t][f].compare( neverUse ) == 0 
                ||
                preNsuffix[t][f].compare( "%" ) == 0
            )
                preNsuffix[t][f] = "" ;
            if( filesNgraphs[t][f] == "%" )
                filesNgraphs[t][f] = "" ;
        }
    }
    
    count = 0 ; 
    
    TGraphErrors ** graphs = new TGraphErrors*[2] ;
    
    for(unsigned int g=0; g<2; g++){

        name = preNsuffix[0][0] ;
        name += filesNgraphs[g][0] ;
        name += preNsuffix[0][1] ;
        TFile * input = new TFile(name,"READ") ;
        if( input->IsZombie() ){
            cout << " ERROR : opening " << input->GetName() << endl ;
            return 4 ;
        }

        name = preNsuffix[1][0] ;
        name += filesNgraphs[g][1] ;
        name += preNsuffix[1][1] ;
        if( input->Get(name) == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            return 5 ;
        }
        
        graphs[g] = (TGraphErrors*)input->Get( name ) ;
        if( graphs[g] == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            return 6 ;
        }
        
        input->Close() ;
        
        count++ ;
    
    }
        
    if( count != 2 ){
        cout << " ERROR : not enough graphs found " << endl ;
        return 7 ;
    }
    
    count = graphs[0]->GetN() ;
        
    if( count != graphs[1]->GetN() ){
        cout << " ERROR : graphs not of same size " << endl ;
        return 8 ;
    }
    
    TFile * outfile = new TFile( outname , "RECREATE" ) ;
    
    outfile->cd() ;
    
    TGraphErrors * relativeDifferences = new TGraphErrors() ;
    relativeDifferences->SetName("relativeDifferences") ;
    relativeDifferences->SetTitle("relativeDifferences") ;
    TGraphErrors * absoluteDifferences = new TGraphErrors() ;
    absoluteDifferences->SetName("absoluteDifferences") ;
    absoluteDifferences->SetTitle("absoluteDifferences") ;
    
    double x[2] , y[2] ;
    unsigned int foundBad = 0 ;
    
    for(unsigned int p=0; p<count; p++){
        
        for(unsigned int g=0; g<2; g++)
            graphs[g]->GetPoint( p , x[g] , y[g] ) ;
        
        if( ! almostEqual( x[0] , x[1] ) ){
            foundBad++ ;
            continue ;
        }
        
        absoluteDifferences->SetPoint(
            absoluteDifferences->GetN() , x[0] , y[0] - y[1]
        ) ;
        
        if( y[1] == 0. ) continue ;
        
        relativeDifferences->SetPoint(
            relativeDifferences->GetN() , x[0] , ( y[0] - y[1] ) / y[1]
        ) ;
        
    }
    
    cout << " # points : " << count << " -> # bad " << foundBad  << endl ;
    
    relativeDifferences->Write() ;
    absoluteDifferences->Write() ;
    
    outfile->Close() ;

    return 0 ;
    
}