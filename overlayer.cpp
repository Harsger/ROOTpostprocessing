#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ) return 1 ;

    TString filename = argv[1] ;

    vector< vector<string> > parameter = getInput( filename.Data() );

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };

    vector< vector<string> > filesNhists ;
    vector<string> strVecDummy ;
    vector<string> spectrumName ;
    string axisTitles[2] = { neverUse , neverUse } ;
    SpecifiedNumber plotRange[2][2] ;
    bool useLogScale[2] = { false , false } ;
    SpecifiedNumber drawPoints ;
    
    TString scaleMode = "integral" ;
    
    SpecifiedNumber colorPalette ;
    SpecifiedNumber statBox ;

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
        
        if( 
            parameter.at(r).at(0).compare("AXIS") == 0 
            &&
            parameter.at(r).size() > 2
        ){
            axisTitles[0] = parameter.at(r).at(1) ;
            axisTitles[1] = parameter.at(r).at(2) ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("RANGE") == 0 
            &&
            parameter.at(r).size() > 4
        ){
            if( parameter.at(r).at(1).compare("%") != 0 ){
                plotRange[0][0]
                    = SpecifiedNumber( atof( parameter.at(r).at(1).c_str() ) ) ;
                plotRange[0][0].specifier = "xlow" ;
            }
            if( parameter.at(r).at(2).compare("%") != 0 ){
                plotRange[0][1] 
                    = SpecifiedNumber( atof( parameter.at(r).at(2).c_str() ) ) ;
                plotRange[0][1].specifier = "xhigh" ;
            }
            if( parameter.at(r).at(3).compare("%") != 0 ){
                plotRange[1][0] 
                    = SpecifiedNumber( atof( parameter.at(r).at(3).c_str() ) ) ;
                plotRange[1][0].specifier = "ylow" ;
            }
            if( parameter.at(r).at(4).compare("%") != 0 ){
                plotRange[1][1] 
                    = SpecifiedNumber( atof( parameter.at(r).at(4).c_str() ) ) ;
                plotRange[1][1].specifier = "yhigh" ;
            }
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("LOG") == 0 
            &&
            parameter.at(r).size() > 2
        ){
            if( parameter.at(r).at(1).compare("1") == 0 ) 
                useLogScale[0] = true ;
            if( parameter.at(r).at(2).compare("1") == 0 )
                useLogScale[1] = true ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("SCALEMODE") == 0 
            &&
            parameter.at(r).size() > 1
        ){
            scaleMode = parameter.at(r).at(1) ;
            continue ;
        }
        
        if( parameter.at(r).at(0).compare("POINTS") == 0 ){
            if( parameter.at(r).size() > 1 )
                drawPoints = SpecifiedNumber( atoi( 
                                                parameter.at(r).at(1).c_str() 
                                            ) ) ;
            else
                drawPoints = SpecifiedNumber( 6 ) ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("PALETTE") == 0 
            &&
            parameter.at(r).size() > 1
        ){
            if( TString(parameter.at(r).at(1)).IsDec() )
                colorPalette = 
                    SpecifiedNumber( atoi( parameter.at(r).at(1).c_str() ) ) ;
            else 
                colorPalette = SpecifiedNumber( 55 ) ; // kRainBow
            if( 
                parameter.at(r).size() > 2 
                && 
                parameter.at(r).at(2).compare("inverted") == 0
            )
                colorPalette.specifier = "inverted" ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("STATBOX") == 0 
            &&
            parameter.at(r).size() > 1
        ){
            if( TString(parameter.at(r).at(1)).IsDec() ){
                statBox = 
                    SpecifiedNumber( atoi( parameter.at(r).at(1).c_str() ) ) ;
                statBox.specifier = parameter.at(r).at(1).c_str() ;
            }
            else{ 
                statBox = SpecifiedNumber( 1111 ) ;
                statBox.specifier = "1111" ;
            }
            continue ;
        }

        if( parameter.at(r).size() > 2 ){
            strVecDummy.push_back( parameter.at(r).at(0) );
            strVecDummy.push_back( parameter.at(r).at(1) );
            filesNhists.push_back( strVecDummy ) ;
            strVecDummy.clear() ;
            spectrumName.push_back( parameter.at(r).at(2) );
        }

    }

    unsigned int nHists = filesNhists.size() ;
    
    if( nHists < 1 ){
        cout << " ERROR : no histograms specified " << endl ;
        return 2 ;
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

    for(unsigned int r=0; r<nHists; r++){
        for(unsigned int c=0; c<2; c++){
            if( filesNhists.at(r).at(c).compare( "%" ) == 0 )
                filesNhists.at(r).at(c) = "" ;
        }
    }
    
    TApplication app("app", &argc, argv) ; 
    
    plotOptions() ;
    
    if( colorPalette.setting ){
        gStyle->SetPalette( colorPalette.number ) ;
        if( colorPalette.specifier.compare("inverted") == 0 )
            TColor::InvertPalette();
    }
        
//     gStyle->SetOptStat(0) ;
    if( statBox.setting ) gStyle->SetOptStat( statBox.number ) ;
    else gStyle->SetOptStat(0) ;

    gStyle->SetPadTopMargin(    0.03 ) ;
    gStyle->SetPadRightMargin(  0.11 ) ;
    gStyle->SetPadBottomMargin( 0.12 ) ;
    gStyle->SetPadLeftMargin(   0.06 ) ;

    gStyle->SetTitleOffset( 1.2 , "x" ) ;
    gStyle->SetTitleOffset( 0.6 , "y" ) ;
    
    TH1D** hists = new TH1D*[nHists] ;
    TString name ;
    unsigned int nBins = 0 ;
    double valueRange[2][2] = { { 0. , 0. } , { 0. , 0. } } ;
    unsigned int notFound = 0 ;
    bool useable[nHists] ;
    
    for(unsigned int h=0; h<nHists; h++){

        name = preNsuffix[0][0] ;
        name += filesNhists.at(h).at(0) ;
        name += preNsuffix[0][1] ;
        TFile * input = new TFile(name,"READ") ;
        if( input->IsZombie() ){
            cout << " ERROR : opening " << name << endl ;
            notFound++ ;
            useable[h] = false ;
            continue ;
        }

        name = preNsuffix[1][0] ;
        name += filesNhists.at(h).at(1) ;
        name += preNsuffix[1][1] ;
        if( input->Get(name) == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            notFound++ ;
            useable[h] = false ;
            continue ;
        }
        hists[h] = (TH1D*)input->Get(name) ;

        hists[h]->SetDirectory(0) ;
        input->Close() ;
        
        hists[h]->SetName( spectrumName.at(h).c_str() ) ;
        hists[h]->SetTitle( spectrumName.at(h).c_str() ) ;

        if( nBins == 0 ){
            nBins = hists[h]->GetNbinsX() ;
            valueRange[0][0] = hists[h]->GetXaxis()->GetXmin() ;
            valueRange[0][1] = hists[h]->GetXaxis()->GetXmax() ;
            valueRange[1][0] = hists[h]->GetMinimum() ;
            valueRange[1][1] = hists[h]->GetMaximum() ;
        }
        else if( 
            nBins != hists[h]->GetNbinsX() 
            ||
            valueRange[0][0] != hists[h]->GetXaxis()->GetXmin()
            ||
            valueRange[0][1] != hists[h]->GetXaxis()->GetXmax()
        ){
            notFound++ ;
            useable[h] = false ;
            continue ;
        }
        
        if( valueRange[1][0] > hists[h]->GetMinimum() )
            valueRange[1][0] = hists[h]->GetMinimum() ;
        if( valueRange[1][1] < hists[h]->GetMaximum() )
            valueRange[1][1] = hists[h]->GetMaximum() ;

        useable[h] = true ;

    }
  
    if( notFound == nHists ){
        cout << " ERROR : no spectra available " << endl ;
        return 3 ;
    }

    if( 
        nBins == 0 
        || 
        (
            valueRange[0][0] == 0.
            &&
            valueRange[0][1] == 0.
        )
        || 
        (
            valueRange[1][0] == 0.
            &&
            valueRange[1][1] == 0.
        )
    ){
        cout << " ERROR : empty histograms " << endl ;
        return 4 ;
    }

    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
    bool resetRange = true ;
    
    for(unsigned int h=0; h<nHists; h++){
            
        if( !useable[h] ) continue ;
        
        if( 
            scaleMode.Contains("ranged")
            && 
            plotRange[0][0].setting 
            &&
            plotRange[0][1].setting
        ){
            hists[h]->GetXaxis()->SetRangeUser( 
                plotRange[0][0].number , plotRange[0][1].number
            );
        }
            
        double scaleFactor = 1. ;
        
        if( scaleMode.Contains("max") )
            scaleFactor = 1. / (double)( hists[h]->GetMaximum() ) ;
        else if( scaleMode.Contains("bare") )
            scaleFactor = 1. ;
        else if( 
            scaleMode.Contains("integral") 
            ||
            scaleMode == "ranged"
        )
            scaleFactor = 1. / (double)( hists[h]->Integral() ) ;
            
        TH1D * swapHist = hists[h] ;
        
        hists[h] = new TH1D( 
                            spectrumName.at(h).c_str() , 
                            spectrumName.at(h).c_str() ,
                            nBins , valueRange[0][0] , valueRange[0][1]
        );
        
        for(unsigned int b=0; b<=nBins+1; b++){
            hists[h]->SetBinContent( b , 
                (double)(swapHist->GetBinContent( b ) ) * scaleFactor
            ) ;
        }
        
        if( 
            plotRange[0][0].setting 
            &&
            plotRange[0][1].setting
        ){
            hists[h]->GetXaxis()->SetRangeUser( 
                plotRange[0][0].number , plotRange[0][1].number
            );
        }
        
        if( resetRange ){
            valueRange[1][0] = hists[h]->GetMinimum() ;
            valueRange[1][1] = hists[h]->GetMaximum() ;
            resetRange = false ;
        }
        else{
            if( valueRange[1][0] > hists[h]->GetMinimum() )
                valueRange[1][0] = hists[h]->GetMinimum() ;
            if( valueRange[1][1] < hists[h]->GetMaximum() )
                valueRange[1][1] = hists[h]->GetMaximum() ;
        }
        
    }
    
    TCanvas * can = new TCanvas( "overlay" , "overlay" , 2000 , 500 ) ; 
    if(useLogScale[0] && !( plotRange[0][0].number <= 0. ) ) can->SetLogx() ; 
    if(useLogScale[1] && !( plotRange[1][0].number <= 0. ) ) can->SetLogy() ; 

    for(unsigned int r=0; r<2; r++){
        for(unsigned int c=0; c<2; c++){
            if( ! plotRange[r][c].setting )
                plotRange[r][c].number = valueRange[r][c] ;
        }
    }
    
    bool firstTOdraw = true ;
    double under , over ;
    
    for(unsigned int h=0; h<nHists; h++){
        
        if( !useable[h] ) continue ;
        
        if( drawPoints.setting ) hists[h]->SetMarkerStyle(drawPoints.number) ;
        else hists[h]->SetMarkerStyle(6) ;
        hists[h]->SetLineWidth(3) ;  
        
        getOutflow( 
                    hists[h] , 
                    plotRange[0][0].number , 
                    plotRange[0][1].number , 
                    under , 
                    over 
                  ) ; 
            
        hists[h]->GetXaxis()->SetRangeUser( 
            plotRange[0][0].number , plotRange[0][1].number
        );
        
        hists[h]->GetYaxis()->SetRangeUser( 
            plotRange[1][0].number , plotRange[1][1].number
        );
        
        hists[h]->SetBinContent( 0       , under ) ;
        hists[h]->SetBinContent( nBins+1 , over  ) ;
        
        if( firstTOdraw ){ 
            
            if( drawPoints.setting ) hists[h]->Draw("P PLC PMC") ;
            else  hists[h]->Draw("PLC PMC") ; 
            
            if( axisTitles[0].compare(neverUse) != 0 )
                hists[h]->SetXTitle( axisTitles[0].c_str() ) ;
            if( axisTitles[1].compare(neverUse) != 0 )
                hists[h]->SetYTitle( axisTitles[1].c_str() ) ;
            
            firstTOdraw = false ;
            
        }
        else{ 
            name = "" ;
            if( drawPoints.setting ) name = "P" ;
            name += "same" ;
            if( statBox.setting ) name += "s" ;
            name += " PLC PMC" ;
            hists[h]->Draw(name) ;
        }
                  
    }
    
    gPad->SetGridx() ;
    gPad->SetGridy() ;
    
    can->BuildLegend( 0.90 , 0.15 , 0.995 , 0.95 ) ;
    
    showing() ;

    cout << " writing ... " ;
    
    outfile->cd() ;
    
    name = outfile->GetName() ;
    name = name.ReplaceAll( ".root" , ".pdf" ) ;
    can->Print(name) ; 
    can->Write() ;
    
    outfile->Write() ;
    outfile->Close() ;

    cout << " done " << endl ;

    return 0 ;

}
