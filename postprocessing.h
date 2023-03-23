#include <TROOT.h>
#include <TSystem.h>
#include <TKey.h>
#include <TApplication.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TStyle.h>
#include <TPad.h> 
#include <TCanvas.h>
#include <TPaveStats.h>
#include <TLegend.h> 
#include <TLegendEntry.h>
#include <TFile.h>
#include <TString.h>
#include <TText.h>
#include <TMath.h>
#include <TBranch.h>
#include <TTree.h>
#include <TF1.h>
#include <TF2.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TGraph2D.h>
#include <TGraphErrors.h>
#include <TProfile.h>
#include <TGaxis.h>

#include "useful.h"
    
double text_size = 0.05 ;
int font = 42 ;

std::vector<TString> badCharsForNames = {
    "/" , "\\" , "+" , "-" , "=" , "." , ":" , "\"" , " " , "\t" , "," , ";" , 
    "#" , "$" , "%" , "^" , "'" , "@" , "!" , "<" , ">" , "?" , "{" , "}" , 
    "[" , "]" , "(" , ")" , "*" , "~" , "|" , "&"
} ;

bool getStats(
    TH2D * hist ,
    double &mean ,
    double &stdv ,
    double &min ,
    double &max ,
    double &median ,
    unsigned int &number ,
    std::vector<double> toSkip ,
    SpecifiedNumber lowLimit ,
    SpecifiedNumber highLimit , 
    std::map< std::string , std::map< unsigned int , bool > > useRowsNcolumns ,
    std::map< unsigned int , std::vector<unsigned int> > pixelList ,
    bool exclude = true
){
    
    bool allGood = true ;
    
    unsigned int rowsNcolumns[2] = { 
        (unsigned int)hist->GetNbinsY() ,
        (unsigned int)hist->GetNbinsX() 
    } ;
    
    if(
        rowsNcolumns[0] == 0
        &&
        rowsNcolumns[1] == 0
    ){
        std::cout << " WARNING : histsize zero " << std::endl;
        return false ;
    } ;
    
    unsigned int nSkipper = toSkip.size() ;
    std::vector<double> values ;
    number = 0 ;
    
    bool discardRowsOrColumns[2] = { false , false } ;
    if( useRowsNcolumns.find("ROWS") != useRowsNcolumns.end() )
        discardRowsOrColumns[0] = true ;
    if( useRowsNcolumns.find("COLUMNS") != useRowsNcolumns.end() )
        discardRowsOrColumns[1] = true ;
    
    bool pixelSpecified = false ;
    if( !( pixelList.empty() ) ) pixelSpecified = true ;
    
    for(unsigned int r=0; r<rowsNcolumns[0]; r++){
        for(unsigned int c=0; c<rowsNcolumns[1]; c++){
            if( discardRowsOrColumns[0] && !( useRowsNcolumns["ROWS"][r+1] ) )
                continue ;
            if( discardRowsOrColumns[1] && !( useRowsNcolumns["COLUMNS"][c+1] ) )
                continue ;
            if( pixelSpecified ){
//                if( exclude ){
//                    if( 
//                        pixelList.find(r+1) != pixelList.end() 
//                        && 
//                        std::find( 
//                                    pixelList[r+1].begin() ,
//                                    pixelList[r+1].end() ,
//                                    c+1
//                        )
//                            != 
//                                    pixelList[r+1].end()
//                    )
//                        continue ;
//                }
//                else{
//                    if( 
//                        pixelList.find(r+1) == pixelList.end() 
//                        || 
//                        std::find( 
//                                    pixelList[r+1].begin() ,
//                                    pixelList[r+1].end() ,
//                                    c+1
//                        )
//                            == 
//                                    pixelList[r+1].end()
//                    )
//                        continue ;
//                }
                if( 
                    pixelList.find(r+1) != pixelList.end() 
                    && 
                    std::find( 
                                pixelList[r+1].begin() , 
                                pixelList[r+1].end() , 
                                c+1 
                             )
                        !=
                            pixelList[r+1].end()
                ){
                    if( exclude ) continue ;
                }
                else{
                    if( !( exclude ) ) continue ;
                }
            }
            double content = hist->GetBinContent( c+1 , r+1 ) ;
            if( toDiscard( content ) ) continue ;
            if( 
                lowLimit.setting 
                && 
                content < lowLimit.number
            )
                continue ;
            if( 
                highLimit.setting 
                && 
                content > highLimit.number
            )
                continue ;
            bool skipThis = false ;
            for(unsigned int s=0; s<nSkipper; s++){
                if( content == toSkip.at(s) ){
                    skipThis = true ;
                    break ;
                }
            }
            if( skipThis ) continue ;
            values.push_back( content ) ;
        }
    }
    
    number = values.size() ;
    
    allGood = getStats(
        &values ,
        mean ,
        stdv ,
        min ,
        max ,
        median 
    );
    
    return allGood ;
    
}

