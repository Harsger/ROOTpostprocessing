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

#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

std::vector< std::vector<std::string> > getInput( std::string filename ){
    
    std::vector< std::vector<std::string> > input;
    
    if( filename.compare("") == 0 ){ 
        std::cout << " WARNING : no input to read from " << std::endl;
        return input;
    }
    
    std::ifstream ifile(filename.c_str());
    if( !( ifile ) ){ 
        std::cout << " WARNING : could not read input file " 
                  << filename << std::endl;
        return input;
    }
    
    std::string line = "";
    std::string word = "";
    std::vector<std::string> dummy;
    
    while( getline( ifile, line) ){
        
        std::stringstream sline(line);
        
        while( !( sline.eof() ) ){ 
            
            sline >> std::skipws >> word;
            if( word != "" ) dummy.push_back(word);
            word="";
            
        }
        
        if( dummy.size() > 0 ) input.push_back(dummy);
        dummy.clear();
    }
    
    ifile.close();
    
    return input;
    
}

bool toDiscard( double d ){
    if( (d==0.) && ((d+1)!=0.) ) return false ;
    else return ( !( std::isnormal( d ) ) ) ;
}

class SpecifiedNumber{
    
    public :
        
        double number ;
        std::string specifier ;
        bool setting ;
        
        SpecifiedNumber(){
            number = 0. ;
            specifier = "" ;
            setting = false ;
        }
        
        SpecifiedNumber( double d ){
            number = d ;
            specifier = "" ;
            setting = true ;
        }
        
} ;

bool getStats(
    std::vector<double> * values ,
    double &mean ,
    double &stdv ,
    double &min ,
    double &max ,
    double &median 
){
    
    unsigned int nValues = values->size() ;
    
    if( nValues < 1 ){ 
        std::cout << " WARNING : vectorSize zero " << std::endl;
        return false ;
    }
                
    std::vector<double> vecCopy = *values ;
    
    mean = 0. ;
    stdv = 0. ;
    min = vecCopy.at(0) ;
    max = vecCopy.at(0) ;
    
    for( auto v : vecCopy ){
        mean += v ;
        stdv += ( v * v ) ;
        if( min > v ) min = v ;
        if( max < v ) max = v ;
    }
    
    mean /= (double)nValues ;
    if( nValues == 1 ) stdv = 0. ;
    else
        stdv = sqrt( 
                        ( stdv - mean * mean * (double)nValues ) 
                        / 
                        ( (double)nValues - 1. ) 
                ) ;
    
    nth_element( 
                    vecCopy.begin() , 
                    vecCopy.begin() + nValues/2 ,
                    vecCopy.end()
               ) ;
               
    if( nValues % 2 == 0 )
        median = 0.5 * ( vecCopy[nValues/2] + vecCopy[nValues/2 - 1] ) ;
    else
        median = vecCopy[nValues/2] ;
        
                
    return true ;
    
}

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
            double content = hist->GetBinContent( r , c ) ;
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

double interpolate(
    double x1 ,
    double y1 ,
    double x2 ,
    double y2 ,
    double v
){
    
    if( x1 == x2 ) return x1 ;
    
    double slope = ( y2 - y1 ) / ( x2 - x1 ) ;
    double intercept = ( y1 * x2 - y2 * x1 ) / ( x2 - x1 ) ;
    
    return slope * v + intercept ;
    
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

