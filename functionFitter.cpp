#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 6 ) return 1 ;

    TString filename       = argv[1] ;
    TString dataname       = argv[2] ;
    TString lowLimit       = argv[3] ;
    TString highLimit      = argv[4] ;
    TString functionString = argv[5] ;
    
    double fitRange[2] = {
                            atof( lowLimit.Data() ) ,
                            atof( highLimit.Data() ) 
                        } ;
    
    if( fitRange[0] == fitRange[1] ){
        cout << " ERROR : fit-range ill defined " << endl ;
        return 2 ;
    }
                        
    if( fitRange[0] > fitRange[1] )
        swap( fitRange[0] , fitRange[1] ) ;
    
                        
    vector< vector<double> > startParameter ;
    vector<double> vecdodummy ;
    double value , low , high ;
    bool draw = true ;
    bool print = false ;
    
    for(unsigned int a=6; a<argc; a++){
        if( argv[a] == "%" ) startParameter.push_back( {} ) ;
        else if( a == argc-1 && string( argv[a] ) == "skip" ){
            draw = false ;
        }
        else if( a == argc-1 && string( argv[a] ) == "print" ){
            print = true ;
        }
        else{
            value = getNumberWithRange( argv[a] , low , high ) ;
            startParameter.push_back( { value , low , high } ) ;
        }
    }
    
    gSystem->RedirectOutput( "/dev/null" ) ;
    
    TApplication app("app", &argc, argv) ; 
    plotOptions() ;

    gStyle->SetOptTitle(1) ;
    gStyle->SetTitleX(0.5) ;
    gStyle->SetTitleAlign(23) ;
    gStyle->SetOptFit( 1 ) ;
    gStyle->SetOptStat( 1001100 ) ;
    gStyle->SetStatX( 0.995 ) ;
    gStyle->SetStatY( 0.90  ) ;
    gStyle->SetStatW( 0.115 );
    gStyle->SetStatH( 0.30  );

    gStyle->SetCanvasDefW( 1000 ) ;
    gStyle->SetCanvasDefH(  600 ) ;
   
    gStyle->SetPadTopMargin(    0.10 ) ; 
    gStyle->SetPadRightMargin(  0.22 ) ;
    gStyle->SetPadBottomMargin( 0.11 ) ;
    gStyle->SetPadLeftMargin(   0.11 ) ;

    gStyle->SetTitleOffset( 1.0 , "x" ) ; 
    gStyle->SetTitleOffset( 1.2 , "y" ) ;
    
    TFile * input = new TFile(filename,"READ") ;
    if( input->IsZombie() ){
        cout << " ERROR : opening " << input->GetName() << endl ;
        return 2 ;
    }
    
    if( input->Get(dataname) == NULL ){
        cout << " ERROR : reading " << dataname 
                          << " in " << input->GetName() << endl ;
        return 3 ;
    }
    
    TObject * data = input->Get(dataname) ;
    TString dataClass = data->ClassName() ;
    
    TF1 * function = new TF1( 
                                "function" , 
                                functionString , 
                                atof( lowLimit.Data() ) ,
                                atof( highLimit.Data() ) 
                            ) ;
    function->SetLineColor( 2 ) ;
    unsigned int nParameter = function->GetNpar() ;
                            
    for(unsigned int p=0; p<startParameter.size(); p++){
        if( p >= nParameter ) break ;
        if( startParameter.at(p).size() < 3 ) continue ;
        if( toDiscard( startParameter.at(p).at(0) ) ) continue ;
        else if( 
            toDiscard( startParameter.at(p).at(1) )
            ||
            toDiscard( startParameter.at(p).at(2) )
        ){
            function->SetParameter( p , startParameter.at(p).at(0) ) ;
        }
        else if( 
            startParameter.at(p).at(0) == startParameter.at(p).at(1) 
            &&
            startParameter.at(p).at(0) == startParameter.at(p).at(2) 
        ){
            function->FixParameter( p , startParameter.at(p).at(0) ) ;
        }
        else{
            function->SetParameter( p , startParameter.at(p).at(0) ) ;
            function->SetParLimits( 
                                    p , 
                                    startParameter.at(p).at(1) ,
                                    startParameter.at(p).at(2) 
                                  ) ;
        }
    }
    
    if( dataClass.Contains("TH1") ){ 
        ( (TH1*)data )->Sumw2() ;
        ( (TH1*)data )->Fit( function , "RQB WL" ) ;
    }
    else if( dataClass.Contains("Graph") ){ 
        ( (TGraphErrors*)data )->Fit( function , "RQB WL" ) ;
    }
    
    unsigned int count = 1 ;
    double chi2ndf = function->GetChisquare() / function->GetNDF() ;
    
    while( count < 10 && ( chi2ndf < 0.5 || chi2ndf > 2. ) ){
        if( dataClass.Contains("TH1") ) 
            ( (TH1*)data )->Fit( function , "RQB WL" ) ;
        else if( dataClass.Contains("Graph") ) 
            ( (TGraphErrors*)data )->Fit( function , "RQB WL" ) ;
        chi2ndf = function->GetChisquare() / function->GetNDF() ;
        count++ ;
    }          
    
    gSystem->RedirectOutput( 0 , 0 ) ;
    cout << " " << chi2ndf ;
    for(unsigned int p=0; p<nParameter; p++){
        cout << " " << function->GetParameter( p ) ;
        cout << " " << function->GetParError( p ) ;
    }
    cout << endl ;
    gSystem->RedirectOutput( "/dev/null" ) ;
    
    TString name = filename ;
    if( name.Contains(".") ) 
        name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) 
        name = name( name.Last('/')+1 , name.Sizeof() ) ;
    name += "_" ;
    name += dataname ;
    name = replaceBadChars( name );
    name += "_fit.root" ;
    
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    outfile->cd() ;
    function->Write() ;
    outfile->Close() ;
    
    if( draw ){

        
        if( dataClass.Contains("TH1") ){ 
            ( (TH1*)data )->SetTitle( functionString ) ;
            ( (TH1*)data )->GetXaxis()
                        ->SetRangeUser( fitRange[0] , fitRange[1] ) ;
            ( (TH1*)data )->Draw() ;
        }
        else if( dataClass.Contains("Graph") ){ 
            ( (TGraphErrors*)data )->SetTitle( functionString ) ;
            ( (TGraphErrors*)data )->GetXaxis()
                                ->SetRangeUser( fitRange[0] , fitRange[1] ) ;
            ( (TGraphErrors*)data )->Draw("AP") ;
        }
        
        gPad->SetGridx() ;
        gPad->SetGridy() ;
        gPad->Modified() ;
        gPad->Update() ;

        if( print ){
            name = outfile->GetName() ;
            name = name.ReplaceAll( ".root" , ".pdf" ) ;
            gPad->GetCanvas()->Print( name ) ;
        }
        else gPad->WaitPrimitive() ;
        
    }

    return 0 ;
    
}