bool getStats(
    TH2D * hist ,
    double &mean ,
    double &stdv ,
    double &min ,
    double &max ,
    double &median ,
    unsigned int &number 
){
    
    std::vector<double> toSkip ;
    SpecifiedNumber lowLimit ;
    SpecifiedNumber highLimit ; 
    std::map< std::string , std::map< unsigned int , bool > > useRowsNcolumns ;
    std::map< unsigned int , std::vector<unsigned int> > pixelList ;
    
    return getStats( 
                        hist , mean , stdv , min , max , median , number ,
                        toSkip , lowLimit , highLimit , 
                        useRowsNcolumns , pixelList
                   ) ;
}

double getFWHM( 
                TH1D * distribution , 
                double maximum , 
                double position , 
                double offset ,
                double &halfLow ,
                double &halfHigh
){

    double FWHM = -1. ;
    
    unsigned int nBins = distribution->GetNbinsX() ;
    unsigned int maxPosBin = distribution->FindBin( position ) ;
//     double binWidth = distribution->GetBinWidth() ;
    
    double halfHeight = 0.5 * ( maximum - offset ) ;
    
    double halfPositions[2] = { 1. , -1. } ;
    int halfBins[2] = { -1 , -1 } ;
    double heights[2] = { 0. , 0. } ;

    for(int s=-1; s<2; s+=2){
    
        unsigned int b = maxPosBin + s ;
        
        while(
            b > 0
            &&
            b < nBins
        ){
            
            heights[0] = distribution->GetBinContent( b ) - offset ;
            heights[1] = distribution->GetBinContent( b + s ) - offset ;
            
            if(
                heights[0] >= halfHeight
                &&
                heights[1] < halfHeight
            ){
                
                halfBins[(s+1)/2] = b ;
                
                halfPositions[(s+1)/2] 
                    = 
                    interpolate(
                        heights[0] ,
                        distribution->GetBinCenter( b ) ,
                        heights[1] ,
                        distribution->GetBinCenter( b + s ) ,
                        halfHeight
                    ) ;
                    
                break ;
            }
            
            b += s ;
            
        }
        
    }
    
    halfLow  = halfPositions[0] ;
    halfHigh = halfPositions[1] ;
    
    if( halfPositions[0] < halfPositions[1] ){
        FWHM = halfPositions[1] - halfPositions[0] ;
    }
    
    return FWHM ;
    
}


double getFWHM( 
                TH1D * distribution , 
                double maximum , 
                double position , 
                double offset = 0.
){
    
    double halfLow , halfHigh ;
    
    return getFWHM( 
                    distribution , 
                    maximum , 
                    position , 
                    offset ,
                    halfLow ,
                    halfHigh
                ) ;
    
}

