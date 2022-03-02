#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ) return 1 ;

    TString filename = argv[1] ;

    vector< vector<string> > parameter = getInput( filename.Data() );
    
    bool draw = true ;
    if( argc > 2 ) draw = false ;

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };
    string axisTitles[2] = { neverUse , neverUse } ;

    vector< vector<string> > filesNhists ;
    vector<string> strVecDummy ;
    vector< vector<double> > valueNerror ;
    vector<double> doVecDummy ;
    vector<double> toSkip ;
    SpecifiedNumber lowLimit , highLimit , plotRange[2][2] ;
    map< string , map< unsigned int , bool > > useRowsNcolumns ;
    map< string , vector<int> > givenRowsNcolumns ;
    string delimiter = " ";
    vector<string> axisLabels ;
    bool setLabels = false ;
    SpecifiedNumber labeblsotpion ;
    map< unsigned int , vector<unsigned int> > pixelList ;
    bool exclude = true ;

    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//            cout << " comment line " << r << endl ;
            continue ;
        }

        int specifier = -1 ;

        if( parameter.at(r).at(0).compare("FILE") == 0  ) specifier = 0 ;
        else if( parameter.at(r).at(0).compare("HIST") == 0  ) specifier = 1 ;
        else if( parameter.at(r).at(0).compare("AXIS") == 0  ) specifier = 2 ;
        else if( parameter.at(r).at(0).compare("RANGE") == 0  ) specifier = 3 ;

        if( specifier > -1 && parameter.at(r).size() > 2 ){
            if( specifier < 2 ){
                preNsuffix[specifier][0] = parameter.at(r).at(1) ;
                preNsuffix[specifier][1] = parameter.at(r).at(2) ;
            }
            else if( specifier == 2 ){
                axisTitles[0] = parameter.at(r).at(1) ;
                axisTitles[1] = parameter.at(r).at(2) ;
            }
            else if( specifier == 3 ){
                if( parameter.at(r).size() == 3 ){
                    plotRange[1][0] = SpecifiedNumber( 
                                        atof( parameter.at(r).at(1).c_str() ) 
                                    );
                    plotRange[1][1] = SpecifiedNumber( 
                                        atof( parameter.at(r).at(2).c_str() ) 
                                    );
                }
                else if( parameter.at(r).size() > 4 ){
                    for(unsigned int a=0; a<2; a++){
                        for(unsigned int l=0; l<2; l++){
                            unsigned int column = 1 + l + a * 2 ;
                            if( 
                                parameter.at(r)
                                         .at(column)
                                         .compare( "%" ) == 0 
                            ) continue ;
                            plotRange[a][l] 
                                = SpecifiedNumber( 
                                    atof( parameter.at(r).at(column).c_str() ) 
                                );
                        }
                    }
                }
            }
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("SKIP") == 0  
            &&
            parameter.at(r).size() > 1
        ){
            for(unsigned int c=1; c<parameter.at(r).size(); c++){
                if( parameter.at(r).at(c).rfind("<",0) == 0 ){
                    TString lowString = parameter.at(r).at(c) ;
                    lowString.ReplaceAll( "<" , "" ) ;
                    lowLimit.number = atof( lowString.Data() ) ;
                    lowLimit.specifier = "<" ;
                    lowLimit.setting = true ;
                }
                else if( parameter.at(r).at(c).rfind(">",0) == 0 ){
                    TString highString = parameter.at(r).at(c) ;
                    highString.ReplaceAll( ">" , "" ) ;
                    highLimit.number = atof( highString.Data() ) ;
                    highLimit.specifier = ">" ;
                    highLimit.setting = true ;
                }
                else
                    toSkip.push_back( atof( parameter.at(r).at(c).c_str() ) ) ;
            }
            continue ;
        }
        
        if( 
            (
                parameter.at(r).at(0).compare("ROWS") == 0  
                ||
                parameter.at(r).at(0).compare("COLUMNS") == 0  
            )
            &&
            parameter.at(r).size() > 1
        ){
            
            for(unsigned int c=1; c<parameter.at(r).size(); c++){
                
                TString tester = parameter.at(r).at(c) ;
                if( tester.EndsWith(".txt") ){
                    tester = filename ;
                    tester = tester( 0 , tester.Last('/')+1 ) ;
                    tester += parameter.at(r).at(c) ;
                    vector< vector<string> > numberStrings = 
                        getInput( tester.Data() ) ;
                    for(unsigned int n=0; n<numberStrings.size(); n++){
                        givenRowsNcolumns[parameter.at(r).at(0)].push_back(
                            atoi( numberStrings.at(n).at(0).c_str() )
                        );
                    }
                }
                else if( tester == "%" ) break ;
                else
                    givenRowsNcolumns[parameter.at(r).at(0)].push_back(
                        atoi( parameter.at(r).at(c).c_str() )
                    );
            }
            
            continue ;
            
        }

        if(
            parameter.at(r).at(0).compare("PIXELS") == 0  
            &&
            parameter.at(r).size() > 1
        ){

            vector< vector<string> > pixelStringList = 
                getInput( parameter.at(r).at(1) ) ;
            unsigned int nPixel = pixelStringList.size() ;
            if( nPixel == 0 ){
                TString pixelFile = filename ;
                pixelFile = pixelFile( 0 , pixelFile.Last('/')+1 ) ;
                pixelFile += parameter.at(r).at(1) ;
                pixelStringList = getInput( pixelFile.Data() ) ;
                nPixel = pixelStringList.size() ;
                if( nPixel > 0 )
                    cout << " -> found at parameter-file " << endl ;
            }
            for(unsigned int p=0; p<nPixel; p++){
                if( pixelStringList.size() < 2 ) continue ;
                pixelList[ 
                        atoi( pixelStringList.at(p).at(1).c_str() ) 
                    ]
                    .push_back( 
                        atoi( pixelStringList.at(p).at(0).c_str() ) 
                    ) ;
            }
            if( 
                parameter.at(r).size() > 2 
                && 
                parameter.at(r).at(2).compare("select") == 0 
            )
                exclude = false ;

            continue ;

        }
        
        if( 
            parameter.at(r).at(0).compare("DELIMITER") == 0  
            &&
            parameter.at(r).size() > 1
        ){
            delimiter = parameter.at(r).at(1) ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("LABELSOPTION") == 0  
            &&
            parameter.at(r).size() > 1
        ){
            labeblsotpion = SpecifiedNumber(0.);
            labeblsotpion.specifier = parameter.at(r).at(1) ;
            continue ;
        }
        
        if( parameter.at(r).size() > 2 ){
            strVecDummy.push_back( parameter.at(r).at(0) );
            strVecDummy.push_back( parameter.at(r).at(1) );
            filesNhists.push_back( strVecDummy ) ;
            strVecDummy.clear() ;
            doVecDummy.push_back( atof( parameter.at(r).at(2).c_str() ) );
            if( 
                parameter.at(r).size() > 3 
                &&
                parameter.at(r).at(3).compare( "%" ) != 0
            )
                doVecDummy.push_back( atof( parameter.at(r).at(3).c_str() ) );
            else
                doVecDummy.push_back( 0. ) ;
            valueNerror.push_back( doVecDummy ) ;
            doVecDummy.clear() ;
            if( 
                parameter.at(r).size() > 4 
                &&
                parameter.at(r).at(4).compare( "%" ) != 0
            ){
                axisLabels.push_back( parameter.at(r).at(4) );
                setLabels = true ;
            }
            else
                axisLabels.push_back( "" ) ;
        }

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

    unsigned int nHists = filesNhists.size() ;
    unsigned int unspecifiedHistname = 0 ;
    
    for(unsigned int r=0; r<nHists; r++){
        for(unsigned int c=0; c<2; c++){
            if( filesNhists.at(r).at(c).compare( "%" ) == 0 ){
                filesNhists.at(r).at(c) = "" ;
                if( c == 1 ) unspecifiedHistname++ ;
            }
        }
    }
    
    for(unsigned int c=0; c<2; c++){
        if( 
            axisTitles[c].compare( neverUse ) == 0 
            ||
            axisTitles[c].compare( "%" ) == 0
        )
            axisTitles[c] = "" ;
    }
    
    if( nHists < 1 ){
        cout << " ERROR : no histograms specified " << endl ;
        return 2 ;
    }
    
    TH2D * readhist ;
    TString name ;
    unsigned int rowsNcolumns[2] = { 0 , 0 } ;
    unsigned int notFound = 0 , number ;
    bool useable[nHists] ;
    double mean , stdv , min , max , median ;
    double extrema[2][2] = { { 0. , 0. } , { 0. , 0. } } ;
    bool toInitialize = true ;
    
    TGraphErrors  
                 * g_mean    = new TGraphErrors() ,
                 * g_stdv    = new TGraphErrors() ,
                 * g_min     = new TGraphErrors() ,
                 * g_max     = new TGraphErrors() ,
                 * g_median  = new TGraphErrors() ,
                 * g_number  = new TGraphErrors() ,
                 * g_extrema = new TGraphErrors() ;
    
    for(unsigned int h=0; h<nHists; h++){

        name = preNsuffix[0][0] ;
        name += filesNhists.at(h).at(0) ;
        name += preNsuffix[0][1] ;
        
        if( name.EndsWith(".root") ){
        
            TFile * input = new TFile(name,"READ") ;
            if( input->IsZombie() ){
                cout << " WARNING : opening " << name << endl ;
                notFound++ ;
                useable[h] = false ;
                continue ;
            }

            name = preNsuffix[1][0] ;
            name += filesNhists.at(h).at(1) ;
            name += preNsuffix[1][1] ;
            if( input->Get(name) == NULL ){
                cout << " WARNING : reading " << name 
                    << " in " << input->GetName() << endl ;
                notFound++ ;
                useable[h] = false ;
                continue ;
            }
            readhist = (TH2D*)input->Get(name) ;

            readhist->SetDirectory(0) ;
            input->Close() ;

            if( rowsNcolumns[1] == 0 ){
                
                rowsNcolumns[1] = readhist->GetNbinsX() ;
                rowsNcolumns[0] = readhist->GetNbinsY() ;
                
                for(unsigned int i=0; i<2; i++){
                    string toUse = "ROWS" ;
                    if( i == 1 ) toUse = "COLUMNS" ;
                    bool overwrite = true ;
                    if( 
                        givenRowsNcolumns.find( toUse ) 
                        == 
                        givenRowsNcolumns.end() 
                    ) 
                        overwrite = false ;
                    bool toSet = true ;
                    if( overwrite && givenRowsNcolumns[toUse].at(0) > 0 ) 
                        toSet = false ;
                    for(unsigned int b=0; b<rowsNcolumns[i]; b++)
                        useRowsNcolumns[toUse][b+1] = toSet ;
                    if( overwrite ){
                        unsigned int nSpecifiedLines = 
                            givenRowsNcolumns[toUse].size() ;
                        for(unsigned int s=0; s<nSpecifiedLines; s++)
                            useRowsNcolumns[toUse][
                                abs( givenRowsNcolumns[toUse].at(s) )
                            ] = !( toSet ) ;
                    }
                }
                
                if( givenRowsNcolumns.size() < 1 )
                    useRowsNcolumns.clear() ;
                
            }        
            else if(  
                rowsNcolumns[1] != readhist->GetNbinsX()
                ||
                rowsNcolumns[0] != readhist->GetNbinsY()
            ){
                cout << " WARNING : histsize " << name 
                    << " in " << input->GetName() << endl ;
                notFound++ ;
                useable[h] = false ;
                readhist->Delete() ;
                continue ;
            }

            number = 1 ;
            useable[h] = getStats(
                                    readhist ,
                                    mean , 
                                    stdv , 
                                    min , 
                                    max , 
                                    median , 
                                    number ,
                                    toSkip , 
                                    lowLimit , 
                                    highLimit , 
                                    useRowsNcolumns ,
                                    pixelList ,
                                    exclude
                                ) ;
        
            readhist->Delete() ;
            
            if( ! useable[h] ){ 
                cout << " WARNING : getStats " << name 
                    << " in " << input->GetName() << endl ;
                if( number != 1 )
                    g_number->SetPoint(
                        g_number->GetN() , valueNerror.at(h).at(0) , 0
                    ) ;
                notFound++ ;
                continue ;
            }
        
        }
        else{
            
            vector<vector<string> > input = getInput( 
                                                        name.Data() , 
                                                        delimiter 
                                                    ) ;
            unsigned int nLines = input.size() ;
            if( nLines < 1 ){
                cout << " WARNING : reading " << name << endl ;
                notFound++ ;
                useable[h] = false ;
                continue ;
            }
                
            rowsNcolumns[0] = nLines ;
            rowsNcolumns[1] = input.at(0).size() ;
            
            for(unsigned int i=0; i<2; i++){
                string toUse = "ROWS" ;
                if( i == 1 ) toUse = "COLUMNS" ;
                bool overwrite = true ;
                if( 
                    givenRowsNcolumns.find( toUse ) 
                    == 
                    givenRowsNcolumns.end() 
                ) 
                    overwrite = false ;
                bool toSet = true ;
                if( overwrite && givenRowsNcolumns[toUse].at(0) > 0 ) 
                    toSet = false ;
                for(unsigned int b=0; b<rowsNcolumns[i]; b++)
                    useRowsNcolumns[toUse][b+1] = toSet ;
                if( overwrite ){
                    unsigned int nSpecifiedLines = 
                        givenRowsNcolumns[toUse].size() ;
                    for(unsigned int s=0; s<nSpecifiedLines; s++)
                        useRowsNcolumns[toUse][
                            abs( givenRowsNcolumns[toUse].at(s) )
                        ] = !( toSet ) ;
                }
            }
            
            if( givenRowsNcolumns.size() < 1 )
                useRowsNcolumns.clear() ;
            
            vector<double> data ;
    
            bool discardRowsOrColumns[2] = { false , false } ;
            if( useRowsNcolumns.find("ROWS") != useRowsNcolumns.end() )
                discardRowsOrColumns[0] = true ;
            if( useRowsNcolumns.find("COLUMNS") != useRowsNcolumns.end() )
                discardRowsOrColumns[1] = true ;
            
            bool pixelSpecified = false ;
            if( !( pixelList.empty() ) ) pixelSpecified = true ;

            for(unsigned int l=0; l<nLines; l++){
                if( 
                    discardRowsOrColumns[0] 
                    && 
                    !( useRowsNcolumns["ROWS"][l+1] ) 
                )
                    continue ;
                unsigned int nWords = input.at(l).size() ;
                for(unsigned int w=0; w<nWords; w++){
                    if( 
                        discardRowsOrColumns[1] 
                        && 
                        !( useRowsNcolumns["COLUMNS"][w+1] ) 
                    )
                        continue ;
                    if( pixelSpecified ){
                        if( 
                            pixelList.find(l+1) != pixelList.end() 
                            && 
                            std::find( 
                                        pixelList[l+1].begin() , 
                                        pixelList[l+1].end() , 
                                        w+1 
                                     )
                                !=
                                    pixelList[l+1].end()
                        ){
                            if( exclude ) continue ;
                        }
                        else{
                            if( !( exclude ) ) continue ;
                        }
                    }
                    double content = atof( input.at(l).at(w).c_str() ) ;
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
                    data.push_back( content ) ;
                }
            }
            
            number = data.size() ;
            
            useable[h] = getStats( &data , mean , stdv , min , max , median ) ;
            
            if( ! useable[h] ){ 
                cout << " WARNING : getStats " << name << endl ;
                if( number != 1 )
                    g_number->SetPoint(
                        g_number->GetN() , valueNerror.at(h).at(0) , 0
                    ) ;
                notFound++ ;
                continue ;
            }
            
        }
        
        if( toInitialize ){
            toInitialize = false ;
            extrema[0][0] = valueNerror.at(h).at(0) ;
            extrema[0][1] = valueNerror.at(h).at(0) ;
            extrema[1][0] = min ;
            extrema[1][1] = max ;
        }
        else{
            if( extrema[0][0] > valueNerror.at(h).at(0) ) 
                extrema[0][0] = valueNerror.at(h).at(0) ;
            if( extrema[0][1] < valueNerror.at(h).at(0) )
                extrema[0][1] = valueNerror.at(h).at(0) ;
            if( extrema[1][0] > min )
                extrema[1][0] = min ;
            if( extrema[1][1] < max )
                extrema[1][1] = max ;
        }
        
        g_mean->SetPoint(
            g_mean->GetN() , valueNerror.at(h).at(0) , mean
        ) ;
        
        g_mean->SetPointError(
            g_mean->GetN()-1 , valueNerror.at(h).at(1) , stdv
        ) ;
        
        g_stdv->SetPoint(
            g_stdv->GetN() , valueNerror.at(h).at(0) , stdv
        ) ;
        
        g_min->SetPoint(
            g_min->GetN() , valueNerror.at(h).at(0) , min
        ) ;
        
        g_max->SetPoint(
            g_max->GetN() , valueNerror.at(h).at(0) , max
        ) ;
        
        g_number->SetPoint(
            g_number->GetN() , valueNerror.at(h).at(0) , number
        ) ;
        
        g_median->SetPoint(
            g_median->GetN() , valueNerror.at(h).at(0) , median
        ) ;
        
    }

    if( notFound == nHists ){
        cout << " ERROR : histograms not useable " << endl ;
        return 3 ;
    }

    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
    outfile->cd() ;
                     
    plotOptions() ;
    
    gStyle->SetOptStat(0) ;

    gStyle->SetPadTopMargin(    0.05 ) ;
    gStyle->SetPadRightMargin(  0.05 ) ;
    gStyle->SetPadBottomMargin( 0.12 ) ;
    gStyle->SetPadLeftMargin(   0.14 ) ;

    gStyle->SetTitleOffset( 1.1 , "x" ) ;
    gStyle->SetTitleOffset( 1.4 , "y" ) ;
    
    TApplication app("app", &argc, argv) ;     
    
    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TCanvas * can = new TCanvas( name , name , 800 , 600 ) ;
    can->SetGridy() ;
    
    if( unspecifiedHistname == nHists ){
        name = preNsuffix[1][0] ;
        name += preNsuffix[1][1] ;
        g_extrema->SetTitle( name ) ;
        g_extrema->SetName( name ) ;
    }
    else{
        g_extrema->SetTitle( "" ) ;
        g_extrema->SetName( "extrema" ) ;
    }
    double ranges[2] = {
        extrema[0][1] - extrema[0][0] ,
        extrema[1][1] - extrema[1][0]
    } ;
    g_extrema->SetPoint( 
                            g_extrema->GetN() , 
                            extrema[0][0] - 0.1 * ranges[0] , 
                            extrema[1][0] - 0.1 * ranges[1] 
                       ) ;
    g_extrema->SetPoint( 
                            g_extrema->GetN() , 
                            extrema[0][0] - 0.1 * ranges[0] , 
                            extrema[1][1] + 0.1 * ranges[1] 
                       ) ;
    g_extrema->SetPoint( 
                            g_extrema->GetN() , 
                            extrema[0][1] + 0.1 * ranges[0] , 
                            extrema[1][0] - 0.1 * ranges[1] 
                       ) ;
    g_extrema->SetPoint( 
                            g_extrema->GetN() , 
                            extrema[0][1] + 0.1 * ranges[0] , 
                            extrema[1][1] + 0.1 * ranges[1] 
                       ) ;
    if( 
        plotRange[0][0].setting 
        && 
        plotRange[0][1].setting 
        &&
        plotRange[1][0].setting 
        && 
        plotRange[1][1].setting 
    ){
        for(unsigned int e=0; e<2; e++)
            g_extrema->SetPoint( 
                            g_extrema->GetN() , 
                            plotRange[0][e].number , 
                            plotRange[1][e].number 
                       ) ;
            
    }
    g_extrema->SetMarkerStyle( 1 ) ;
    g_extrema->SetMarkerColor( 0 ) ;
    g_extrema->SetLineColor( 0 ) ;
    g_extrema->GetYaxis()->SetNdivisions(520) ;
    
    if( setLabels ){
        for(unsigned int h=0; h<nHists; h++){
            g_extrema->GetXaxis()->SetBinLabel( 
                g_extrema->GetXaxis()->FindBin(valueNerror.at(h).at(0)) , 
                axisLabels.at(h).c_str() 
            ) ;
        }
        if( labeblsotpion.setting )
            g_extrema->GetHistogram()
                     ->LabelsOption( labeblsotpion.specifier.c_str() , "X" ) ;
    }
    
    g_extrema->Draw( "AP" ) ;
    g_extrema->GetXaxis()->SetTitle( axisTitles[0].c_str() ) ;
    g_extrema->GetYaxis()->SetTitle( axisTitles[1].c_str() ) ;
    
    double low , high ;
    
    if( plotRange[0][0].setting && plotRange[0][1].setting ){
        low  = plotRange[0][0].number ;
        high = plotRange[0][1].number ;
    }
    else
        getLimits( extrema[0][0] , extrema[0][1] , low , high ) ;
    
    g_extrema->GetXaxis()->SetRangeUser( low , high ) ;
    
    if( plotRange[1][0].setting && plotRange[1][1].setting ){
        low  = plotRange[1][0].number ; 
        high = plotRange[1][1].number ;
    }
    else
        getLimits( extrema[1][0] , extrema[1][1] , low , high ) ;
    
    g_extrema->GetYaxis()->SetRangeUser( low , high ) ;
    
    g_mean->SetName("mean") ;
    g_mean->SetTitle("mean+/-std.dev.") ;
    g_mean->SetMarkerStyle( 20 ) ;
    g_mean->SetMarkerSize( 1.5 ) ;
    g_mean->SetMarkerColor( 1 ) ;
    g_mean->SetLineColor( 1 ) ;
    g_mean->Draw("Psame") ;
    
    g_median->SetName("median") ;
    g_median->SetTitle("median") ;
    g_median->SetMarkerStyle( 25 ) ;
    g_median->SetMarkerSize( 1.5 ) ;
    g_median->SetMarkerColor( 6 ) ;
    g_median->SetLineColor( 6 ) ;
    g_median->Draw("Psame") ;
    
    g_max->SetName("maximum") ;
    g_max->SetTitle("maximum") ;
    g_max->SetMarkerStyle( 22 ) ;
    g_max->SetMarkerSize( 1.5 ) ;
    g_max->SetMarkerColor( 2 ) ;
    g_max->SetLineColor( 2 ) ;
    g_max->Draw("Psame") ;
    
    g_min->SetName("minimum") ;
    g_min->SetTitle("minimum") ;
    g_min->SetMarkerStyle( 23 ) ;
    g_min->SetMarkerSize( 1.5 ) ;
    g_min->SetMarkerColor( 4 ) ;
    g_min->SetLineColor( 4 ) ;
    g_min->Draw("Psame") ;
    
    g_mean->Write() ;
    g_stdv->SetTitle("stdv") ;
    g_stdv->SetName("stdv") ;
    g_stdv->Write() ;
    g_min->Write() ;
    g_max->Write() ;
    g_median->Write() ;
    g_number->SetTitle("foundINrange") ;
    g_number->SetName("number") ;
    g_number->Write() ;
    
    can->BuildLegend() ;
    
    TGraphErrors * g_underflow ;
    if( 
        plotRange[1][0].setting 
        &&  
        extrema[1][0] < plotRange[1][0].number 
    ){
        
        g_underflow = new TGraphErrors() ;
        double xmin = g_extrema->GetXaxis()->GetXmin();
        double xmax = g_extrema->GetXaxis()->GetXmax();
        
        double x , y ;
        for(unsigned int p=0; p<g_min->GetN(); p++){
            g_min->GetPoint( p , x , y ) ;
            if( y < plotRange[1][0].number ){
                g_underflow->SetPoint(
                    g_underflow->GetN() ,
                    x , plotRange[1][0].number
                ) ;
                name = "" ;
                if( abs(y) < 1000. && abs(y) > 1. ) name.Form( "%2.1f" , y ) ;
                else name.Form( "%.1e" , y ) ;
                TText * t = new TText( 
                    x , 
                    plotRange[1][0].number + 0.02 
                    * ( plotRange[1][1].number - plotRange[1][0].number ) , 
                    name 
                ) ;
                t->SetTextColor(4);
                t->SetTextFont(font);
                t->SetTextSize(text_size*0.5);
                t->SetTextAlign(12);
                t->SetTextAngle(90);
                t->SetNDC(false) ;
                t->Draw();
            }
        }
        
        if( g_underflow->GetN() > 0 ){
            g_underflow->SetMarkerStyle( 23 ) ;
            g_underflow->SetMarkerSize( 1.5 ) ;
            g_underflow->SetMarkerColor( 4 ) ;
            g_underflow->SetLineColor( 4 ) ;
            g_underflow->Draw("Psame") ;
        }
        else g_underflow->Delete() ;
        
    }
    
    TGraphErrors * g_overflow ;
    if( 
        plotRange[1][1].setting 
        &&  
        extrema[1][1] > plotRange[1][1].number 
    ){
        
        g_overflow = new TGraphErrors() ;
        double xmin = g_extrema->GetXaxis()->GetXmin();
        double xmax = g_extrema->GetXaxis()->GetXmax();
        
        double x , y ;
        for(unsigned int p=0; p<g_max->GetN(); p++){
            g_max->GetPoint( p , x , y ) ;
            if( y > plotRange[1][1].number ){
                g_overflow->SetPoint(
                    g_overflow->GetN() ,
                    x , plotRange[1][1].number
                ) ;
                name = "" ;
                if( abs(y) < 1000. && abs(y) > 1. ) name.Form( "%2.1f" , y ) ;
                else name.Form( "%.1e" , y ) ;
                TText * t = new TText( 
                    x , 
                    plotRange[1][1].number - 0.02 
                    * ( plotRange[1][1].number - plotRange[1][0].number ) , 
                    name 
                ) ;
                t->SetTextColor(2);
                t->SetTextFont(font);
                t->SetTextSize(text_size*0.5);
                t->SetTextAlign(32);
                t->SetTextAngle(90);
                t->SetNDC(false) ;
                t->Draw();
            }
        }
        
        if( g_overflow->GetN() > 0 ){
            g_overflow->SetMarkerStyle( 22 ) ;
            g_overflow->SetMarkerSize( 1.5 ) ;
            g_overflow->SetMarkerColor( 2 ) ;
            g_overflow->SetLineColor( 2 ) ;
            g_overflow->Draw("Psame") ;
        }
        else g_overflow->Delete() ;
        
    }
    
    if( draw ){
    
        showing() ;
        
        name = can->GetName() ;
        name += ".pdf" ;
        can->Print(name) ;
        can->Write() ;
    
    }
    
    outfile->Write() ;
    outfile->Close() ;

    return 0 ;

}
