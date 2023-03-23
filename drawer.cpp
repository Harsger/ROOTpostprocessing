#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 3 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

    TString filename = argv[1] ;
    TString histname = argv[2] ;
    
    bool skipShowing = false ;
    
    vector<string> parameter = {
        "lowLimit" ,
        "highLimit" ,
        "nContours" ,
        "colorPalette" ,
        "statBox" 
    } ;
    map< string , SpecifiedNumber > values ;
    
    for(unsigned int p=0; p<parameter.size(); p++){
        values[parameter.at(p)] = SpecifiedNumber() ;
        if( argc > p+3 ){
            string argument = argv[p+3] ; 
            if( argument == "%" ) continue ;
            if( argument == "skip" ) skipShowing = true ;
            else 
                values[parameter.at(p)] = 
                                SpecifiedNumber( atof( argument.c_str() ) ) ;
        }
    }
    if( 
        argc > parameter.size()+3 
        && 
        string( argv[parameter.size()+3] ) == "skip" 
    )
        skipShowing = true ;
                  
    TApplication app("app", &argc, argv) ;    
    plotOptions( false , true ) ;
    
    if( values["colorPalette"].setting ){
        if( values["colorPalette"].number < 0 ){
            gStyle->SetPalette( -values["colorPalette"].number ) ;
            TColor::InvertPalette() ;
        }
        else gStyle->SetPalette( values["colorPalette"].number ) ;
    }
    
    gStyle->SetOptStat(0) ;
    if( values["statBox"].setting ) 
        gStyle->SetOptStat( values["statBox"].number ) ;
        
    gStyle->SetPadRightMargin(  0.18 ) ;
    
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
    
    if( values["lowLimit"].setting ) 
        minimum = values["lowLimit"].number ;
    if( values["highLimit"].setting ) 
        maximum = values["highLimit"].number ;
    
    if( values["lowLimit"].setting || values["highLimit"].setting )
        hist->GetZaxis()->SetRangeUser( minimum , maximum ) ;
    
    if( values["nContours"].setting ){
        gStyle->SetNumberContours( 
            (unsigned int)( values["nContours"].number ) 
        ) ; 
        hist->GetZaxis()->SetNdivisions(520) ;
    }
    
    hist->Draw("COLZ") ;
        
    if( !( skipShowing ) ) showing() ;
    
    name += ".pdf" ;
        
    can->Print( name ) ;
    
    can->Close() ;

    hist->Delete() ;

    return 0 ;

}