void project(
                TH2D * hist , TH1D * projection ,
                unsigned int axis = 0 , bool toAverage = true
){
    
    if( axis == 1 ){
        if( projection->GetNbinsX() != hist->GetNbinsY() ) return ;
    }
    else{
        if( projection->GetNbinsX() != hist->GetNbinsX() ) return ;
    }
    
    double value , mean = 0. , error = 0. ;
    unsigned int number = 1 ;
    
    unsigned int bins[2] = { 
        (unsigned int)hist->GetNbinsX()+1 ,
        (unsigned int)hist->GetNbinsY()+1
    } ;
    unsigned int other = 1 , x , y ;
    if( axis == 1 ) other = 0 ;
    
    unsigned int start[2] = { 1 , 1 } ;
    if( !( toAverage )  ){
        bins[0]++ ;
        bins[1]++ ;
        start[0] = 0 ;
        start[1] = 0 ;
    }

    for(unsigned int b=start[axis]; b<bins[axis]; b++){
        mean = 0. ;
        error = 0. ;
        if( toAverage ) number = 0 ;
        for(unsigned int o=start[other]; o<bins[other]; o++){
            x = b ;
            y = o ;
            if( axis == 1 ){
                x = o ;
                y = b ;
            }
            value = hist->GetBinContent( x , y ) ;
            if( toDiscard( value ) ) continue ;
            mean += value ;
            error += hist->GetBinError( x , y ) ;
            if( toAverage ) number++ ;
        }
        mean /= (double)number ;
        projection->SetBinContent( b , mean ) ;
        error /= (double)number ;
        projection->SetBinError( b , error ) ;
    }
    
}

void plotOptions( 
                    bool broadCanvas = false ,
                    bool shiftXexponent = false ,
                    unsigned int maxAxisDigits = 3
                ){
        
    gROOT->SetStyle("Plain") ;
    
    gStyle->SetPalette(55); // kRainBow
    gStyle->SetOptTitle(0) ;
    gStyle->SetOptStat(11110) ;
    gStyle->SetOptFit(0) ;

    gStyle->SetPadTopMargin(    0.06 ) ;
    gStyle->SetPadRightMargin(  0.10 ) ;
    gStyle->SetPadBottomMargin( 0.12 ) ;
    gStyle->SetPadLeftMargin(   0.15 ) ;

    gStyle->SetTitleOffset( 1.1 , "x" ) ;
    gStyle->SetTitleOffset( 1.6 , "y" ) ;
    gStyle->SetTitleOffset( 1.4 , "z" ) ;

    if( broadCanvas ){
        gStyle->SetPadRightMargin(  0.04 ) ;
        gStyle->SetPadLeftMargin(   0.06 ) ;
        gStyle->SetTitleOffset( 0.6 , "y" ) ;
    }
    if( shiftXexponent ) TGaxis::SetExponentOffset( 0.016 , -0.07 , "x" ) ;
    TGaxis::SetMaxDigits( maxAxisDigits ) ;

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
    
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);

    gROOT->ForceStyle() ;
    
}

void padWaiting(){

    gPad->Modified() ;
    gPad->Update() ;
    gPad->WaitPrimitive() ;

}

void showing(){
    
    bool toBeEdited = true ;
    
    while( toBeEdited ){
        
        padWaiting() ;
        padWaiting() ;
        
        std::cout << " plot OK ? (y/n) : " ;
        std::string answer ;
        std::cin >> answer ;
        if( answer.compare("y") == 0 ){
            toBeEdited = false ;
            break ;
        }
        
    }
    
}

TString replaceBadChars( TString input ){
    TString output = input ;
    for( auto c : badCharsForNames ){
        if( output.Contains(c) )
            output.ReplaceAll( c , "" ) ;
    }
    return output ;
}

void getOutflow( 
                    TH1D * hist , 
                    double lowLimit , 
                    double highLimit , 
                    double &underFlow , 
                    double &overFlow
){
    
    unsigned int nbins = hist->GetNbinsX() ;
    
//     double range[2] = {
//         hist->GetXaxis()->GetXmin() ,
//         hist->GetXaxis()->GetXmax() 
//     } ;
    
    underFlow = 0. ;
    underFlow += hist->GetBinContent( 0 ) ;
    int bin = hist->GetXaxis()->FindBin(lowLimit)-1 ;
    if( bin > 0 )
        underFlow += hist->Integral( 1 , bin ) ;
    
    overFlow = 0. ;
    overFlow += hist->GetBinContent( nbins+1 );
    bin = hist->GetXaxis()->FindBin(highLimit)+1 ;
    if( bin <= nbins )
        overFlow += hist->Integral( bin , nbins ) ;
    
}

