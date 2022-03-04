#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 3 ){ 
        cout << " ERROR : filename and histname required " << endl ;
        return 1 ;
    }

    TString filename = argv[1] ;
    TString histname = argv[2] ;
    
    vector<unsigned int> specifiedBins ;
    bool slice[2] = { true , true } ;
    
    bool show = true ;
    
    if( argc > 3 ){
        TString tester = argv[3] ;
        tester.ToUpper() ;
        if(      tester == "X" ) slice[1] = false ;
        else if( tester == "Y" ) slice[0] = false ;
        if( argc > 4 ){
            for(unsigned int a=4; a<argc; a++){
                int b = atoi( argv[a] ) ;
                if( b > 0 ) specifiedBins.push_back( b ) ;
            }
        }
    }
    
    TFile * input = new TFile(filename,"READ") ;
    if( input->IsZombie() ){
        cout << " ERROR : opening " << filename << endl ;
        return 2 ;
    }
    
    if( input->Get(histname) == NULL ){
        cout << " ERROR : reading " << histname 
                << " in " << input->GetName() << endl ;
        return 3 ;
    }
    TH2D * hist = (TH2D*)input->Get(histname) ;
    hist->SetDirectory(0) ;
    input->Close() ;

    TString name = filename ;
    if( name.Contains(".") ) 
        name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) 
        name = name( name.Last('/')+1 , name.Sizeof() ) ;
    name += "_" ;
    name += histname ;
    name = replaceBadChars( name );
    name += "_slice" ;
    if( specifiedBins.size() < 1 ) name += "s" ;
    if(      slice[0] && !( slice[1] ) ) name += "X" ;
    else if( slice[1] && !( slice[0] ) ) name += "Y" ;
    if( specifiedBins.size() == 1 ) name += specifiedBins.at(0) ;
    name += ".root" ;
    
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
    unsigned int nBins[2] = { 
        (unsigned int)hist->GetNbinsX() ,
        (unsigned int)hist->GetNbinsY() 
    } ;
    
    outfile->cd();
    TH1D * projection ;
    for(unsigned int c=0; c<2; c++){
        if( !slice[(c+1)%2] ) continue ;
        if( c == 0 ) name = "Y" ;
        else         name = "X" ;
        if( specifiedBins.size() > 0 ){
            unsigned int bin ;
            for(unsigned int b=0; b<specifiedBins.size(); b++){
                bin = specifiedBins.at(b) ;
                if( bin > nBins[c] ) continue ;
                name = name( 0 , 1 ) ;
                name += bin ;
                if( c == 0 ) projection = hist->ProjectionX( name, bin, bin ) ;
                else         projection = hist->ProjectionY( name, bin, bin ) ;
                if( projection != NULL ) projection->Write() ;
            }
        }
        else{
            for(unsigned int b=1; b<nBins[c]; b++){
                name = name( 0 , 1 ) ;
                name += b ;
                if( c == 0 ) projection = hist->ProjectionX( name , b , b ) ;
                else         projection = hist->ProjectionY( name , b , b ) ;
                if( projection != NULL ) projection->Write() ;
            }
        }
    }

    hist->Delete() ;
    outfile->Close() ;

    return 0 ;

}
