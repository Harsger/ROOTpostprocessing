#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

    plotOptions( false  , true ) ;
    gStyle->SetPadRightMargin( 0.18 ) ;
    gStyle->SetPadTopMargin(   0.08 ) ;
    gStyle->SetTitleOffset( 1.0 , "x" ) ;
    gStyle->SetOptTitle(1) ;
    gStyle->SetTitleX(0.5);
    gStyle->SetTitleAlign(23);

    TString filename = argv[1] ;

    vector< vector<string> > parameter = getInput( filename.Data() );
    
    bool show = true ;
    
    if( argc > 2 ) show = false ;

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };

    vector< vector<string> > filesNhists ;
    vector<string> strVecDummy ;
    vector<TString> histIdentifier ;
    vector<TString> histName ;
    unsigned int nIdentified = 0 ;
    SpecifiedNumber labeblsotpion ;
    
    map< string , vector<double> > ranges = {
        { "difference_relative" , { 2000 , -1. , 1. } } ,
        { "difference_absolute" , { 2000 , -1. , 1. } } ,
        { "variation_relative"  , { 2000 ,  0. , 1. } } ,
        { "variation_absolute"  , { 2000 ,  0. , 1. } } ,
        { "spectra"             , { 2000 , -1. , 1. } } ,
    } ;
    map< string , bool > rangeSetting = {
        { "difference_relative" , false } ,
        { "difference_absolute" , false } ,
        { "variation_relative"  , false } ,
        { "variation_absolute"  , false } ,
        { "spectra"             , false } ,
    } ;

    map< string , map< unsigned int , bool > > useRowsNcolumns ;
    map< string , vector<int> > givenRowsNcolumns ;
    map< unsigned int , vector<unsigned int> > pixelList ;
    bool exclude = true ;

    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//            cout << " comment line " << r << endl ;
            continue ;
        }

        int fileORhist = -1 ;

        if( parameter.at(r).at(0).compare("FILE") == 0  ) fileORhist = 0 ;
        else if( parameter.at(r).at(0).compare("HIST") == 0  ) fileORhist = 1 ;

        if( fileORhist > -1 && parameter.at(r).size() > 2 ){
            preNsuffix[fileORhist][0] = parameter.at(r).at(1) ;
            preNsuffix[fileORhist][1] = parameter.at(r).at(2) ;
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
        
        if( 
            parameter.at(r).at(0).compare("RANGE") == 0  
            &&
            parameter.at(r).size() > 4
            &&
            ranges.find( parameter.at(r).at(1) ) != ranges.end() 
        ){
            for(unsigned int i=0; i<3; i++){
                if( parameter.at(r).at(i+2).compare( "%" ) != 0 ){
                    ranges[parameter.at(r).at(1)].at(i) =
                        atof( parameter.at(r).at(i+2).c_str() ) ;
                }
            }
            rangeSetting[parameter.at(r).at(1)] = true ;
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

        if( parameter.at(r).size() > 1 ){
            strVecDummy.push_back( parameter.at(r).at(0) );
            strVecDummy.push_back( parameter.at(r).at(1) );
            if( parameter.at(r).size() > 2 ){
                histIdentifier.push_back( parameter.at(r).at(2) ) ;
                nIdentified++ ;
            }
            else
                histIdentifier.push_back( "" ) ;
        }

        filesNhists.push_back( strVecDummy ) ;
        strVecDummy.clear() ;

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

    for(unsigned int r=0; r<filesNhists.size(); r++){
        for(unsigned int c=0; c<2; c++){
            if( filesNhists.at(r).at(c).compare( "%" ) == 0 )
                filesNhists.at(r).at(c) = "" ;
        }
    }

    unsigned int nHists = filesNhists.size() ;
    
    if( nHists < 2 ){
        cout << " ERROR : not enough histograms specified " << endl ;
        return 2 ;
    }
    
    TH2D** hists = new TH2D*[nHists] ;
    TString name ;
    unsigned int rowsNcolumns[2] = { 0 , 0 } ;
    double axesRanges[2][2] = { { 0. , 0. } , { 0. , 0. } } ;
    unsigned int notFound = 0 ;
    bool useable[nHists] ;
    
    for(unsigned int r=0; r<nHists; r++){

        name = preNsuffix[0][0] ;
        name += filesNhists.at(r).at(0) ;
        name += preNsuffix[0][1] ;
        TFile * input = new TFile(name,"READ") ;
        if( input->IsZombie() ){
            cout << " ERROR : opening " << name << endl ;
            notFound++ ;
            useable[r] = false ;
            continue ;
        }

        name = preNsuffix[1][0] ;
        name += filesNhists.at(r).at(1) ;
        name += preNsuffix[1][1] ;
        if( input->Get(name) == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            notFound++ ;
            useable[r] = false ;
            continue ;
        }
        hists[r] = (TH2D*)input->Get(name) ;

        hists[r]->SetDirectory(0) ;
        input->Close() ;

        if( rowsNcolumns[1] == 0 ){
            rowsNcolumns[1] = hists[r]->GetNbinsX() ;
            rowsNcolumns[0] = hists[r]->GetNbinsY() ;
            axesRanges[0][0] = hists[r]->GetXaxis()->GetXmin() ;
            axesRanges[0][1] = hists[r]->GetXaxis()->GetXmax() ;
            axesRanges[1][0] = hists[r]->GetYaxis()->GetXmin() ;
            axesRanges[1][1] = hists[r]->GetYaxis()->GetXmax() ;
        }        
        else if(  
            rowsNcolumns[1] != hists[r]->GetNbinsX()
            ||
            rowsNcolumns[0] != hists[r]->GetNbinsY()
        ){
            notFound++ ;
            useable[r] = false ;
            continue ;
        }

        useable[r] = true ;

    }

    if( rowsNcolumns[1] == 0 && rowsNcolumns[0] == 0 ){
        cout << " ERROR : empty histograms " << endl ;
        return 3 ;
    }
 
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

    bool discardRowsOrColumns[2] = { false , false } ;
    if( useRowsNcolumns.find("ROWS") != useRowsNcolumns.end() )
        discardRowsOrColumns[0] = true ;
    if( useRowsNcolumns.find("COLUMNS") != useRowsNcolumns.end() )
        discardRowsOrColumns[1] = true ;

    unsigned int nToUse = nHists - notFound ;

    TApplication app("app", &argc, argv) ;

//    for(unsigned int h=0; h<hists.size(); h++){
//        hists.at(h)->Draw("COLZ") ;
//        gPad->Modified() ;
//        gPad->Update() ;
//        gPad->WaitPrimitive() ;
//        hists.at(h)->Delete() ;
//    }

    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;

    bool tooManyDefault = false ;
    for(unsigned int h=0; h<nHists; h++){
        name = "" ;
        if( histIdentifier.at(h).Length() < 1 ){ 
            bool bothSpecified = true ;
            if( 
                filesNhists.at(h).at(0).empty() 
                ||
                filesNhists.at(h).at(1).empty() 
            )
                bothSpecified = false ;
            if( bothSpecified ) name += "file_" ;
            name += filesNhists.at(h).at(0) ;
            if( bothSpecified ) name += "_hist_" ;
            name += filesNhists.at(h).at(1) ;
        }
        else
            name = histIdentifier.at(h) ;
        name = replaceBadChars( name ) ;
        if( name == "" ){
            name = "default" ;
            if( tooManyDefault ){
                cout << " ERROR : " 
                     << "histnames can not be properly specified " << endl ;
                return 4 ;
            }
            else tooManyDefault = true ;
        }
        TString test = name(0,1) ;
        if( test.IsDec() ) name = "h"+name ;
        histName.push_back( name ) ;
    }

    outfile->cd() ;

    TH2I ** differenceTOmean = new TH2I*[2] ;
    double sigmaFactor = 5.;
    if( rangeSetting["difference_relative"] ) sigmaFactor = 1. ;
    differenceTOmean[0] = new TH2I( 
                                    "differenceTOmean_relative" ,
                                    "differenceTOmean_relative" ,
                                    nHists , 0.5 , nHists+0.5 ,
                                    ranges["difference_relative"].at(0) ,
                                    sigmaFactor 
                                    * ranges["difference_relative"].at(1) ,
                                    sigmaFactor 
                                    * ranges["difference_relative"].at(2) 
                                );

    TH2I *** differenceTOeach = new TH2I**[2] ;
    differenceTOeach[0] = new TH2I*[nHists] ;
    for(unsigned int h=0; h<filesNhists.size(); h++){
        name = histName.at( h ) ;
        name += "_diffRelative" ;
        differenceTOeach[0][h] = new TH2I(
                                        name , name ,
                                        nHists , 0.5 , nHists+0.5 ,
                                        ranges["difference_relative"].at(0) ,
                                        ranges["difference_relative"].at(1) ,
                                        ranges["difference_relative"].at(2) 
                                    );
    }

    TH2I ** variationDistribution = new TH2I*[2] ;
    variationDistribution[0] = new TH2I(
                            "variation_relative" ,
                            "variation_relative" ,
                            nHists , 0.5 , nHists+0.5 ,
                            ranges["variation_relative"].at(0) ,
                            ranges["variation_relative"].at(1) ,
                            ranges["variation_relative"].at(2) 
                        ) ;

    double mean , stdv , minMax[3][2] , content , reference , difference ; 
    unsigned int number ;
    bool setMinMax[3] = { true , true , true } ;

    for(unsigned int r=1; r<=rowsNcolumns[0]; r++){
        if( discardRowsOrColumns[0] && !( useRowsNcolumns["ROWS"][r-1] ) )
            continue ;
        for(unsigned int c=1; c<=rowsNcolumns[1]; c++){
            if( 
                discardRowsOrColumns[1] 
                && 
                !( useRowsNcolumns["COLUMNS"][r-1] ) 
            )
                continue ;
            if( 
                pixelList.find(r-1) != pixelList.end() 
                && 
                std::find( 
                            pixelList[r-1].begin() , 
                            pixelList[r-1].end() , 
                            c-1 
                         )
                    !=
                        pixelList[r-1].end()
            ){
                if( exclude ) continue ;
            }
            else{
                if( !( exclude ) ) continue ;
            }
            for(unsigned int h=0; h<nHists; h++){
                if( ! useable[h] ) continue ;
                content = hists[h]->GetBinContent( c , r ) ;
                if( toDiscard( content ) ) continue ;
                if( setMinMax[0] ){
                    minMax[0][0] = content ;
                    minMax[0][1] = content ;
                    setMinMax[0] = false ;
                }
                if( minMax[0][0] > content ) minMax[0][0] = content ;
                if( minMax[0][1] < content ) minMax[0][1] = content ;
                mean = 0. ;
                stdv = 0. ;
                number = 0 ;
                for(unsigned int o=0; o<nHists; o++){
                    if( ! useable[o] ) continue ;
                    if( h == o ) continue ;
                    reference = hists[o]->GetBinContent( c , r ) ;
                    if( toDiscard( reference ) ) continue ;
                    difference = content - reference ;
                    mean += reference ;
                    stdv += ( reference * reference ) ;
                    number++ ;
                    if( reference != 0. )
                        differenceTOeach[0][h]->Fill( 
                                            o+1 , difference/abs(reference) 
                        ) ;
                    if( h >= o ) continue ;
                    if( setMinMax[1] ){
                        minMax[1][0] = difference ;
                        minMax[1][1] = difference ;
                        setMinMax[1] = false ;
                    }
                    if( minMax[1][0] > difference ) minMax[1][0] = difference ;
                    if( minMax[1][1] < difference ) minMax[1][1] = difference ;
                }
                if( number != nToUse-1 ) continue ;
                mean /= number ;
                if( number > 1 )
                    stdv = sqrt( 
                                    ( stdv - mean * mean * number ) 
                                    / 
                                    ( number - 1. ) 
                                ) ; 
                else
                    stdv = 0. ;
                if( setMinMax[2] ){
                    minMax[2][0] = stdv ;
                    minMax[2][1] = stdv ;
                    setMinMax[2] = false ;
                }
                if( minMax[2][0] > stdv ) minMax[2][0] = stdv ;
                if( minMax[2][1] < stdv ) minMax[2][1] = stdv ;
                difference = content - mean ;
                differenceTOmean[0]->Fill( h+1 , difference/abs(stdv) ) ;
                if( mean == 0. ) continue ;
                variationDistribution[0]->Fill( h+1 , stdv/abs(mean) ) ;
            }
        }
    }

    cout << " value range " << minMax[0][0] << " to " << minMax[0][1] << endl ;
    cout << " diff. range " << minMax[1][0] << " to " << minMax[1][1] << endl ;
    cout << " STDV  range " << minMax[2][0] << " to " << minMax[2][1] << endl ;
    
    if( !( rangeSetting["spectra"] ) ){
        ranges["spectra"].at(0) = 2000 ;
        ranges["spectra"].at(1) = minMax[0][0] ;
        ranges["spectra"].at(2) = minMax[0][1] ;
    }
         
    TH2I * spectra = new TH2I( 
                                "spectra" , "spectra" ,  
                                nHists , 0.5 , nHists+0.5 ,
                                ranges["spectra"].at(0) ,
                                ranges["spectra"].at(1) ,
                                ranges["spectra"].at(2) 
                             );
    
    if( !( rangeSetting["difference_absolute"] ) ){
        ranges["difference_absolute"].at(0) = 2000 ;
        ranges["difference_absolute"].at(1) = minMax[1][0] ;
        ranges["difference_absolute"].at(2) = minMax[1][1] ;
    }

    differenceTOmean[1] = new TH2I( 
                                    "differenceTOmean_absolute" ,
                                    "differenceTOmean_absolute" ,
                                    nHists , 0.5 , nHists+0.5 ,
                                    ranges["difference_absolute"].at(0) ,
                                    -ranges["difference_absolute"].at(2) ,
                                     ranges["difference_absolute"].at(2) 
                                );

    differenceTOeach[1] = new TH2I*[nHists] ;
    for(unsigned int h=0; h<filesNhists.size(); h++){
        name = histName.at( h ) ;
        name += "_diffAbsolute" ;
        differenceTOeach[1][h] = new TH2I(
                                        name , name ,
                                        nHists , 0.5 , nHists+0.5 ,
                                        ranges["difference_absolute"].at(0) ,
                                        -ranges["difference_absolute"].at(2) ,
                                         ranges["difference_absolute"].at(2) 
                                    );
    }

    if( 
        minMax[2][0] == 0. && minMax[2][1] == 0. 
        ||
        minMax[2][0] == minMax[2][1]
    ){
        minMax[2][0] -= 1. ;
        minMax[2][1] += 1. ;
    }
    
    if( !( rangeSetting["variation_absolute"] ) ){
        ranges["variation_absolute"].at(0) = 2000 ;
        ranges["variation_absolute"].at(1) = minMax[2][0] ;
        ranges["variation_absolute"].at(2) = minMax[2][1] ;
    }
    
    variationDistribution[1] = new TH2I(
                                        "variation_absolute" ,
                                        "variation_absolute" ,
                                        nHists , 0.5 , nHists+0.5 ,
                                        ranges["variation_absolute"].at(0) ,
                                        ranges["variation_absolute"].at(1) ,
                                        ranges["variation_absolute"].at(2) 
                                    ) ;
                        
    TH2D * variationMap = new TH2D(
                                    "variationMap" , "variationMap" ,
                                    rowsNcolumns[0] ,
                                    axesRanges[0][0] , axesRanges[0][1] ,
                                    rowsNcolumns[1] ,
                                    axesRanges[1][0] , axesRanges[1][1] 
    ) ;
    
    unsigned int count ;
    double average , variation ;

    for(unsigned int r=1; r<=rowsNcolumns[0]; r++){
        if( discardRowsOrColumns[0] && !( useRowsNcolumns["ROWS"][r-1] ) )
            continue ;
        for(unsigned int c=1; c<=rowsNcolumns[1]; c++){
            if( 
                discardRowsOrColumns[1] 
                && 
                !( useRowsNcolumns["COLUMNS"][r-1] ) 
            )
                continue ;
            if( 
                pixelList.find(r-1) != pixelList.end() 
                && 
                std::find( 
                            pixelList[r-1].begin() , 
                            pixelList[r-1].end() , 
                            c-1 
                         )
                    !=
                        pixelList[r-1].end()
            ){
                if( exclude ) continue ;
            }
            else{
                if( !( exclude ) ) continue ;
            }
            count = 0 ;
            average = 0. ;
            variation = 0. ;
            for(unsigned int h=0; h<nHists; h++){
                if( ! useable[h] ) continue ;
                content = hists[h]->GetBinContent( c , r ) ;
                if( toDiscard( content ) ) continue ;
                count++ ;
                average += content ;
                variation += ( content * content ) ;
                spectra->Fill( h+1 , content ) ; 
                mean = 0. ;
                stdv = 0. ;
                number = 0 ;
                for(unsigned int o=0; o<nHists; o++){
                    if( ! useable[o] ) continue ;
                    if( h == o ) continue ;
                    reference = hists[o]->GetBinContent( c , r ) ;
                    if( toDiscard( reference ) ) continue ;
                    difference = content - reference ;
                    mean += reference ;
                    stdv += ( reference * reference ) ;
                    number++ ;
                    differenceTOeach[1][h]->Fill( o+1 , difference ) ;
                }
                if( number != nToUse-1 ) continue ;
                mean /= number ;
                if( number > 1 )
                    stdv = sqrt( 
                                    ( stdv - mean * mean * number ) 
                                    / 
                                    ( number  - 1. ) 
                                ) ; 
                else
                    stdv = 0. ;
                difference = content - mean ;
                differenceTOmean[1]->Fill( h+1 , difference ) ;
                variationDistribution[1]->Fill( h+1 , stdv ) ;
            }
            if( count > 1 )
                variation = sqrt(
                                    ( 
                                        variation 
                                        -  
                                        average * average / (double)count
                                    )
                                    /
                                    ( (double)count - 1. )
                                ) ;
            else variation = 0. ;
            variationMap->SetBinContent( c , r , variation ) ;
        }
    }

    name = outfile->GetName() ;
    name = name.ReplaceAll( ".root" , "" ) ;
    name += "_difference_maps" ;
    TCanvas * can = new TCanvas( name , name , 1000 , 800 ) ;
    can->Divide( nHists-1 , nHists-1 ) ;

    gStyle->SetOptStat(0) ;

    for(unsigned int h=0; h<nHists; h++){
        for(unsigned int o=1; o<nHists; o++){
            TH2D * diffHist = (TH2D*)hists[h]->Clone() ;
            name = histName.at(h) ;
            name += "_minus_" ;
            name += histName.at(o) ;
            diffHist->SetName(name);
            name = histIdentifier.at(h) ;
            name += " - " ;
            name += histIdentifier.at(o) ;
            diffHist->SetTitle(name);
            diffHist->Add( hists[h] , hists[o] , 1. , -1. ) ;
            if( h >= o ) continue ;
            can->cd( h * (nHists-1) + o ) ;
            diffHist->Draw("COLZ");
            diffHist->GetZaxis()->SetRangeUser( 
                                        ranges["difference_absolute"].at(1) ,
                                        ranges["difference_absolute"].at(2) 
                                    ) ;
        }
    }

    for(unsigned int h=0; h<nHists; h++) hists[h]->Delete() ;
    
    if( nIdentified == nHists ){
        for(unsigned int r=0; r<2; r++){
            for(unsigned int h=0; h<nHists; h++){
                differenceTOmean[r]->GetXaxis()->SetBinLabel(
                    differenceTOmean[r]->GetXaxis()->FindBin(h+1) ,
                    histIdentifier.at(h)
                ) ;
                for(unsigned int o=0; o<nHists; o++){
                    differenceTOeach[r][h]->GetXaxis()->SetBinLabel(
                        differenceTOeach[r][h]->GetXaxis()->FindBin(o+1) ,
                        histIdentifier.at(o)
                    ) ;
                }
                variationDistribution[r]->GetXaxis()->SetBinLabel(
                    variationDistribution[r]->GetXaxis()->FindBin(h+1) ,
                    histIdentifier.at(h)
                ) ;
            }
            if( labeblsotpion.setting ){
                differenceTOmean[r]->LabelsOption( 
                    labeblsotpion.specifier.c_str() , "X" 
                ) ;
                for(unsigned int h=0; h<nHists; h++){
                    differenceTOeach[r][h]->LabelsOption( 
                        labeblsotpion.specifier.c_str() , "X" 
                    ) ;
                }
                variationDistribution[r]->LabelsOption( 
                    labeblsotpion.specifier.c_str() , "X" 
                ) ;
            }
        }
        for(unsigned int h=0; h<nHists; h++){
            spectra->GetXaxis()->SetBinLabel(
                spectra->GetXaxis()->FindBin(h+1) ,
                histIdentifier.at(h)
            ) ;
        }
        if( labeblsotpion.setting ){
            spectra->LabelsOption( 
                labeblsotpion.specifier.c_str() , "X" 
            ) ;
        }
    }

    cout << " writing ... " << flush ;

    outfile->Write() ;
    
    if( show ){

        cout << " plotting ... " << flush ;
            
        showing() ;

        name = can->GetName() ;
        name += ".pdf" ;
        can->Print(name) ;
    
    }

    cout << " closing ... " << flush ;
    
    outfile->Close() ;

    cout << " done " << endl ;

    return 0 ;

}
