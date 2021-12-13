#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 4 ) return 1 ;

    TString filename = argv[1] ;
    TString histname = argv[2] ;
    
    SpecifiedNumber threshold ;
    SpecifiedNumber lowThreshold ;
    SpecifiedNumber highThreshold ;
    SpecifiedNumber nBins ;
    
    bool show = true ;
    
    if( argc == 4 ){
        TString threshString = argv[3] ;
        TString tester = threshString( 0 , 1 ) ;
        if( tester.IsDec() ){
            threshold = SpecifiedNumber( atof( threshString.Data() ) ) ;
        }
        else show = false ;
    }
    else if( argc > 4 ){
        TString tester = argv[3] ;
        if( tester != "%" )
            lowThreshold = SpecifiedNumber( atof( tester.Data() ) ) ;
        tester = argv[4] ;
        if( tester != "%" )
            highThreshold = SpecifiedNumber( atof( tester.Data() ) ) ;
        if( argc > 5 ){ 
            tester = argv[5] ;
            TString firstChar = tester( 0 , 1 ) ;
            if( !( firstChar.IsDec() ) ) show = false ;
            else nBins = SpecifiedNumber( atof( tester.Data() ) ) ;
            if( argc > 6 ) show = false ;
        }
    }
                  
    TApplication app("app", &argc, argv) ;    
    plotOptions() ;
    
    if( show ) gStyle->SetOptStat(1110) ;
    else gStyle->SetOptStat(0) ;
    
    gStyle->SetPadTopMargin(    0.04 ) ;
    gStyle->SetPadRightMargin(  0.03 ) ;
    gStyle->SetPadBottomMargin( 0.10 ) ;
    gStyle->SetPadLeftMargin(   0.08 ) ;

    gStyle->SetTitleOffset( 1.0 , "x" ) ;
    gStyle->SetTitleOffset( 0.8 , "y" ) ;
    
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
        
    name += ".root" ;
    TFile * outfile = new TFile( name , "RECREATE" );
    name = name.ReplaceAll( ".root" , "" ) ;
    
    double mean , stdv , min , max , median ;
    unsigned int number ;
    vector<double> toSkip ;
    SpecifiedNumber lowLimit , highLimit ;
    map< string , map< unsigned int , bool > > useRowsNcolumns ;
    map< unsigned int , vector<unsigned int> > pixelList ;
    bool exclude = true ;
    
    bool working = getStats(
        hist , mean , stdv , min , max , median , number ,
        toSkip , lowLimit , highLimit , useRowsNcolumns ,
        pixelList , exclude
    ) ;
    
    if( ! working ) return 4 ;
            
    cout << " mean " << mean << " \t stdv " << stdv << endl ;
    
    if( threshold.setting ){
            lowThreshold  
                = SpecifiedNumber( mean - threshold.number * stdv ) ;
            highThreshold 
                = SpecifiedNumber( mean + threshold.number * stdv ) ;
    }
            
    if( lowThreshold.setting ) 
        cout << " lowThreshold " << lowThreshold.number << endl;
    if( highThreshold.setting ) 
        cout << " highThreshold " << highThreshold.number << endl;
    
    unsigned int bins[2] = {
        (unsigned int)hist->GetNbinsX() ,
        (unsigned int)hist->GetNbinsY() 
    } ;
    
    name += "_" ;
    name += "noisyBins.txt" ;
    
    ofstream textout( name.Data() , std::ofstream::out ) ;
    
    TString proName = name ;
    proName = proName.ReplaceAll( "_noisyBins.txt" , "" ) ;
    
    TCanvas * can = new TCanvas( proName , proName , 1000 , 500 ) ; 
    
    TH1I * binSpectrum = new TH1I( 
                                    "binSpectrum" , "binSpectrum" , 
                                    number/10. , 
                                    min , max+(max-min)/(double)number
                                ) ;
                                
    if( nBins.setting ){
        binSpectrum->Delete() ;
        binSpectrum = new TH1I( 
                                    "binSpectrum" , "binSpectrum" , 
                                    nBins.number , 
                                    lowThreshold.number , 
                                    highThreshold.number
                                ) ;
    }
    
    for(unsigned int x=0; x<bins[0]; x++){
        for(unsigned int y=0; y<bins[1]; y++){
            double content = hist->GetBinContent( x+1 , y+1 ) ;
            binSpectrum->Fill( content ) ;
            if( 
                (
                    lowThreshold.setting
                    &&
                    content < lowThreshold.number
                )
                ||
                (
                    highThreshold.setting
                    &&
                    content > highThreshold.number
                )
            ){
                double center[2] = {
                    hist->GetXaxis()->GetBinCenter(x+1) ,
                    hist->GetYaxis()->GetBinCenter(y+1) ,
                } ;
                textout 
                        << center[0] << " " 
                        << center[1] << " " 
                        << content << endl ;
            }
        }
    }
    
    textout.close() ;
    
    binSpectrum->Draw("HIST") ;
    
    TLine * lineLow , * lineHigh ;
    if( lowThreshold.setting ){
        lineLow = new TLine( 
                                lowThreshold.number , 
                                0. , 
                                lowThreshold.number , 
                                binSpectrum->GetMaximum() 
                            ) ;
        lineLow->SetLineColor(kBlue) ;
        lineLow->Draw() ;
    }
    if( highThreshold.setting ){
        lineHigh = new TLine( 
                                highThreshold.number , 
                                0. , 
                                highThreshold.number , 
                                binSpectrum->GetMaximum() 
                            ) ;
        lineHigh->SetLineColor(kRed) ;
        lineHigh->Draw() ;
    }
    
    if( show ) showing() ;
    
    proName += "_binSpectrum.pdf" ;
    can->Print( proName ) ;
    can->Close() ;
    
    TF1 * lowLine , * highLine ;
    if( lowThreshold.setting ){
        lowLine = new TF1( "lowThreshold" , "[0]" ) ;
        lowLine->SetParameter( 0 , lowThreshold.number ) ;
        lowLine->SetLineColor(kBlue) ;
    }
    if( highThreshold.setting ){
        highLine = new TF1( "highThreshold" , "[0]" ) ;
        highLine->SetParameter( 0 , highThreshold.number ) ;
        highLine->SetLineColor(kRed) ;
    }
    
    TString zTitle = hist->GetZaxis()->GetTitle() ;
    
    TH1D * projection ;
    
    for(unsigned int c=0; c<2; c++){
        
        proName = name ;
        TString replacement = "noisyColumns.txt" ;
        if( c == 0 ){ 
            projection = hist->ProjectionX() ;
        }
        else{
            projection = hist->ProjectionY() ;
            replacement = "noisyRows.txt" ;
        }
        
        project( hist , projection , c ) ;
        
        TString nameHist = replacement ;
        nameHist = nameHist.ReplaceAll( "noisy" , "" ) ;
        nameHist = nameHist.ReplaceAll( ".txt" , "" ) ;
        projection->SetName( nameHist ) ;
        projection->SetTitle( nameHist ) ;
        
//         projection->Scale(1./bins[(c+1)%2]) ; 
        
        proName = proName.ReplaceAll( "noisyBins.txt" , replacement ) ;
        textout.open( proName.Data() , std::ofstream::out ) ;
        
        for(unsigned int b=0; b<bins[c]; b++){
            double content = projection->GetBinContent( b+1 ) ;
            if( 
                (
                    lowThreshold.setting
                    &&
                    content < lowThreshold.number
                )
                ||
                (
                    highThreshold.setting
                    &&
                    content > highThreshold.number
                )
            ){
                textout << projection->GetXaxis()->GetBinCenter( b+1 ) 
                        << " " << content << endl ;
            }
        }
        
        textout.close() ;
        
        proName = replacement ;
        proName = proName.ReplaceAll( "noisy" , "" ) ;
        proName = proName.ReplaceAll( ".txt" , "" ) ;
        
        can = new TCanvas( proName , proName , 1000 , 500 ) ; 
        
        projection->GetYaxis()->SetTitle( zTitle ) ;
        projection->Draw("HIST") ;
        
        if( lowThreshold.setting ){ 
            lowLine->SetRange(
                projection->GetXaxis()->GetXmin() ,
                projection->GetXaxis()->GetXmax() 
            ) ;
            lowLine->Draw("same") ;
        }
        if( highThreshold.setting ){
            highLine->SetRange(
                projection->GetXaxis()->GetXmin() ,
                projection->GetXaxis()->GetXmax() 
            ) ;
            highLine->Draw("same") ;
        }
        
        if( show ) showing() ;
        
        replacement = proName ;
        proName = name ;
        proName = proName.ReplaceAll( "noisyBins.txt" , replacement ) ;
        proName += ".pdf" ;
        
        can->Print( proName ) ;
        
        can->Close() ;
        
    }
    
    outfile->Write() ;
    outfile->Close() ;

    hist->Delete() ;

    return 0 ;

}
