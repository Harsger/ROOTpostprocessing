#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 3 ){ 
        cout << " ERROR : filename and histname required " << endl ;
        return 1 ;
    }

    TString filename = argv[1] ;
    TString histname = argv[2] ;
    
    SpecifiedNumber threshold ;
    
    if( argc > 3 ){
        TString threshString = argv[3] ;
        TString tester = threshString( 0 , 1 ) ;
        if( tester.IsDec() ){
            threshold = SpecifiedNumber( atof( threshString.Data() ) ) ;
        }
    }
                  
    TApplication app("app", &argc, argv) ;    
    plotOptions() ;
    
    gStyle->SetOptStat(0) ;
    
    if( threshold.setting ){

        gStyle->SetPadTopMargin(    0.04 ) ;
        gStyle->SetPadRightMargin(  0.03 ) ;
        gStyle->SetPadBottomMargin( 0.10 ) ;
        gStyle->SetPadLeftMargin(   0.08 ) ;

        gStyle->SetTitleOffset( 1.0 , "x" ) ;
        gStyle->SetTitleOffset( 0.8 , "y" ) ;
        
    }
    else{
        
        gStyle->SetPadRightMargin(  0.18 ) ;

        gStyle->SetTitleOffset( 1.0 , "x" ) ;
        gStyle->SetTitleOffset( 1.2 , "y" ) ;
        gStyle->SetTitleOffset( 1.4 , "z" ) ;
        
    }
    
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
    
    if( threshold.setting ){
        
        double mean , stdv , min , max , median ;
        unsigned int number ;
        vector<double> toSkip ;
        SpecifiedNumber lowLimit , highLimit ;
        
        bool working = getStats(
            hist , mean , stdv , min , max , median , number ,
            toSkip , lowLimit , highLimit
        ) ;
        
        if( ! working ) return 4 ;
                
        cout << " mean " << mean << " \t stdv " << stdv << endl ;
        
        double
                lowThreshold  = mean - threshold.number * stdv ,
                highThreshold = mean + threshold.number * stdv ;
                
        cout << " low " << lowThreshold 
             << " \t high " << highThreshold << endl ;
        
        unsigned int bins[2] = {
            (unsigned int)hist->GetNbinsX() ,
            (unsigned int)hist->GetNbinsY() 
        } ;
        
        name += "_" ;
        name += "noisyBins.txt" ;
        
        ofstream outfile( name.Data() , std::ofstream::out ) ;
        
        for(unsigned int x=0; x<bins[0]; x++){
            for(unsigned int y=0; y<bins[1]; y++){
                double content = hist->GetBinContent( x+1 , y+1 ) ;
                if( 
                    content < lowThreshold
                    ||
                    content > highThreshold
                ){
                    double center[2] = {
                        hist->GetXaxis()->GetBinCenter(x+1) ,
                        hist->GetYaxis()->GetBinCenter(y+1) ,
                    } ;
                    outfile << center[0] << " " << center[1] << " " << content << endl ;
                }
            }
        }
        
        outfile.close() ;
        
        TString zTitle = hist->GetZaxis()->GetTitle() ;
        
        TH1D * projection ;
        
        for(unsigned int c=0; c<2; c++){
            
            TString proName = name ;
            TString replacement = "noisyColumns.txt" ;
            
            if( c == 0 ){
                projection = hist->ProjectionX() ;
            }
            else{
                projection = hist->ProjectionY() ;
                replacement = "noisyRows.txt" ;
            }
            
            projection->Scale(1./bins[(c+1)%2]) ; 
            
            proName = proName.ReplaceAll( "noisyBins.txt" , replacement ) ;
            outfile.open( proName.Data() , std::ofstream::out ) ;
            
            for(unsigned int b=0; b<bins[c]; b++){
                double content = projection->GetBinContent( b+1 ) ;
                if( 
                    content < lowThreshold
                    ||
                    content > highThreshold
                ){
                    outfile << projection->GetXaxis()->GetBinCenter( b+1 ) 
                            << " " << content << endl ;
                }
            }
            
            outfile.close() ;
            
            proName = replacement ;
            proName = proName.ReplaceAll( "noisy" , "" ) ;
            proName = proName.ReplaceAll( ".txt" , "" ) ;
            
            TCanvas * can 
                = new TCanvas( proName , proName , 1000 , 500 ) ; 
            
            projection->GetYaxis()->SetTitle( zTitle ) ;
            projection->Draw("HIST") ;
            
            showing() ;
            
            replacement = proName ;
            proName = name ;
            proName = proName.ReplaceAll( "noisyBins.txt" , replacement ) ;
            proName += ".pdf" ;
            
            can->Print( proName ) ;
            
            projection->Delete() ;
            
            can->Close() ;
            
        }
        
    }
    else{
        
        TCanvas * can = new TCanvas( histname , histname , 700 , 600 ) ;
        
        hist->Draw("COLZ") ;
            
        showing() ;
        
        name += ".pdf" ;
            
        can->Print( name ) ;
        
        can->Close() ;
        
    }

    hist->Delete() ;

    return 0 ;

}
