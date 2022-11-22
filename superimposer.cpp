#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

    TString filename = argv[1] ;
    
    bool toShow = true ;
    if( argc > 2 ) toShow = false ;

    vector< vector<string> > parameter = getInput( filename.Data() );

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };

    vector< vector<string> > filesNgraphsNtitles ;
    vector<string> strVecDummy ;
    vector< vector<SpecifiedNumber> > markerNcolorNline ;
    vector<SpecifiedNumber> specVecDummy ;
    string axisTitles[2] = { neverUse , neverUse } ;
    SpecifiedNumber plotRanges[2][2] ;
    bool useLogScale[2] = { false , false } ;
    bool skipErrors = false ;
    SpecifiedNumber textDataFormat ;
    SpecifiedNumber divisions[2] ;
    double markerSize = 1. ;
    SpecifiedNumber legendText ;
    SpecifiedNumber legendPosition ;
    SpecifiedNumber colorPalette ;
    bool broadCanvas = false ;
    SpecifiedNumber replaceNaN[2] ;
    bool removeNaN = false ;
    SpecifiedNumber fitting ;
    vector< vector<SpecifiedNumber> > startParameter ;
    double value , low , high ;
    TString saveAS = "pdf" ;

    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//            cout << " comment line " << r << endl ;
            continue ;
        }

        int fileORgraph = -1 ;

        if( parameter.at(r).at(0).compare("FILE") == 0  ) fileORgraph = 0 ;
        else if( parameter.at(r).at(0).compare("GRAPH") == 0  ) fileORgraph = 1 ;

        if( fileORgraph > -1 && parameter.at(r).size() > 2 ){
            preNsuffix[fileORgraph][0] = parameter.at(r).at(1) ;
            preNsuffix[fileORgraph][1] = parameter.at(r).at(2) ;
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
            TString( parameter.at(r).at(0) ).BeginsWith("AXIS") 
            && 
            (
                TString( parameter.at(r).at(0) ).EndsWith("X") 
                ||
                TString( parameter.at(r).at(0) ).EndsWith("Y") 
            )
            &&
            parameter.at(r).size() > 1
        ){
            unsigned int ax = 0 ;
            if( TString( parameter.at(r).at(0) ).EndsWith("Y") ) ax = 1;
            axisTitles[ax] = parameter.at(r).at(1) ;
            for(unsigned int c=2; c<parameter.at(r).size(); c++){
                axisTitles[ax] += " ";
                axisTitles[ax] += parameter.at(r).at(c) ;
            }
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
        
        if( parameter.at(r).at(0).compare("NOERRORS") == 0 ){
            skipErrors = true ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("FORMAT") == 0 
            &&
            parameter.at(r).size() > 1
        ){
            textDataFormat = SpecifiedNumber(0.) ;
            textDataFormat.specifier = parameter.at(r).at(1) ;
            for(unsigned int c=2; c<parameter.at(r).size(); c++){
                textDataFormat.specifier += " " ;
                textDataFormat.specifier += parameter.at(r).at(c) ;
            }
            continue ;
        }

        if(
            parameter.at(r).at(0).compare("DIVISIONS") == 0
            &&
            parameter.at(r).size() > 2
        ){
            if( parameter.at(r).at(1).compare("%") != 0 )
                divisions[0] = SpecifiedNumber( atoi(
                                                parameter.at(r).at(1).c_str()
                                            ) ) ;
            if( parameter.at(r).at(2).compare("%") != 0 )
                divisions[1] = SpecifiedNumber( atoi(
                                                parameter.at(r).at(2).c_str()
                                            ) ) ;
            continue ;
        }

        if(
            parameter.at(r).at(0).compare("MARKERSIZE") == 0 
            &&
            parameter.at(r).size() > 1
        ){
            markerSize = atof( parameter.at(r).at(1).c_str() ) ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("LEGEND") == 0 
            &&
            parameter.at(r).size() > 1
        ){
            legendText = SpecifiedNumber( 0. ) ;
            legendText.specifier = parameter.at(r).at(1) ;
            for(unsigned int w=2; w<parameter.at(r).size(); w++){
                legendText.specifier += " " ;
                legendText.specifier += parameter.at(r).at(w) ;
                
            }
            continue ;
        }
        
        if( parameter.at(r).at(0).compare("LEGENDPOSITION") == 0 ){
            legendPosition = SpecifiedNumber( 0. ) ;
            if( parameter.at(r).size() > 1 ){
                legendPosition.specifier = parameter.at(r).at(1) ;
                for(unsigned int w=2; w<parameter.at(r).size(); w++){
                    legendPosition.specifier += " " ;
                    legendPosition.specifier += parameter.at(r).at(w) ;
                    
                }
            }
            continue ;
        }

        if( parameter.at(r).at(0).compare("BROADCANVAS") == 0 ){
            broadCanvas = true ;
            continue ;
        }
        
        if( parameter.at(r).at(0).compare("REPLACENAN") == 0 ){
            if( parameter.at(r).size() > 2 ){
                if( parameter.at(r).at(1).compare("%") != 0 ) 
                    replaceNaN[0] = SpecifiedNumber( atof( 
                                                parameter.at(r).at(1).c_str() 
                                            ) ) ;
                if( parameter.at(r).at(2).compare("%") != 0 )
                    replaceNaN[1] = SpecifiedNumber( atof( 
                                                parameter.at(r).at(2).c_str() 
                                            ) ) ;
            }
            else{
                removeNaN = true ;
                replaceNaN[0] = SpecifiedNumber( 0. ) ;
                replaceNaN[1] = SpecifiedNumber( 0. ) ;
            }
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
            parameter.at(r).at(0).compare("SAVEAS") == 0
            &&
            parameter.at(r).size() > 1
        ){
            saveAS = parameter.at(r).at(1).c_str() ;
            continue ;
        }

        if(
            parameter.at(r).at(0).compare("FUNCTION") == 0
            &&
            parameter.at(r).size() > 1
        ){
            fitting = SpecifiedNumber( 0. ) ;
            fitting.specifier = parameter.at(r).at(1) ;
            for(unsigned int c=2; c<parameter.at(r).size(); c++){
                value = getNumberWithRange(
                                            parameter.at(r).at(c) , low , high
                                          ) ;
                specVecDummy.clear() ;
                if( toDiscard( value ) )
                    specVecDummy.push_back( SpecifiedNumber() ) ;
                else
                    specVecDummy.push_back( SpecifiedNumber( value ) ) ;
                if( toDiscard( low ) )
                    specVecDummy.push_back( SpecifiedNumber() ) ;
                else
                    specVecDummy.push_back( SpecifiedNumber( low ) ) ;
                if( toDiscard( high ) )
                    specVecDummy.push_back( SpecifiedNumber() ) ;
                else
                    specVecDummy.push_back( SpecifiedNumber( high ) ) ;
                startParameter.push_back( specVecDummy ) ;
                specVecDummy.clear() ;
            }
            continue ;
        }

        if( parameter.at(r).size() > 2 ){
            strVecDummy.push_back( parameter.at(r).at(0) );
            strVecDummy.push_back( parameter.at(r).at(1) );
            strVecDummy.push_back( parameter.at(r).at(2) );
            filesNgraphsNtitles.push_back( strVecDummy ) ;
            strVecDummy.clear() ;
            if( 
                parameter.at(r).size() > 3 
                &&  
                parameter.at(r).at(3).compare("%") != 0
            )
                specVecDummy.push_back( SpecifiedNumber(
                    atof( parameter.at(r).at(3).c_str() )
                ) ) ;
            else specVecDummy.push_back( SpecifiedNumber() ) ;
            if( 
                parameter.at(r).size() > 4 
                &&  
                parameter.at(r).at(4).compare("%") != 0
            )
                specVecDummy.push_back( SpecifiedNumber(
                    atof( parameter.at(r).at(4).c_str() )
                ) ) ;
            else specVecDummy.push_back( SpecifiedNumber() ) ;
            if( 
                parameter.at(r).size() > 5 
                &&  
                parameter.at(r).at(5).compare("%") != 0
            )
                specVecDummy.push_back( SpecifiedNumber(
                    atof( parameter.at(r).at(5).c_str() )
                ) ) ;
            else specVecDummy.push_back( SpecifiedNumber() ) ;
            markerNcolorNline.push_back( specVecDummy ) ;
            specVecDummy.clear() ;
                
        }

    }

    unsigned int nGraphs = filesNgraphsNtitles.size() ;
    
    if( nGraphs < 1 ){
        cout << " ERROR : no graphs specified " << endl ;
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

    for(unsigned int r=0; r<nGraphs; r++){
        for(unsigned int c=0; c<3; c++){
            if( filesNgraphsNtitles.at(r).at(c).compare( "%" ) == 0 )
                filesNgraphsNtitles.at(r).at(c) = "" ;
        }
    }
    
    TGraphErrors ** graphs = new TGraphErrors*[nGraphs] ;
    TString name ;
    unsigned int notFound = 0 ;
    bool useable[nGraphs] ;
    
    for(unsigned int r=0; r<nGraphs; r++){

        name = preNsuffix[0][0] ;
        name += filesNgraphsNtitles.at(r).at(0) ;
        name += preNsuffix[0][1] ;
        
        if( name.EndsWith(".root") ){
        
            TFile * input = new TFile(name,"READ") ;
            if( input->IsZombie() ){
                cout << " ERROR : opening " << name << endl ;
                notFound++ ;
                useable[r] = false ;
                continue ;
            }

            name = preNsuffix[1][0] ;
            name += filesNgraphsNtitles.at(r).at(1) ;
            name += preNsuffix[1][1] ;
            if( input->Get(name) == NULL ){
                cout << " ERROR : reading " << name 
                    << " in " << input->GetName() << endl ;
                notFound++ ;
                useable[r] = false ;
                continue ;
            }
            graphs[r] = (TGraphErrors*)input->Get(name) ;

            input->Close() ;
        
        }
        else{
            
            if( textDataFormat.setting )
                graphs[r] = new TGraphErrors(
                                                name , 
                                                textDataFormat.specifier.c_str() 
                                            ) ;
            else
                graphs[r] = new TGraphErrors(name) ;
            
        }
            
        if( graphs[r]->GetN() < 1 ){
            cout << " ERROR : " << name 
                << " is empty " << endl ;
            notFound++ ;
            useable[r] = false ;
            continue ;
        }
        
        graphs[r]->SetName( filesNgraphsNtitles.at(r).at(2).c_str() ) ;
        graphs[r]->SetTitle( filesNgraphsNtitles.at(r).at(2).c_str() ) ;

        useable[r] = true ;

    }
  
    if( notFound == nGraphs ){
        cout << " ERROR : no graphs available " << endl ;
        return 3 ;
    }

    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TString outname = name ;
    
    for(unsigned int g=0; g<nGraphs; g++){
        if( ! useable[g] ) continue ;
        unsigned int nPoints = graphs[g]->GetN() ;
        double x , y ;
        for(int p=0; p<nPoints; p++){
            graphs[g]->GetPoint( p , x , y ) ;
            if( replaceNaN[0].setting || replaceNaN[1].setting ){
                if( toDiscard( x ) ){
                    if( removeNaN ){
                        cout 
                            << " remove in graph " << g 
                            << " (title: " << filesNgraphsNtitles.at(g).at(2) 
                            << ") point " << p 
                            << " due to NaN in X " << endl ; 
                        graphs[g]->RemovePoint( p );
                        p--;
                        nPoints--;
                        continue;
                    }
                    else if( replaceNaN[0].setting ){
                        cout 
                            << " replace X in graph " << g 
                            << " (title: " << filesNgraphsNtitles.at(g).at(2) 
                            << ") of point " << p << endl ; 
                        x = replaceNaN[0].number ;
                        graphs[g]->SetPoint( p , x , y ) ;
                    }
                }
                if( toDiscard( y ) ){
                    if( removeNaN ){
                        cout 
                            << " remove in graph " << g 
                            << " (title: " << filesNgraphsNtitles.at(g).at(2) 
                            << ") point " << p 
                            << " due to NaN in Y " << endl ; 
                        graphs[g]->RemovePoint( p );
                        p--;
                        nPoints--;
                        continue;
                    }
                    else if( replaceNaN[1].setting ){
                        cout 
                            << " replace Y in graph " << g 
                            << " (title: " << filesNgraphsNtitles.at(g).at(2) 
                            << ") of point " << p << endl ; 
                        y = replaceNaN[1].number ;
                        graphs[g]->SetPoint( p , x , y ) ;
                    }
                }
            }
            if( ! plotRanges[0][0].setting ){
                if( plotRanges[0][0].specifier.empty() ){
                    plotRanges[0][0].specifier = "xlow" ;
                    plotRanges[0][0].number = x ;
                }
                else if( plotRanges[0][0].number > x )
                    plotRanges[0][0].number = x ;
            }
            if( ! plotRanges[0][1].setting ){
                if( plotRanges[0][1].specifier.empty() ){
                    plotRanges[0][1].specifier = "xhigh" ;
                    plotRanges[0][1].number = x ;
                }
                else if( plotRanges[0][1].number < x )
                    plotRanges[0][1].number = x ;
            }
            if( ! plotRanges[1][0].setting ){
                if( plotRanges[1][0].specifier.empty() ){
                    plotRanges[1][0].specifier = "ylow" ;
                    plotRanges[1][0].number = y ;
                }
                else if( plotRanges[1][0].number > y )
                    plotRanges[1][0].number = y ;
            }
            if( ! plotRanges[1][1].setting ){
                if( plotRanges[1][1].specifier.empty() ){
                    plotRanges[1][1].specifier = "yhigh" ;
                    plotRanges[1][1].number = y ;
                }
                else if( plotRanges[1][1].number < y )
                    plotRanges[1][1].number = y ;
            }
        }
    }
    
    TGraphErrors * extrema = new TGraphErrors() ;
    extrema->SetName( "extrema" ) ;
    extrema->SetTitle( "extrema" ) ;
    if( legendText.setting ){
        extrema->SetName( legendText.specifier.c_str() ) ;
        extrema->SetTitle( legendText.specifier.c_str() ) ;
    }
    extrema->SetPoint( 
                        extrema->GetN() , 
                        plotRanges[0][0].number , 
                        plotRanges[1][0].number 
                     ) ;
    extrema->SetPoint( 
                        extrema->GetN() , 
                        plotRanges[0][0].number , 
                        plotRanges[1][1].number 
                     ) ;
    extrema->SetPoint( 
                        extrema->GetN() , 
                        plotRanges[0][1].number , 
                        plotRanges[1][1].number 
                     ) ;
    extrema->SetPoint( 
                        extrema->GetN() , 
                        plotRanges[0][1].number , 
                        plotRanges[1][0].number 
                     ) ;
    
    plotOptions() ;
    
    if( colorPalette.setting ){
        gStyle->SetPalette( colorPalette.number ) ;
        if( colorPalette.specifier.compare("inverted") == 0 )
            TColor::InvertPalette();
    }
    
    gStyle->SetOptStat(0) ;

    gStyle->SetPadTopMargin(    0.03 ) ;
    gStyle->SetPadRightMargin(  0.14 ) ;
    gStyle->SetPadBottomMargin( 0.12 ) ;
    gStyle->SetPadLeftMargin(   0.13 ) ;

    gStyle->SetTitleOffset( 1.1 , "x" ) ;
    gStyle->SetTitleOffset( 1.4 , "y" ) ;
    
    if( broadCanvas ){
        gStyle->SetPadRightMargin( 0.11 ) ;
        gStyle->SetPadLeftMargin(  0.06 ) ;
        gStyle->SetTitleOffset( 1.2 , "x" ) ;
        gStyle->SetTitleOffset( 0.6 , "y" ) ;
    }
    if( legendPosition.setting ) gStyle->SetPadRightMargin( 0.03 ) ;
    if( fitting.setting ){
        gStyle->SetOptFit(1) ;
        gStyle->SetPadRightMargin( 0.3 ) ;
        if( broadCanvas) gStyle->SetPadRightMargin( 0.2 ) ;
    }
    
    TApplication app("app", &argc, argv) ; 
    
    name = outname ;
    name = name.ReplaceAll( ".root" , "_canvas" ) ;
    
    unsigned int canvasSizes[2] = { 900 , 600 } ; 
    if( broadCanvas ){
        canvasSizes[0] = 2000 ;
        canvasSizes[1] = 500 ;
    }
    TCanvas * can = new TCanvas( 
                                    name , name , 
                                    canvasSizes[0] , canvasSizes[1] 
                               ) ; 
    if(useLogScale[0]) can->SetLogx() ; 
    if(useLogScale[1]) can->SetLogy() ; 
    
    extrema->SetMarkerStyle( 1 ) ;
    extrema->SetMarkerColor( 0 ) ;
    extrema->SetMarkerSize( markerSize ) ;
    extrema->SetLineColor( 0 ) ;
    extrema->Draw( "AP" ) ;
    extrema->GetXaxis()->SetTitle( axisTitles[0].c_str() ) ;
    extrema->GetYaxis()->SetTitle( axisTitles[1].c_str() ) ;
    if( plotRanges[0][0].setting || plotRanges[0][1].setting )
        extrema->GetXaxis()->SetRangeUser( 
                                                plotRanges[0][0].number , 
                                                plotRanges[0][1].number 
                                            ) ;
    else{
        getLimits( 
                    plotRanges[0][0].number , 
                    plotRanges[0][1].number , 
                    low , high 
                ) ;
        extrema->GetXaxis()->SetRangeUser( low , high ) ;
    }
    if( plotRanges[1][0].setting || plotRanges[1][1].setting )
        extrema->GetYaxis()->SetRangeUser( 
                                                plotRanges[1][0].number , 
                                                plotRanges[1][1].number 
                                            ) ;
    else{
        getLimits(
                    plotRanges[1][0].number ,
                    plotRanges[1][1].number ,
                    low , high
                ) ;
        extrema->GetYaxis()->SetRangeUser( low , high ) ;
    }   
    if( broadCanvas ) extrema->GetXaxis()->SetNdivisions(525) ; 
    else              extrema->GetXaxis()->SetNdivisions(505) ; 

    if( divisions[0].setting )
        extrema->GetXaxis()->SetNdivisions( divisions[0].number ) ;
    if( divisions[1].setting )
        extrema->GetYaxis()->SetNdivisions( divisions[1].number ) ;

    TF1 * function ;
    unsigned int count ;
    double chi2ndf ;

    for(unsigned int g=0; g<nGraphs; g++){
        
        if( !useable[g] ) continue ;
        
        if( fitting.setting ){
            name = "function" ;
            name += g ;
            function = new TF1(
                                name.Data() , fitting.specifier.c_str() ,
                                plotRanges[0][0].number ,
                                plotRanges[0][1].number
                              ) ;

            if( function->GetNpar() > 0. && startParameter.size() > 0. ){
                for(unsigned int p=0; p<startParameter.size(); p++){
                    if( p >= function->GetNpar() ) break ;
                    if( startParameter.at(p).size() > 2 ){
                        if( startParameter.at(p).at(0).setting )
                            function->SetParameter(
                                p , startParameter.at(p).at(0).number
                            ) ;
                        if(
                            startParameter.at(p).at(1).setting
                            &&
                            startParameter.at(p).at(2).setting
                        ){
                            if(
                                startParameter.at(p).at(1).number
                                ==
                                startParameter.at(p).at(2).number
                            )
                                function->FixParameter(
                                    p , startParameter.at(p).at(1).number
                                ) ;
                            else
                                function->SetParLimits(
                                    p ,
                                    startParameter.at(p).at(1).number ,
                                    startParameter.at(p).at(2).number
                                ) ;
                        }
                    }
                }
            }

            graphs[g]->Fit( function , "RQB" ) ;
    
            count = 1 ;
            chi2ndf = function->GetChisquare() / function->GetNDF() ;
            while( count < 10 && ( chi2ndf < 0.5 || chi2ndf > 2. ) ){
                graphs[g]->Fit( function , "RQB" ) ;
                chi2ndf = function->GetChisquare() / function->GetNDF() ;
                count++ ;
            }  
            graphs[g]->Fit( function , "RB" ) ;
        }

        name = "graph" ;
        name += g ;
        graphs[g]->SetName( name ) ;
        name = filesNgraphsNtitles.at(g).at(2) ;
        graphs[g]->SetTitle( name ) ;
        
        TString toAdd = "" ;
        
        if( markerNcolorNline.at(g).at(0).setting )
            graphs[g]->SetMarkerStyle( 
                (unsigned int)markerNcolorNline.at(g).at(0).number 
            ) ;
        else graphs[g]->SetMarkerStyle( 20 ) ;
        
        if( markerNcolorNline.at(g).at(1).setting ){
            graphs[g]->SetMarkerColor( 
                (unsigned int)markerNcolorNline.at(g).at(1).number 
            ) ;
            graphs[g]->SetLineColor( 
                (unsigned int)markerNcolorNline.at(g).at(1).number 
            ) ;
        }
        else toAdd += " PMC PLC " ;
        
        if( markerNcolorNline.at(g).at(2).setting ){ 
            graphs[g]->SetLineStyle( 
                (unsigned int)markerNcolorNline.at(g).at(2).number 
            ) ;
            name = "PLsame" ;
        }
        else name = "Psame" ;
        
        name += toAdd ;
        if( skipErrors ) name += " X " ;
        
        graphs[g]->SetMarkerSize( markerSize ) ;
        
        graphs[g]->Draw( name ) ;
        
    }
    
//     gPad->SetGridx() ;
    gPad->SetGridy() ;
    
    double legendEdges[4] = { 0.87 , 0.15 , 0.995 , 0.95 } ;
    if( broadCanvas ) legendEdges[0] = 0.90 ;
    if( legendPosition.setting ){
        legendEdges[0] = 0.76 ;
        legendEdges[2] = 0.96 ;
        if(      legendPosition.specifier.find( "top") != std::string::npos ) 
            legendEdges[1] = 0.55 ;
        else if( legendPosition.specifier.find( "bot") != std::string::npos ) 
            legendEdges[3] = 0.55 ;
        if(      legendPosition.specifier.find("left") != std::string::npos ){
            legendEdges[0] = 0.16 ;
            legendEdges[2] = 0.36 ;
            if( broadCanvas ){
                legendEdges[0] = 0.09  ;
                legendEdges[2] = 0.29 ;
            }
        }
    }
    
    TLegend * legend = can->BuildLegend( 
                                            legendEdges[0] , legendEdges[1] , 
                                            legendEdges[2] , legendEdges[3] 
                                       ) ;
//     legend->RecursiveRemove( extrema );
//     legend->Draw() ;

    if( ! legendText.setting ){
    
        TList * legendEntries = legend->GetListOfPrimitives();
        TIter next(legendEntries);
        TObject * obj;
        TLegendEntry * le;
        
        while( ( obj = next() ) ){
            le = (TLegendEntry*)obj ;
            name = le->GetLabel() ;
            if( name.CompareTo("extrema") == 0 )
                legendEntries->RemoveAt( legendEntries->IndexOf( obj ) ) ;
        }
        
    }

    if( fitting.setting ){

        gPad->Modified() ;
        gPad->Update() ;

        TPaveStats * box ;

        double boxRanges[2][2] = {
            { 0.71 , 0.99 } , { 0.12 , 0.97 }
        } ;
        if( broadCanvas ) boxRanges[0][0] = 0.81 ;
        double margin = 0.007 ;
        double boxHeight =
                            (
                                boxRanges[1][1] - boxRanges[1][0]
                                                - margin * ( nGraphs - 1. )
                            ) / (double)nGraphs ;
        double stepSize = boxHeight + margin  ;

        for(unsigned int g=0; g<nGraphs; g++){

            if( !useable[g] ) continue ;

            box = (TPaveStats*)graphs[g]->FindObject("stats") ;

            box->SetX1NDC( boxRanges[0][0] ) ;
            box->SetX2NDC( boxRanges[0][1] ) ;
            box->SetY1NDC( boxRanges[1][1] - stepSize * g - boxHeight ) ;
            box->SetY2NDC( boxRanges[1][1] - stepSize * g ) ;

        }
    }
        
    if( toShow ) showing() ;
    else{
        gPad->Modified() ;
        gPad->Update() ;
    }

    cout << " writing ... " ;
    
    name = outname ;
    saveAS = "." + saveAS ;
    name = name.ReplaceAll( ".root" , saveAS ) ;
    can->Print(name) ; 

    cout << " done " << endl ;

    return 0 ;

}
