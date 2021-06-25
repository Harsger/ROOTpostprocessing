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
    SpecifiedNumber plotRanges[2][2] ;
    bool useLogScale[2] = { false , false } ;

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
                plotRanges[0][0].number 
                    = atof( parameter.at(r).at(1).c_str() ) ;
                plotRanges[0][0].specifier = "xlow" ;
                plotRanges[0][0].setting = true ;
            }
            if( parameter.at(r).at(2).compare("%") != 0 ){
                plotRanges[0][1].number 
                    = atof( parameter.at(r).at(2).c_str() ) ;
                plotRanges[0][1].specifier = "xhigh" ;
                plotRanges[0][1].setting = true ;
            }
            if( parameter.at(r).at(3).compare("%") != 0 ){
                plotRanges[1][0].number 
                    = atof( parameter.at(r).at(3).c_str() ) ;
                plotRanges[1][0].specifier = "ylow" ;
                plotRanges[1][0].setting = true ;
            }
            if( parameter.at(r).at(4).compare("%") != 0 ){
                plotRanges[1][1].number 
                    = atof( parameter.at(r).at(4).c_str() ) ;
                plotRanges[1][1].specifier = "yhigh" ;
                plotRanges[1][1].setting = true ;
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
    
    TH1D** hists = new TH1D*[nHists] ;
    TString name ;
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
        
        hists[r]->SetName( spectrumName.at(r).c_str() ) ;
        hists[r]->SetTitle( spectrumName.at(r).c_str() ) ;

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
  
    if( notFound == nHists ){
        cout << " ERROR : no spectra available " << endl ;
        return 3 ;
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
        return 4 ;
    }

    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;
        
    gStyle->SetPalette(55);
    gStyle->SetOptTitle(0) ;
    gStyle->SetOptStat(0) ;
    gStyle->SetOptFit(0) ;

    gStyle->SetPadTopMargin(    0.03 ) ;
    gStyle->SetPadRightMargin(  0.11 ) ;
    gStyle->SetPadBottomMargin( 0.12 ) ;
    gStyle->SetPadLeftMargin(   0.06 ) ;

    gStyle->SetTitleOffset( 1.2 , "x" ) ;
    gStyle->SetTitleOffset( 0.6 , "y" ) ;
    
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
    
    TCanvas * can = new TCanvas( "overlay" , "overlay" , 2000 , 500 ) ; 
    if(useLogScale[0]) can->SetLogx() ; 
    if(useLogScale[1]) can->SetLogy() ; 
    
    bool firstTOdraw = true ;
    
    TH1D ** normalized = new TH1D*[nHists] ;
    
    for(unsigned int h=0; h<nHists; h++){
        
        if( !useable[h] ) continue ;
        
        normalized[h] = new TH1D( 
                                    spectrumName.at(h).c_str() , 
                                    spectrumName.at(h).c_str() ,
                                    nBins , range[0] , range[1]
                                );
        
        double entries = (double)( hists[h]->GetEntries() ) ;
        
        for(unsigned int b=1; b<=nBins ; b++ ){
            normalized[h]->SetBinContent( 
                b ,
                (double)( hists[h]->GetBinContent( b ) )
                / entries
            ) ;
            normalized[h]->SetBinError( 
                b ,
                sqrt( (double)( hists[h]->GetBinContent( b ) ) )
                / entries
            ) ;
        }
        
        hists[h]->Delete() ;
        
        normalized[h]->SetMarkerStyle(6) ;
        normalized[h]->SetLineWidth(3) ;
        
        if( firstTOdraw ){ 
            
            normalized[h]->Draw("P PLC PMC") ;
            
            if( ! plotRanges[0][0].setting ) 
                plotRanges[0][0].number = 0. ;
            if( ! plotRanges[0][1].setting ) 
                plotRanges[0][1].number = 1. ;
            if( ! plotRanges[1][0].setting ) 
                plotRanges[1][0].number = range[0] ;
            if( ! plotRanges[1][1].setting ) 
                plotRanges[1][1].number = range[1] ;
                
            normalized[h]->GetXaxis()->SetRangeUser( 
                plotRanges[0][0].number , plotRanges[0][1].number
            );
            normalized[h]->GetYaxis()->SetRangeUser( 
                plotRanges[1][0].number , plotRanges[1][1].number
            );
            
            if( axisTitles[0].compare(neverUse) != 0 )
                normalized[h]->SetXTitle( axisTitles[0].c_str() ) ;
            if( axisTitles[1].compare(neverUse) != 0 )
                normalized[h]->SetYTitle( axisTitles[1].c_str() ) ;
            
            firstTOdraw = false ;
            
        }
        else{ 
            normalized[h]->Draw("P same PLC PMC") ;
        }
        
//         normalized[h]->Write() ;
        
    }
    
    gPad->SetGridx() ;
    gPad->SetGridy() ;
    
    can->BuildLegend( 0.90 , 0.15 , 0.995 , 0.95 ) ;
    
    bool toBeEdited = true ;
    
    while( toBeEdited ){
        
        gPad->Modified() ;
        gPad->Update() ;
        gPad->WaitPrimitive() ;
        
        gPad->Modified() ;
        gPad->Update() ;
        gPad->WaitPrimitive() ;
        
        cout << " plot OK ? (y/n) : " ;
        string answer ;
        cin >> answer ;
        if( answer.compare("y") == 0 ){
            toBeEdited = false ;
            break ;
        }
        
    }

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
