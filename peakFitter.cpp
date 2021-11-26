#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ) return 1 ;

    TString filename = argv[1] ;

    bool drawFits  = true ;
    bool printFits = false ;
    if( argc > 2 ){ 
        string option = argv[2] ;
        if( option.compare("skip") == 0 ) drawFits = false ; 
        else printFits = true ;
    }
    
    double fwhmFactor = 2. * sqrt( 2. * log( 2. ) ) ;
    
    TString ARGUSstring = "[H]**3/sqrt(2*pi)" ;
    ARGUSstring += "/" ;
    ARGUSstring += "(" ;
    ARGUSstring +=      "0.5*(1.+tanh(sqrt(2/pi)*[H]))" ;
    ARGUSstring +=      "-[H]/sqrt(2*pi)*exp(-[H]**2/2)" ;
    ARGUSstring +=      "-0.5" ;
    ARGUSstring += ")" ;
    ARGUSstring += "*x/[C]**2*sqrt(1.-(x/[C])**2)" ;
    ARGUSstring += "*exp(-0.5*[H]**2*(1.-(x/[C])**2))" ;

    vector< vector<string> > parameter = getInput( filename.Data() );

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };
                            
    vector< vector<string> > filesNhists ;
    vector<SpecifiedNumber> plotPosition ;
    vector< vector<double> > peakNrange ;
    vector<string> fitAddFunctions ;
    vector< vector<SpecifiedNumber> > fitStartParameter ;

    vector<string> strVecDummy ;
    vector<double> doVecDummy ;
    vector<SpecifiedNumber> specVecDummy ;

    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//            cout << " comment line " << r << endl ;
            continue ;
        }

        int fileORhist = -1 ;

        if( parameter.at(r).at(0).compare("FILE") == 0  ) fileORhist = 0 ;
        else if( parameter.at(r).at(0).compare("HIST") == 0  ) fileORhist = 1 ;

        if( fileORhist > -1 && parameter.at(r).size() > 2 ){
            preNsuffix[fileORhist][0] = parameter.at(r).at(1) ;
            preNsuffix[fileORhist][1] = parameter.at(r).at(2) ;
            continue ;
        }

        if( parameter.at(r).size() > 5 ){
            strVecDummy.push_back( parameter.at(r).at(0) );
            strVecDummy.push_back( parameter.at(r).at(1) );
            filesNhists.push_back( strVecDummy ) ;
            strVecDummy.clear() ;
            if( parameter.at(r).at(2).compare( "%" ) != 0 )
                plotPosition.push_back( 
                    SpecifiedNumber(
                        atof( parameter.at(r).at(2).c_str() )
                    )
                ) ;
            else
                plotPosition.push_back( SpecifiedNumber() ) ;
            doVecDummy.push_back( atof( parameter.at(r).at(3).c_str() ) );
            doVecDummy.push_back( atof( parameter.at(r).at(4).c_str() ) );
            doVecDummy.push_back( atof( parameter.at(r).at(5).c_str() ) );
            peakNrange.push_back( doVecDummy ) ;
            doVecDummy.clear() ;
            if( parameter.at(r).size() > 6 ){
                fitAddFunctions.push_back( parameter.at(r).at(6).c_str() ) ;
                for(unsigned int c=7; c<parameter.at(r).size(); c++){
                    if( parameter.at(r).at(c).compare( "%" ) != 0 )
                        specVecDummy.push_back( 
                            SpecifiedNumber(
                                atof( parameter.at(r).at(c).c_str() )
                            )
                        ) ;
                    else
                        specVecDummy.push_back( SpecifiedNumber() ) ;
                }
                fitStartParameter.push_back( specVecDummy ) ;
                specVecDummy.clear() ;
            }
            else{ 
                fitAddFunctions.push_back("") ;
                fitStartParameter.push_back( specVecDummy ) ;
            }
        }

    }

    for(unsigned int t=0; t<2; t++){
        for(unsigned int f=0; f<2; f++){
            if( 
                preNsuffix[t][f].compare( neverUse ) == 0 
                ||
                preNsuffix[t][f].compare( "%" ) == 0
            )
                preNsuffix[t][f] = "" ;
        }
    }

    for(unsigned int r=0; r<filesNhists.size(); r++){
        for(unsigned int c=0; c<2; c++){
            if( filesNhists.at(r).at(c).compare( "%" ) == 0 )
                filesNhists.at(r).at(c) = "" ;
        }
    }

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

    unsigned int nHists = filesNhists.size() ;
    TH1D** hists = new TH1D*[nHists] ;
    TString name , title ;
    unsigned int nBins = 0 ;
    double range[2] = { 0. , 0. } ;
    unsigned int notFound = 0 ;
    bool useable[nHists] ;
    
    for(unsigned int r=0; r<nHists; r++){

        name = preNsuffix[0][0] ;
        name += filesNhists.at(r).at(0) ;
        name += preNsuffix[0][1] ;
        TFile * input = new TFile(name,"READ") ;
        if( input->IsZombie() ){
            cout << " ERROR : opening " << name << endl ;
            notFound++ ;
            useable[r] = false ;
            continue ;
        }

        name = preNsuffix[1][0] ;
        name += filesNhists.at(r).at(1) ;
        name += preNsuffix[1][1] ;
        if( input->Get(name) == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            notFound++ ;
            useable[r] = false ;
            continue ;
        }
        hists[r] = (TH1D*)input->Get(name) ;

        hists[r]->SetDirectory(0) ;
        input->Close() ;

        if( nBins == 0 ){
            nBins = hists[r]->GetNbinsX() ;
            range[0] = hists[r]->GetXaxis()->GetXmin() ;
            range[1] = hists[r]->GetXaxis()->GetXmax() ;
        }
        else if( 
            nBins != hists[r]->GetNbinsX() 
            ||
            range[0] != hists[r]->GetXaxis()->GetXmin()
            ||
            range[1] != hists[r]->GetXaxis()->GetXmax()
        ){
            notFound++ ;
            useable[r] = false ;
            continue ;
        }

        useable[r] = true ;

    }

    if( 
        nBins == 0 
        || 
        (
            range[0] == 0.
            &&
            range[1] == 0.
        )
    ){
        cout << " ERROR : empty histograms " << endl ;
        return 1 ;
    }

    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
    TGraphErrors * g_width = new TGraphErrors() ;
    g_width->SetName("width") ;
    g_width->SetTitle("width") ;
    
    TGraphErrors * g_fwhm = new TGraphErrors() ;
    g_fwhm->SetName("fwhm") ;
    g_fwhm->SetTitle("fwhm") ;
    
    TGraphErrors * g_centerDifference = new TGraphErrors() ;
    g_centerDifference->SetName("centerDifference") ;
    g_centerDifference->SetTitle("centerDifference") ;
    
    TF1 * fitfunction ;
    double fwhm , halfLow , halfHigh ;
    TLine * lineLow , * lineHigh ;
    
    TApplication app("app", &argc, argv) ; 
    
    for(unsigned int h=0; h<nHists; h++){
        
        if( !useable[h] ) continue ;
        
        name = "histogram" ;
        name += h ;
        hists[h]->SetName(name) ;
        hists[h]->SetTitle(name) ;
        
        hists[h]->GetXaxis()->SetRangeUser( 
            peakNrange.at(h).at(1) , peakNrange.at(h).at(2) 
        ) ;
        
        double maximum = hists[h]->GetMaximum() ;
        double width = peakNrange.at(h).at(2) - peakNrange.at(h).at(1) ;
        
        name = "fitfunction" ;
        name += h ;
        title = "gaus(0)" ;
        vector<unsigned int> argusPositions ;
        vector<unsigned int> argusIndices ;
        if( ! fitAddFunctions.at(h).empty() ){
            TString toAdd = fitAddFunctions.at(h) ;
            if( toAdd.Contains("ARGUS") ){
                unsigned int endigs = toAdd.CountChar('S') ;
                TString tester = toAdd ;
                unsigned int removedChars = 0 ;
                for(unsigned int e=0; e<endigs; e++){
                    if( tester.Length() == 0 ) break ;
                    int firstS = tester.First('S') ;
                    if( firstS < 0 ) break ;
                    TString argusPart = tester( tester.First('S')-4 , 5 ) ;
                    if( argusPart.EqualTo("ARGUS") ){
                        TString indexString 
                                    = tester( tester.First('S')+1 , 1 ) ;
                        if( indexString.IsDec() ){
                            argusPositions.push_back( firstS+removedChars ) ;
                            argusIndices.push_back(
                                atoi( indexString.Data() )
                            ) ;
                        }
                        else{
                            cout << " ERROR : no index found for ARGUS " 
                                 << "=> EXIT" << endl ;
                            return 5 ;
                        }
                    }
                    removedChars += tester.First('S')+1 ;
                    tester = tester( tester.First('S')+1 , tester.Sizeof() ) ;
                }
                for(unsigned int a=0; a<argusPositions.size(); a++){
                    unsigned int posS = toAdd.Sizeof() 
                                        - fitAddFunctions.at(h).length() 
                                        + argusPositions.at(a) ;
                    toAdd = toAdd( 0 , posS ) 
                            + toAdd( posS+1 , toAdd.Sizeof() ) ;
                    toAdd = toAdd.Replace( 
                                            posS - 5 , 5 , 
                                            ARGUSstring , 
                                            ARGUSstring.Length() 
                                         ) ;
                    TString replacement = "" ; 
                    replacement += argusIndices.at(a) ;
                    toAdd = toAdd.ReplaceAll( "H" , replacement ) ;
                    replacement = "" ;
                    replacement += (argusIndices.at(a)+1) ;
                    toAdd = toAdd.ReplaceAll( "C" , replacement ) ;
                    
                }
//                 cout << " toAdd " << toAdd << endl ;
            }
            title += toAdd ;
        }
        
        fitfunction = new TF1( 
                                name , title , 
                                peakNrange.at(h).at(1) , 
                                peakNrange.at(h).at(2) 
                             ) ;

        hists[h]->SetTitle(title) ;
                             
        fitfunction->SetParameter( 0 , 0.9 * maximum ) ;
        fitfunction->SetParameter( 1 , peakNrange.at(h).at(0) ) ;
        fitfunction->SetParameter( 2 , width ) ;
        
        fitfunction->SetParLimits( 0 , 0.3 * maximum , 1.5 * maximum ) ;
        fitfunction->SetParLimits( 1 , 
                                        peakNrange.at(h).at(0)
                                        - 0.5 * width 
                                        , 
                                        peakNrange.at(h).at(0)
                                        + 0.5 * width 
                                 ) ;
        fitfunction->SetParLimits( 2 , 
                                        0.001 * width , 
                                        2.   * width  
                                 ) ;
                                 
        for(unsigned int p=0; p<fitStartParameter.at(h).size(); p++){
            if( fitStartParameter.at(h).at(p).setting ){
                fitfunction->SetParameter( 3 + p , 
                    fitStartParameter.at(h).at(p).number
                ) ;
                for(unsigned int a=0; a<argusIndices.size(); a++){
                    if( argusIndices.at(a)+1 == 3+p ){
                        if( argusIndices.at(a) > 3 ){
                            fitfunction->SetParameter( 
                                                        argusIndices.at(a)-1 , 
                                                        maximum 
                                                     ) ;
                            fitfunction->SetParLimits(
                                                        argusIndices.at(a)-1 ,
                                                        0.1 * maximum ,
                                                        hists[h]->GetEntries()
                                                    ) ;
                        }
                        fitfunction->SetParameter( argusIndices.at(a) , 1. ) ;
                        fitfunction->SetParLimits( 
                                                    argusIndices.at(a) , 
                                                    0. , 10. 
                                                 ) ;
                        fitfunction->FixParameter( 3+p , 
                            fitStartParameter.at(h).at(p).number
                        ) ;
                    }
                }
            }
        }
        
        hists[h]->Sumw2() ;
        hists[h]->Fit( fitfunction , "RQB WL" ) ;
        
        unsigned int count = 1 ;
        double chi2ndf = 
                            fitfunction->GetChisquare()
                            / fitfunction->GetNDF() ;
        
        while(
            count < 10
            &&
            chi2ndf > 2.
        ){
            hists[h]->Fit( fitfunction , "RQB WL" ) ;
            chi2ndf = 
                        fitfunction->GetChisquare()
                        / fitfunction->GetNDF() ;
            count++ ;
        }
        
        cout 
             << " " 
             << preNsuffix[0][0] 
             << filesNhists.at(h).at(0) 
             << preNsuffix[0][1]
             << endl << " " 
             << preNsuffix[1][0]
             << filesNhists.at(h).at(1)
             << preNsuffix[1][1]
             << endl
             << " \t DISTRIBUTION : "
             << " entries "     << hists[h]->GetEntries()
             << " - reference " << peakNrange.at(h).at(0)
             << " - steps "     << count
             << " - chi2ndf "   << chi2ndf 
             << endl ;
             
        cout << " \t FIT : " ;
        for(unsigned int p=0; p<fitfunction->GetNpar(); p++){
            cout << " " << fitfunction->GetParameter(p) ;
        }
        cout << endl;
        
        fwhm = getFWHM(
                        hists[h] ,
                        fitfunction->GetParameter( 0 ) ,
                        peakNrange.at(h).at(0) ,
                        maximum - fitfunction->GetParameter( 0 ) ,
                        halfLow ,
                        halfHigh
                    ) ;
        cout << " \t FWHM : "  << fwhm
             << " - maximum "  << maximum 
             << " - halfLow "  << halfLow 
             << " - halfHigh " << halfHigh
             << endl ;

        if( drawFits ){

            hists[h]->Draw() ;
            gPad->SetGridx() ;
            gPad->SetGridy() ;
            gPad->Modified() ;
            gPad->Update() ;
            
            double 
                    ymin = gPad->GetUymin() , 
                    ymax = gPad->GetUymax() ;
                 
            lineLow  = new TLine( halfLow  , ymin , halfLow  , ymax ) ;
            lineLow->SetLineColor(kBlue) ;
            lineLow->SetLineWidth(3) ;
            lineHigh = new TLine( halfHigh , ymin , halfHigh , ymax ) ;
            lineHigh->SetLineColor(kRed) ;
            lineHigh->SetLineWidth(3) ;
            
            lineLow->Draw() ;
            lineHigh->Draw() ;
            
            gPad->Modified() ;
            gPad->Update() ;

            if( printFits ){
                name = outfile->GetName() ;
                name = name.ReplaceAll( ".root" , "_" ) ;
                name += hists[h]->GetName() ;
                name += ".pdf" ;
                gPad->GetCanvas()->Print( name ) ;
            }
            else gPad->WaitPrimitive() ;

        }

        hists[h]->SetTitle( hists[h]->GetName() ) ;

        if( chi2ndf > 10. ){ 
            outfile->cd() ;
            hists[h]->Write() ;
            fitfunction->Write() ;
            continue ;
        }
        
        double position = plotPosition.at(h).number ;
        double posError = 0 ;
        if( ! plotPosition.at(h).setting ){
            position = fitfunction->GetParameter( 1 ) ;
            posError = fitfunction->GetParError( 1 ) ;
        }
        
        g_width->SetPoint(
            g_width->GetN() ,
            position ,
            abs( 
                fitfunction->GetParameter( 2 ) 
                * fwhmFactor 
            )
        ) ;
        
        g_width->SetPointError(
            g_width->GetN()-1 ,
            posError ,
            fitfunction->GetParError( 2 ) * fwhmFactor
        ) ;
        
        g_fwhm->SetPoint(
            g_fwhm->GetN() ,
            position ,
            fwhm
        );
        
        g_fwhm->SetPointError(
            g_fwhm->GetN()-1 ,
            posError ,
            hists[h]->GetBinWidth(1) 
        );
        
        if( ! plotPosition.at(h).setting )
            position = peakNrange.at(h).at(0) ;
        
        g_centerDifference->SetPoint(
            g_centerDifference->GetN() ,
            position ,
            fitfunction->GetParameter( 1 )
             - peakNrange.at(h).at(0) 
        ) ;
        
        g_centerDifference->SetPointError(
            g_centerDifference->GetN()-1 ,
            posError ,
            fitfunction->GetParError( 1 ) 
        ) ;
    
        outfile->cd() ;
        hists[h]->Write() ;
        fitfunction->Write() ;
        
    }

    cout << " writing ... " ;
    
    outfile->cd() ;

    g_width->Write() ;
    g_fwhm->Write() ;
    g_centerDifference->Write() ;
    
    outfile->Write() ;
    outfile->Close() ;

    cout << " done " << endl ;

    return 0 ;

}
