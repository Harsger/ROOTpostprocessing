#include "postprocessing.h"

using namespace std;

class AxisSettings{
public:
    
    string title ;
    double low , high ;
    unsigned int divisions ;
    bool logarithmic , grid ;
    
    AxisSettings(){
        title = "" ;
        low = nan("") ;
        high = nan("") ;
        divisions = 510 ;
        logarithmic = false ;
        grid = false ;
    }
    
    AxisSettings( string t , string settings ){
        title = t ;
        if( settings.find("log") != std::string::npos ){
            logarithmic = true ;
            settings.erase( settings.find("log") , 3 ) ;
        }
        else logarithmic = false ;
        if( settings.find("grid") != std::string::npos ){
            grid = true ;
            settings.erase( settings.find("grid") , 4 ) ;
        }
        else grid = false ;
        divisions = 510 ;
        double div = getNumberWithRange( settings , low , high ) ;
        if( !( toDiscard( div ) ) ) divisions = (unsigned int)div ;
    }
    
} ;

int main(int argc, char *argv[]){

    if( argc < 3 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

    TString filename  = argv[1] ;
    TString graphname = argv[2] ;
    
    bool skipShowing = false ;
    
    vector<string> parameter = {
        "xAxis" ,
        "yAxis" ,
        "markerStyle" ,
        "lineStyle" ,
        "xTitle" ,
        "yTitle" 
    } ;
    map< string , SpecifiedNumber > values ;
    
    for(unsigned int p=0; p<parameter.size(); p++){
        values[parameter.at(p)] = SpecifiedNumber() ;
        if( argc > p+3 ){
            string argument = argv[p+3] ; 
            if( argument == "%" ) continue ;
            if( argument == "skip" ) skipShowing = true ;
            else 
                values[parameter.at(p)] = 
                                SpecifiedNumber( argument.length() ) ;
                values[parameter.at(p)].specifier = argument ;
        }
    }
    if( 
        argc > parameter.size()+3 
        && 
        string( argv[parameter.size()+3] ) == "skip" 
    )
        skipShowing = true ;
                  
    TApplication app("app", &argc, argv) ;    
    plotOptions() ;
    gStyle->SetOptStat(0) ;
    
    TFile * input = new TFile(filename,"READ") ;
    if( input->IsZombie() ){
        cout << " ERROR : opening " << filename << endl ;
        return 2 ;
    }
    
    if( input->Get(graphname) == NULL ){
        cout << " ERROR : reading " << graphname 
                << " in " << input->GetName() << endl ;
        return 3 ;
    }
    TGraphErrors * graph = (TGraphErrors*)input->Get(graphname) ;
    input->Close() ;

    TString name = filename ;
    if( name.Contains(".") ) 
        name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) 
        name = name( name.Last('/')+1 , name.Sizeof() ) ;
    name += "_" ;
    name += graphname ;
    name = replaceBadChars( name );
    
    double plotRanges[2][2] ;
    unsigned int nPoints = graph->GetN() ;
    
    double x , y ;
    if( nPoints > 0 ){
        graph->GetPoint( 0 , x , y ) ;
        plotRanges[0][0] = x ;
        plotRanges[0][1] = x ;
        plotRanges[1][0] = y ;
        plotRanges[1][1] = y ;
    }
    for(unsigned int p=1; p<nPoints; p++){
        graph->GetPoint( p , x , y ) ;
        if( plotRanges[0][0] > x ) plotRanges[0][0] = x ;
        if( plotRanges[0][1] < x ) plotRanges[0][1] = x ;
        if( plotRanges[1][0] > y ) plotRanges[1][0] = y ;
        if( plotRanges[1][1] < y ) plotRanges[1][1] = y ;
    }
    getLimits( plotRanges[0][0] , plotRanges[0][1] , x , y ) ;
    plotRanges[0][0] = x ;
    plotRanges[0][1] = y ;
    getLimits( plotRanges[1][0] , plotRanges[1][1] , x , y ) ;
    plotRanges[1][0] = x ;
    plotRanges[1][1] = y ;
    
    AxisSettings axis[2] ;
    for(unsigned int a=0; a<2; a++){
        string dir = "x" ;
        if( a == 1 ) dir = "y" ;
        if( values[dir+"Axis"].setting && values[dir+"Axis"].number > 0 ){
            axis[a] = AxisSettings( 
                                    values[dir+"Title"].specifier , 
                                    values[dir+"Axis" ].specifier 
                                ) ;
            if( !( toDiscard( axis[a].low  ) ) ) 
                plotRanges[a][0] = axis[a].low  ;
            if( !( toDiscard( axis[a].high ) ) ) 
                plotRanges[a][1] = axis[a].high ;
        }
    }

    if( values["xTitle"].setting ) 
        graph->GetXaxis()->SetTitle( values["xTitle"].specifier.c_str() ) ;
    if( values["yTitle"].setting ) 
        graph->GetYaxis()->SetTitle( values["yTitle"].specifier.c_str() ) ;
    
    unsigned int markerStyle = 8 , markerColor = 1 ;
    double markerSize = 1. ;
    if( values["markerStyle"].setting ){
        double s = 
            getNumberWithRange( values["markerStyle"].specifier , x , y ) ;
        if( !( toDiscard( s ) ) ) markerStyle = s ;
        if( !( toDiscard( x ) ) ) markerColor = x ;
        if( !( toDiscard( y ) ) ) markerSize  = y ;
    }
    graph->SetMarkerStyle( markerStyle ) ;
    graph->SetMarkerColor( markerColor ) ;
    graph->SetMarkerSize(  markerSize  ) ;
    
    unsigned int lineStyle = 0 , lineColor = markerColor , lineWidth = 2 ;
    if( values["lineStyle"].setting ){
        double s = 
            getNumberWithRange( values["lineStyle"].specifier , x , y ) ;
        if( !( toDiscard( s ) ) ) lineStyle = s ;
        if( !( toDiscard( x ) ) ) lineColor = x ;
        if( !( toDiscard( y ) ) ) lineWidth = y ;
    }
    string plotString = "AP" ;
    if( lineStyle > 0 ){
        plotString += "L" ;
        graph->SetLineStyle( lineStyle ) ;
        graph->SetLineColor( lineColor ) ;
        graph->SetLineWidth( lineWidth ) ;
    }
    
    TCanvas * can = new TCanvas( name , name , 800 , 600 ) ;
    if( axis[0].logarithmic ) can->SetLogx() ; 
    if( axis[1].logarithmic ) can->SetLogy() ; 
    
    graph->Draw( plotString.c_str() ) ;
    graph->GetXaxis()->SetRangeUser( plotRanges[0][0] , plotRanges[0][1] ) ;
    graph->GetYaxis()->SetRangeUser( plotRanges[1][0] , plotRanges[1][1] ) ;
    graph->GetXaxis()->SetNdivisions( axis[0].divisions ) ; 
    graph->GetYaxis()->SetNdivisions( axis[1].divisions ) ; 
    if( axis[0].grid ) gPad->SetGridx() ;
    if( axis[1].grid ) gPad->SetGridy() ;
        
    if( !( skipShowing ) ) showing() ;
    
    name += ".pdf" ;
        
    can->Print( name ) ;
    
    can->Close() ;

    graph->Delete() ;

    return 0 ;

}
