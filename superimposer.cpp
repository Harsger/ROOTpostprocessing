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

    vector< vector<string> > filesNgraphsNtitles ;
    vector<string> strVecDummy ;
    vector< vector<SpecifiedNumber> > plotOptions ;
    vector<SpecifiedNumber> specVecDummy ;
    string axisTitles[2] = { neverUse , neverUse } ;
    SpecifiedNumber plotRanges[2][2] ;
    bool useLogScale[2] = { false , false } ;

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
            plotOptions.push_back( specVecDummy ) ;
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
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
    for(unsigned int g=0; g<nGraphs; g++){
        if( ! useable[g] ) continue ;
        unsigned int nPoints = graphs[g]->GetN() ;
        double x , y ;
        for(unsigned int p=0; p<nPoints; p++){
            graphs[g]->GetPoint( p , x , y ) ;
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
    
//     name = outfile->GetName() ;
//     name = name.ReplaceAll( ".root" , "" ) ;
    TGraphErrors * extrema = new TGraphErrors() ;
//     extrema->SetName( name ) ;
//     extrema->SetTitle( name ) ;
    extrema->SetName( "extrema" ) ;
    extrema->SetTitle( "extrema" ) ;
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
    
    name = outfile->GetName() ;
    name = name.ReplaceAll( ".root" , "_canvas" ) ;
    
    TCanvas * can = new TCanvas( name , name , 1500 , 500 ) ; 
    if(useLogScale[0]) can->SetLogx() ; 
    if(useLogScale[1]) can->SetLogy() ; 
    
    extrema->SetMarkerStyle( 1 ) ;
    extrema->SetMarkerColor( 0 ) ;
    extrema->SetLineColor( 0 ) ;
    extrema->Draw( "AP" ) ;
    extrema->GetXaxis()->SetTitle( axisTitles[0].c_str() ) ;
    extrema->GetYaxis()->SetTitle( axisTitles[1].c_str() ) ;
    if( plotRanges[0][0].setting || plotRanges[0][1].setting )
        extrema->GetXaxis()->SetRangeUser( 
                                                plotRanges[0][0].number , 
                                                plotRanges[0][1].number 
                                            ) ;
    if( plotRanges[1][0].setting || plotRanges[1][1].setting )
        extrema->GetYaxis()->SetRangeUser( 
                                                plotRanges[1][0].number , 
                                                plotRanges[1][1].number 
                                            ) ;
    
    for(unsigned int g=0; g<nGraphs; g++){
        
        if( !useable[g] ) continue ;
        
        name = "graph" ;
        name += g ;
        graphs[g]->SetName( name ) ;
        name = filesNgraphsNtitles.at(g).at(2) ;
        graphs[g]->SetTitle( name ) ;
        
        TString toAdd = "" ;
        
        if( plotOptions.at(g).at(0).setting )
            graphs[g]->SetMarkerStyle( 
                (unsigned int)plotOptions.at(g).at(0).number 
            ) ;
        else graphs[g]->SetMarkerStyle( 20 ) ;
        
        if( plotOptions.at(g).at(1).setting ){
            graphs[g]->SetMarkerColor( 
                (unsigned int)plotOptions.at(g).at(1).number 
            ) ;
            graphs[g]->SetLineColor( 
                (unsigned int)plotOptions.at(g).at(1).number 
            ) ;
        }
        else toAdd += " PMC PLC " ;
        
        if( plotOptions.at(g).at(2).setting ){ 
            graphs[g]->SetLineStyle( 
                (unsigned int)plotOptions.at(g).at(2).number 
            ) ;
            name = "PLsame" ;
        }
        else name = "Psame" ;
        
        name += toAdd ;
        
        graphs[g]->Draw( name ) ;
        
    }
    
//     gPad->SetGridx() ;
    gPad->SetGridy() ;
    
    TLegend * legend = can->BuildLegend( 0.90 , 0.15 , 0.995 , 0.95 ) ;
//     legend->RecursiveRemove( extrema );
//     legend->Draw() ;

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
