#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 3 ){ 
        cout << " ERROR : filename and histname required " << endl ;
        return 1 ;
    }

    TString filename = argv[1] ;
    TString histname = argv[2] ;
    
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
                     
    plotOptions() ;
    
    gStyle->SetOptStat(1000000) ;

    gStyle->SetPadTopMargin(    0.03 ) ;
    gStyle->SetPadRightMargin(  0.15 ) ;
    gStyle->SetPadBottomMargin( 0.09 ) ;
    gStyle->SetPadLeftMargin(   0.09 ) ;

    gStyle->SetTitleOffset( 1.3 , "x" ) ;
    gStyle->SetTitleOffset( 0.7 , "y" ) ;
    gStyle->SetTitleOffset( 2.0 , "z" ) ;
    
    TApplication app("app", &argc, argv) ; 
    TCanvas * can = new TCanvas( histname , histname , 700 , 600 ) ;
    
    hist->Draw("COLZ") ;
        
    showing() ;

    hist->Delete() ;

    return 0 ;

}
