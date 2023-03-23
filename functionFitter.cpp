#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 6 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

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
    bool interactionRequired = true ;
    
    for(unsigned int a=6; a<argc; a++){
        if( argv[a] == "%" ) startParameter.push_back( {} ) ;
        else if( a == argc-1 && string( argv[a] ) == "skip" ){
            draw = false ;
        }
        else if( 
            a == argc-1 
            && 
            string( argv[a] ).find("print") != std::string::npos 
        ){
            print = true ;
            if( string( argv[a] ).find("auto") != std::string::npos ) 
                interactionRequired = false ;
        }
        else{
            value = getNumberWithRange( argv[a] , low , high ) ;
            startParameter.push_back( { value , low , high } ) ;
        }
    }
    
    gSystem->RedirectOutput( "/dev/null" ) ;
    
    TApplication app("app", &argc, argv) ; 
    plotOptions( false , true ) ;

    gStyle->SetOptTitle(1) ;
    gStyle->SetTitleX(0.5) ;
    gStyle->SetTitleAlign(23) ;
    gStyle->SetOptFit( 1 ) ;
    gStyle->SetOptStat( 1001100 ) ;

    gStyle->SetCanvasDefW( 1000 ) ;
    gStyle->SetCanvasDefH(  600 ) ;
   
    gStyle->SetPadTopMargin(   0.12 ) ;
    gStyle->SetPadRightMargin( 0.30 ) ;
    gStyle->SetPadLeftMargin(  0.11 ) ;

    gStyle->SetTitleOffset( 1.2 , "y" ) ;
    
    TFile * input = new TFile(filename,"READ") ;
    if( input->IsZombie() ){
        gSystem->RedirectOutput( 0 , 0 ) ;
        cout << " ERROR : opening " << input->GetName() << endl ;
        return 2 ;
    }
    
    if( input->Get(dataname) == NULL ){
        gSystem->RedirectOutput( 0 , 0 ) ;
        cout << " ERROR : reading " << dataname 
                          << " in " << input->GetName() << endl ;
        return 3 ;
    }
    
    TObject * data = input->Get(dataname) ;
    TString dataClass = data->ClassName() ;
    
    TF1 * function = new TF1( 
                                "fittedFunction" ,
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
    if( dataClass.Contains("TH1") ){
        TH1D * hist = (TH1D*)data ;
        unsigned int nBins = hist->GetNbinsX() ;
        Double_t binEdges[ nBins + 1 ] ;
        hist->GetXaxis()->GetLowEdge( binEdges ) ;
        binEdges[ nBins ] = hist->GetXaxis()->GetXmax() ;
        TH1D * difference = new TH1D(
                                        "difference" , "difference" ,
                                        nBins , binEdges
                                    ) ;
        nBins++ ;
        double x , y , ey ;
        for(unsigned int b=1; b<nBins; b++){
            x = hist->GetXaxis()->GetBinCenter( b ) ;
            y = hist->GetBinContent( b ) ;
            difference->SetBinContent( b , y - function->Eval( x ) ) ;
            ey = hist->GetBinError( b ) ;
            if( ey > 0. ){
                difference->SetBinError(
                    b , sqrt( ey * ey + pow( function->Derivative( x ) , 2 ) )
                ) ;
            }
        }
        difference->Write() ;
    }
    else if( dataClass.Contains("Graph") ){
        TGraphErrors * graph = (TGraphErrors*)data ;
        TGraphErrors * difference = new TGraphErrors() ;
        difference->SetName(  "difference" ) ;
        difference->SetTitle( "difference" ) ;
        unsigned int nPoints = graph->GetN() ;
        double x , y  , ex , ey ;
        bool withErrors = dataClass.Contains("Errors") ;
        for(unsigned int p=0; p<nPoints; p++){
            graph->GetPoint( p , x , y ) ;
            difference->SetPoint( p , x , y - function->Eval( x ) ) ;
            if( withErrors ){
                ex = graph->GetErrorX( p ) ;
                ey = graph->GetErrorY( p ) ;
                if( ey > 0. ){
                    if( !( ex > 0. ) ) ex = 0. ;
                    difference->SetPointError(
                        p , ex ,
                        sqrt( ey * ey + pow( function->Derivative( x ) , 2 ) )
                    ) ;
                }
            }
        }
        difference->Write() ;
    }
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
            ( (TGraphErrors*)data )->SetMarkerStyle(8) ;                    
            ( (TGraphErrors*)data )->SetMarkerSize(1.5) ;                    
            ( (TGraphErrors*)data )->Draw("AP") ;
        }
        
        gPad->SetGridx() ;
        gPad->SetGridy() ;

        gPad->Modified() ;
        gPad->Update() ;

        TPaveStats * box = (TPaveStats*)data->FindObject("stats") ;

        box->SetX1NDC( 0.703 ) ;
        box->SetX2NDC( 0.997 ) ;
        box->SetY1NDC( 0.120 ) ;
        box->SetY2NDC( 0.880 ) ;

        gPad->Modified() ;
        gPad->Update() ;

        if( print ){
            if( interactionRequired ){
                gSystem->RedirectOutput( 0 , 0 ) ;
                showing() ;
            }
            name = outfile->GetName() ;
            name = name.ReplaceAll( ".root" , ".pdf" ) ;
            gPad->GetCanvas()->Print( name ) ;
        }
        else gPad->WaitPrimitive() ;
        
    }

    return 0 ;
    
}