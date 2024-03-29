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
    TString histname = argv[2] ;
    
    bool skipShowing = false ;
    
    vector<string> parameter = {
        "xAxis" ,
        "yAxis" ,
        "statBox" ,
        "xTitle" ,
        "yTitle" ,
        "lineStyle" ,
        "plotOptions" ,
        "markerStyle" ,
        "dataFormat",
        "binning"
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
    plotOptions( false , values["statBox"].setting ) ;

    if( values["statBox"].setting ){
        gStyle->SetOptStat( atoi( values["statBox"].specifier.c_str() ) ) ;
        gStyle->SetPadRightMargin( 0.20 ) ;
    }
    else gStyle->SetOptStat( 0 ) ;
    
    TH1D * hist ;
    
    if( filename.EndsWith(".root") ){
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
        hist = (TH1D*)input->Get(histname) ;
        hist->SetDirectory(0) ;
        input->Close() ;
        if( values["binning"].setting && values["binning"].number >= 2 )
            hist->Rebin( (unsigned int)values["binning"].number ) ;
    }
    else{
        if( !( values["xAxis"].setting ) || !( values["binning"].setting ) ){
            cout << " ERROR : x-axis range and binning required " << endl ;
            return 4 ;
        }
        AxisSettings axe(
                            values["xTitle"].specifier ,
                            values["xAxis" ].specifier
                        ) ;
        hist = new TH1D(
                            histname , histname ,
                            atoi( values["binning"].specifier.c_str() ) ,
                            axe.low , axe.high
                        ) ;
        vector< vector<string> > data = getInput( filename.Data() ) ;
        unsigned int nData = data.size() ;
        if( nData < 1 ){
            cout << " ERROR : data empty " << endl ;
            return 5 ;
        }
        TString dataFormat = values["dataFormat"].specifier ;
        bool rowNOTcolumn   = false ;
        bool weightNOTvalue = false ;
        bool allEntries     = false ;
        if( dataFormat.Contains("ROW")    ) rowNOTcolumn   = true ;
        if( dataFormat.Contains("WEIGHT") ) weightNOTvalue = true ;
        if( dataFormat.Contains("ALL")    ) allEntries     = true ;
        unsigned int sliceDefault = 0 ;
        TString sliceNumber = dataFormat ;
        sliceNumber = sliceNumber.ReplaceAll( "COLUMN" , "" ) ;
        sliceNumber = sliceNumber.ReplaceAll( "ROW"    , "" ) ;
        sliceNumber = sliceNumber.ReplaceAll( "VALUE"  , "" ) ;
        sliceNumber = sliceNumber.ReplaceAll( "WEIGHT" , "" ) ;
        sliceNumber = sliceNumber.ReplaceAll( "ALL"    , "" ) ;
        if( sliceNumber.Length() > 0 )
            sliceDefault = atoi( sliceNumber.Data() ) ;
        double value ;
        if( rowNOTcolumn && !( allEntries )  ){
            if( nData > sliceDefault ) nData = data.at(sliceDefault).size() ;
            else{
                cout << " ERROR : data provides not enough rows " << endl ;
                return 5 ;
            }
        }
        for(unsigned int d=0; d<nData; d++){
            if( allEntries ){
                sliceDefault = data.at(d).size() ;
                for(unsigned int c=0; c<sliceDefault; c++)
                    hist->Fill( atof( data.at(d).at(c).c_str() ) ) ;
                continue ;
            }
            if( rowNOTcolumn )
                value = atof( data.at(sliceDefault).at(d).c_str() ) ;
            else{
                if( data.at(d).size() > sliceDefault )
                    value = atof( data.at(d).at(sliceDefault).c_str() ) ;
                else continue ;
            }
            if( weightNOTvalue ) hist->SetBinContent( d , value ) ;
            else                 hist->Fill( value ) ;
        }
    }

    TString name = filename ;
    if( name.Contains(".") ) 
        name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) 
        name = name( name.Last('/')+1 , name.Sizeof() ) ;
    name += "_" ;
    name += histname ;
    name = replaceBadChars( name );
    
    unsigned int nBins = hist->GetXaxis()->GetNbins() ;
    double plotRanges[2][2] ;
    plotRanges[0][0] = hist->GetXaxis()->GetXmin() ;
    plotRanges[0][1] = hist->GetXaxis()->GetXmax() ;
    plotRanges[1][0] = hist->GetMinimum() ;
    plotRanges[1][1] = hist->GetMaximum() ;
    
    double s , x , y ;
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
            if( 
                !( toDiscard( axis[a].low  ) ) 
                && 
                ( a != 0 || plotRanges[a][0] < axis[a].low )
            ) 
                plotRanges[a][0] = axis[a].low  ;
            if( 
                !( toDiscard( axis[a].high ) ) 
                &&
                ( a != 0 || plotRanges[a][1] > axis[a].high )
            ) 
                plotRanges[a][1] = axis[a].high ;
        }
    }

    if( values["xTitle"].setting ) 
        hist->GetXaxis()->SetTitle( values["xTitle"].specifier.c_str() ) ;
    if( values["yTitle"].setting ) 
        hist->GetYaxis()->SetTitle( values["yTitle"].specifier.c_str() ) ;
    
    unsigned int lineStyle = 1 , lineColor = 1 , lineWidth = 2 ;
    if( values["lineStyle"].setting ){
        s = getNumberWithRange( values["lineStyle"].specifier , x , y ) ;
        if( !( toDiscard( s ) ) ) lineStyle = s ;
        if( !( toDiscard( x ) ) ) lineColor = x ;
        if( !( toDiscard( y ) ) ) lineWidth = y ;
    }
    if( lineStyle > 0 ){
        hist->SetLineStyle( lineStyle ) ;
        hist->SetLineColor( lineColor ) ;
        hist->SetLineWidth( lineWidth ) ;
    }
    
    unsigned int markerStyle = 8 , markerColor = lineColor ;
    double markerSize = 1. ;
    if( values["markerStyle"].setting ){
        s = getNumberWithRange( values["markerStyle"].specifier , x , y ) ;
        if( !( toDiscard( s ) ) ) markerStyle = s ;
        if( !( toDiscard( x ) ) ) markerColor = x ;
        if( !( toDiscard( y ) ) ) markerSize  = y ;
    }
    hist->SetMarkerStyle( markerStyle ) ;
    hist->SetMarkerColor( markerColor ) ;
    hist->SetMarkerSize(  markerSize  ) ;
   
    unsigned int canvasSizes[2] = { 800 , 600 } ;
    if( values["statBox"].setting ) canvasSizes[0] = 1000 ;
    TCanvas * can = new TCanvas( 
                                    name , name , 
                                    canvasSizes[0] , canvasSizes[1] 
                                ) ;
    if( axis[0].logarithmic ) can->SetLogx() ; 
    if( axis[1].logarithmic ) can->SetLogy() ; 

    double under , over ;
    getOutflow( hist , plotRanges[0][0] , plotRanges[0][1] , under , over ) ;
    
    if( values["plotOptions"].setting )
        hist->Draw( values["plotOptions"].specifier.c_str() ) ;
    else hist->Draw() ;
    hist->GetXaxis()->SetRangeUser( plotRanges[0][0] , plotRanges[0][1] ) ;
    hist->GetYaxis()->SetRangeUser( plotRanges[1][0] , plotRanges[1][1] ) ;
    hist->GetXaxis()->SetNdivisions( axis[0].divisions ) ; 
    hist->GetYaxis()->SetNdivisions( axis[1].divisions ) ; 
    if( axis[0].grid ) gPad->SetGridx() ;
    if( axis[1].grid ) gPad->SetGridy() ;

    hist->SetBinContent( 0       , under ) ;
    hist->SetBinContent( nBins+1 , over  ) ;
   
    if( values["statBox"].setting ){
        gPad->Modified() ;
        gPad->Update() ;
        TPaveStats * box = (TPaveStats*)hist->FindObject("stats") ;           
        box->SetX1NDC( 0.803 ) ;
        box->SetX2NDC( 0.997 ) ; 
        box->SetY1NDC( 0.120 ) ;
        box->SetY2NDC( 0.940 ) ; 
    }
     
    if( !( skipShowing ) ) showing() ;
    
    name += ".pdf" ;
        
    can->Print( name ) ;
    
    can->Close() ;

    hist->Delete() ;

    return 0 ;

}
