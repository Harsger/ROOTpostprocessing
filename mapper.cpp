#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

    TString filename = argv[1] ;
    
    TString axisTitle[3] = { "" , "" , "" } ;
    double markerSize = 1. ;
    int colorPalette = -100 ;
    unsigned int nContours = 20 ;
    SpecifiedNumber plotRanges[3][3] ; 
    bool useLogScale[3] = { false , false , false } ;
    bool drawGrid[3]    = { false , false , false } ;

    for(unsigned int c=0; c<3; c++)
        if( argc > 2+c && string( argv[2+c] ) != "%" )
            axisTitle[c] = argv[2+c] ;
   
    TString name ; 
    double x , y , z ;
    if( argc > 5 && string( argv[5] ) != "%" ){
        name = argv[5] ;
        z = getNumberWithRange( name.Data() , x , y ) ;
        if( !( toDiscard(x) ) ) colorPalette = (int)x ;
        if( !( toDiscard(y) ) ) nContours    = (unsigned int)y ;
        if( !( toDiscard(z) ) ) markerSize   = z ;
    }
   
    for(unsigned int c=0; c<3; c++){
        if( argc > 6+c && string( argv[6+c] ) != "%" ){
            name = argv[6+c] ;
            if( name.Contains("log") ){
                useLogScale[c] = true ;
                name = name.ReplaceAll( "log" , "" ) ;
            }
            if( name.Contains("grid") ){
                drawGrid[c] = true ;
                name = name.ReplaceAll( "grid" , "" ) ;
            }
            z = getNumberWithRange( name.Data() , x , y ) ;
            if( !( toDiscard(x) ) ) plotRanges[c][0] = SpecifiedNumber( x ) ;
            if( !( toDiscard(y) ) ) plotRanges[c][1] = SpecifiedNumber( y ) ;
            if( !( toDiscard(z) ) ) plotRanges[c][2] = SpecifiedNumber( z ) ;
        }
    }

    vector< vector<double> > data ;
    vector<double> singleRow ;
    unsigned int nRows ;

    if( argc > 9 && filename.EndsWith(".root") ){
        TString graphname = argv[9] ;
        TFile * input = new TFile( filename , "READ" ) ;
        if( input->IsZombie() ){
            cout << " ERROR : can not read root-file " << endl ;
            return 2 ;
        }
        if( input->Get( graphname ) == NULL ){
            cout << " ERROR : reading " << graphname ;
            cout << " in " << filename << endl ;
            return 3 ;
        }
        TGraph2D * dataPoints = (TGraph2D*)( input->Get( graphname ) ) ;
        nRows = dataPoints->GetN() ;
        for(unsigned int r=0; r<nRows; r++){
            dataPoints->GetPoint( r , x , y , z ) ;
            data.push_back( { x , y , z } ) ;
        }
        dataPoints->Delete() ;
        input->Close() ;
    }
    else{
        vector< vector<string> > textData = getInput( filename.Data() ) ;
        nRows = textData.size() ;
        for(unsigned int r=0; r<nRows; r++){
            if( textData.at(r).size() < 3 ) continue ;
            for(unsigned int c=0; c<3; c++)
                singleRow.push_back( atof( textData.at(r).at(c).c_str() ) ) ;
            data.push_back( singleRow ) ;
            singleRow.clear() ;
        }
    }

    nRows = data.size() ;
    if( nRows < 1 ){
        cout << " ERROR : data is empty " << endl ;
        return 2 ;
    }

    string path , base ;
    splitFilename( filename.Data() , path , base ) ;
    TString outname = base ;
    if( outname.Contains(".") ) outname = outname( 0 , outname.Last('.') ) ;

    TApplication app("app", &argc, argv) ; 
    plotOptions( false , true ) ;
    gStyle->SetOptStat(0) ;
    gStyle->SetPadRightMargin(0.18) ;
    
    gStyle->SetPalette( abs( colorPalette ) ) ; 
    if( colorPalette < 0 ) TColor::InvertPalette() ;
    gStyle->SetNumberContours( nContours ) ;

    auto paletteList = TColor::GetPalette() ;
    unsigned int nColors = TColor::GetNumberOfColors() ;
    int colorList[ nContours ] ;
    int colorNumber ;
    for(unsigned int c=0; c<nContours; c++){
        colorNumber = (int)(
                            (double)(c+1)
                            /
                            (double)nContours
                            *
                            (double)nColors
                        ) ;
        if( colorNumber > nColors-1 ) colorNumber = nColors-1 ;
        colorList[c] = paletteList.At( colorNumber ) ;
    }

    map< int , TGraphErrors* > graphMap ;

    if(
        !( plotRanges[0][0].setting ) || !( plotRanges[0][1].setting )
        ||
        !( plotRanges[1][0].setting ) || !( plotRanges[2][1].setting )
        ||
        !( plotRanges[2][0].setting ) || !( plotRanges[2][1].setting )
    ){
        bool toInitialize[3][2] ;
        for(unsigned int c=0; c<3; c++)
            for(unsigned int r=0; r<2; r++)
                toInitialize[c][r] = true ;
        for(unsigned int r=0; r<nRows; r++){
            for(unsigned int c=0; c<3; c++){
                for(int p=-1; p<2; p+=2){
                    if( 
                        !( plotRanges[c][(p+1)/2].setting ) 
                        && 
                        (
                            plotRanges[c][(p+1)/2].number*p
                                < data.at(r).at(c)*p
                            ||
                            toInitialize[c][(p+1)/2]
                        )
                    ){
                        plotRanges[c][(p+1)/2].number = data.at(r).at(c) ;
                        toInitialize[c][(p+1)/2] = false ;
                    }
                }
            }
        }
        for(unsigned int c=0; c<3; c++){
            getLimits( 
                        plotRanges[c][0].number , plotRanges[c][1].number ,
                        x                       , y
                    ) ;
            if( !( plotRanges[c][0].setting ) ) plotRanges[c][0].number = x ;
            if( !( plotRanges[c][1].setting ) ) plotRanges[c][1].number = y ;
        }
    }

    for(unsigned int c=0; c<3; c++){
        if( useLogScale[c] && plotRanges[c][0].number <= 0. ){
            cout << " ERROR : negativ plotting-range" ;
            cout <<         " not possible with log-scale " ;
            cout <<         " ( axis " << c << " ) " << endl ;
            return 3 ;
        }
    }
    
    double 
            offset   = plotRanges[2][0].number , 
            distance = plotRanges[2][1].number - plotRanges[2][0].number ;
    if( useLogScale[2] ){
        offset   = TMath::Log10( offset   ) ;
        distance = TMath::Log10( distance ) ;
    }
    double toCompare ;
    for(unsigned int r=0; r<nRows; r++){
        toCompare = data.at(r).at(2) ;
        if( useLogScale[2] ) toCompare = TMath::Log10( toCompare ) ;
        colorNumber = (int)(
            ( toCompare - offset ) / distance * (double)( nContours )
        ) ;
        if( colorNumber < 0 ) continue ;
        if( colorNumber > nContours-1 ) colorNumber = nContours-1 ;
        if( graphMap.find( colorNumber ) == graphMap.end() ){
            graphMap[ colorNumber ] = new TGraphErrors() ;
            name = "" ;
            name += colorNumber ;
            graphMap[ colorNumber ]->SetName(  name ) ;
            graphMap[ colorNumber ]->SetTitle( name ) ;
            graphMap[ colorNumber ]->SetMarkerStyle(8) ;
            graphMap[ colorNumber ]->SetMarkerSize( markerSize ) ;
            graphMap[ colorNumber ]
                        ->SetMarkerColor( colorList[ colorNumber ] ) ;
            graphMap[ colorNumber ]
                        ->SetLineColor(   colorList[ colorNumber ] ) ;
        }
        graphMap[ colorNumber ]->SetPoint(
            graphMap[ colorNumber ]->GetN() ,
            data.at(r).at(0) , data.at(r).at(1)
        ) ;
    }

    TH2D * valueMap = new TH2D( 
        "valueMap" , "valueMap" , 
        nRows , plotRanges[0][0].number , plotRanges[0][1].number ,
        nRows , plotRanges[1][0].number , plotRanges[1][1].number 
    ) ;
    valueMap->GetXaxis()->SetTitle( axisTitle[0] ) ;
    valueMap->GetYaxis()->SetTitle( axisTitle[1] ) ;
    valueMap->GetZaxis()->SetTitle( axisTitle[2] ) ;
    if( plotRanges[0][2].setting )
        valueMap->GetXaxis()->SetNdivisions( plotRanges[0][2].number ) ;
    if( plotRanges[1][2].setting )
        valueMap->GetYaxis()->SetNdivisions( plotRanges[1][2].number ) ;
    if( plotRanges[2][2].setting )
        valueMap->GetZaxis()->SetNdivisions( plotRanges[2][2].number ) ;
    valueMap->SetMinimum( plotRanges[2][0].number ) ;
    valueMap->SetMaximum( plotRanges[2][1].number ) ;
    valueMap->SetEntries(1) ;

    TCanvas * can = new TCanvas( 
                                    outname , outname , 
                                    800 , 600 
                                ) ;

    if( drawGrid[0] ) can->SetGridx();
    if( drawGrid[1] ) can->SetGridy();

    if( useLogScale[0] ) can->SetLogx() ; 
    if( useLogScale[1] ) can->SetLogy() ; 
    if( useLogScale[2] ) can->SetLogz() ; 

    valueMap->Draw("COLZ1") ;
    for( auto g : graphMap )
        g.second->Draw("Psame") ;

    showing() ;

    outname += "_valueMap.pdf" ;
    
    cout << " writing ... " ;
 
    can->Print( outname ) ;
    
    can->Close() ;

    cout << " done " << endl ;

    valueMap->Delete() ;
    for( auto g : graphMap )
        g.second->Delete() ;

    return 0 ;

}

