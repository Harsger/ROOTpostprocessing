#include <TROOT.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TStyle.h>
#include <TPad.h> 
#include <TCanvas.h>
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
    "/" , "\\" , "+" , "-" , "=" , "." , ":" , "\"" , " " , "\t" , "," , ";"
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
    SpecifiedNumber highLimit
){
    
    bool allGood = true ;
    
    unsigned int rowsNcolumns[2] = { 
        (unsigned int)hist->GetNbinsX() ,
        (unsigned int)hist->GetNbinsY() 
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
    
    for(unsigned int r=0; r<rowsNcolumns[0]; r++){
        for(unsigned int c=0; c<rowsNcolumns[1]; c++){
            double content = hist->GetBinContent( r+1 , c+1 ) ;
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

double getFWHM( 
                TH1D * distribution , 
                double maximum , 
                double position , 
                double offset = 0
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
                heights[0] > halfHeight
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
    
    if( halfPositions[0] < halfPositions[1] ){
        FWHM = halfPositions[1] - halfPositions[0] ;
    }
    
    return FWHM ;
    
}

void plotOptions(){
        
    gROOT->SetStyle("Plain") ;
    
    gStyle->SetPalette(kRainBow);
    gStyle->SetOptTitle(0) ;
    gStyle->SetOptStat(11110) ;
    gStyle->SetOptFit(0) ;

    gStyle->SetPadTopMargin(    0.05 ) ;
    gStyle->SetPadRightMargin(  0.16 ) ;
    gStyle->SetPadBottomMargin( 0.10 ) ;
    gStyle->SetPadLeftMargin(   0.12 ) ;

    gStyle->SetTitleOffset( 1.3 , "x" ) ;
    gStyle->SetTitleOffset( 0.7 , "y" ) ;
    gStyle->SetTitleOffset( 2.0 , "z" ) ;

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
    
    gROOT->ForceStyle() ;
    
}

void showing(){
    
    bool toBeEdited = true ;
    
    while( toBeEdited ){
        
        gPad->Modified() ;
        gPad->Update() ;
        gPad->WaitPrimitive() ;
        
        gPad->Modified() ;
        gPad->Update() ;
        gPad->WaitPrimitive() ;
        
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

