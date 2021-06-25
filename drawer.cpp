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
        
    gStyle->SetPalette(55);
    gStyle->SetOptTitle(0) ;
    gStyle->SetOptStat(1000000) ;
    gStyle->SetOptFit(0) ;

    gStyle->SetPadTopMargin(    0.03 ) ;
    gStyle->SetPadRightMargin(  0.15 ) ;
    gStyle->SetPadBottomMargin( 0.09 ) ;
    gStyle->SetPadLeftMargin(   0.09 ) ;

    gStyle->SetTitleOffset( 1.3 , "x" ) ;
    gStyle->SetTitleOffset( 0.7 , "y" ) ;
    gStyle->SetTitleOffset( 2.0 , "z" ) ;
    
    double text_size = 0.05 ;
    int font = 42 ;

    gStyle->SetLabelFont(font,"x");
    gStyle->SetTitleFont(font,"x");
    gStyle->SetLabelFont(font,"y");
    gStyle->SetTitleFont(font,"y");
    gStyle->SetLabelFont(font,"z");
    gStyle->SetTitleFont(font,"z");

    gStyle->SetLabelSize(text_size,"x") ;
    gStyle->SetTitleSize(text_size,"x") ;
    gStyle->SetLabelSize(text_size,"y") ;
    gStyle->SetTitleSize(text_size,"y") ;
    gStyle->SetLabelSize(text_size,"z") ;
    gStyle->SetTitleSize(text_size,"z") ;
    
    TApplication app("app", &argc, argv) ; 
    TCanvas * can = new TCanvas( histname , histname , 700 , 600 ) ;
    
    hist->Draw("COLZ") ;
        
    gPad->Modified() ;
    gPad->Update() ;
    gPad->WaitPrimitive() ;

    hist->Delete() ;

    return 0 ;

}
