#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 3 ){ 
        cout << " ERROR : filename and histname required " << endl ;
        return 1 ;
    }

    TString filename = argv[1] ;
    TString histname = argv[2] ;
    
    SpecifiedNumber lowLimit ;
    SpecifiedNumber highLimit ;
    SpecifiedNumber nContours ;
    
    bool show = true ;
    
    if( argc == 4 ) show = false ;
    else if( argc > 4 ){
        TString tester = argv[3] ;
        if( tester != "%" )
            lowLimit = SpecifiedNumber( atof( tester.Data() ) ) ;
        tester = argv[4] ;
        if( tester != "%" )
            highLimit = SpecifiedNumber( atof( tester.Data() ) ) ;
        if( argc > 5 ){ 
            tester = argv[5] ;
            if( tester.IsDec() )
                nContours = SpecifiedNumber( atoi( tester.Data() ) ) ;
            else show = false ;
            if( argc > 6 ) show = false ;
        }
    }
                  
    TApplication app("app", &argc, argv) ;    
    plotOptions() ;
    
    gStyle->SetOptStat(0) ;
        
    gStyle->SetPadRightMargin(  0.18 ) ;

    gStyle->SetTitleOffset( 1.0 , "x" ) ;
    gStyle->SetTitleOffset( 1.2 , "y" ) ;
    gStyle->SetTitleOffset( 1.4 , "z" ) ;
    
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
        
    TCanvas * can = new TCanvas( histname , histname , 700 , 600 ) ;
    
    double minimum = hist->GetMinimum() ;
    double maximum = hist->GetMaximum() ;
    
    if( lowLimit.setting ) minimum = lowLimit.number ;
    if( highLimit.setting ) maximum = highLimit.number ;
    
    if( lowLimit.setting || highLimit.setting )
        hist->GetZaxis()->SetRangeUser( minimum , maximum ) ;
    
    if( nContours.setting ) 
        gStyle->SetNumberContours( (unsigned int)( nContours.number ) ) ; 
    
    hist->Draw("COLZ") ;
        
    if( show ) showing() ;
    
    name += ".pdf" ;
        
    can->Print( name ) ;
    
    can->Close() ;

    hist->Delete() ;

    return 0 ;

}
