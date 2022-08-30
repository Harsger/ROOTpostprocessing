#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ) return 1 ;
    
    plotOptions() ;

    gStyle->SetPadTopMargin(    0.055 ) ;
    gStyle->SetPadRightMargin(  0.08 ) ;
    gStyle->SetPadBottomMargin( 0.10 ) ;
    gStyle->SetPadLeftMargin(   0.14 ) ;

    gStyle->SetTitleOffset( 1.0 , "x" ) ;
    gStyle->SetTitleOffset( 1.4 , "y" ) ;

    TString filename = argv[1] ;
    TString filesNdata[2][2] ;
    SpecifiedNumber ranges[2][2] ;
    SpecifiedNumber divisions[2] ;
    SpecifiedNumber maxDistance ;
    TString outname ;
    TString name ;

    bool draw = true ;
    bool print = true ;
    bool useErrors = true ;
    TString toDraw = "GRAPH" ;
    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };
    string axisTitles[2] = { neverUse , neverUse } ;
    bool titlesDefined = false ;
    bool sameSource[2] = { false , false } ;
    map< string , map< unsigned int , bool > > useRowsNcolumns ;
    map< string , vector<int> > givenRowsNcolumns ;
    map< unsigned int , vector<unsigned int> > pixelList ;
    bool exclude = true ;
    bool useFirstOccurence = false ;
    vector< vector< SpecifiedNumber > > intervals ;
    vector< SpecifiedNumber > specVecDummy ;

    bool calculateDifferences = false ;
    unsigned int diffBins ;
    double diffRange[2] ;

    unsigned int count = 0 ;
    double value , low , high ;
    
    if( argc > 4 && filename.EndsWith(".root") ){

        filesNdata[0][0] = argv[1] ;
        filesNdata[0][1] = argv[2] ;
        filesNdata[1][0] = argv[3] ;
        filesNdata[1][1] = argv[4] ;

        if( argc > 5 ){
            name = argv[5] ;
            if( name == "skip" ) draw = false ;
            else if( name.Contains("skip") ) print = false ;
            if(      name.Contains("HIST") ) toDraw = "HIST" ;
            else if( name.Contains("DIFF") ) toDraw = "DIFF" ;
        }
        if( argc > 7 ){
            for(unsigned int c=0; c<2; c++){
                if( string( argv[6+c] ).compare( "%" ) != 0  ){
                    value = getNumberWithRange(
                                                string( argv[6+c] ) ,
                                                low ,
                                                high
                                              ) ;
                    if( !( toDiscard(value) ) )
                        divisions[c] = SpecifiedNumber( value ) ;
                    if( !( toDiscard(low)   ) )
                        ranges[c][0] = SpecifiedNumber( low   ) ;
                    if( !( toDiscard(high)  ) )
                        ranges[c][1] = SpecifiedNumber( high  ) ;
                }
            }
        }
        if( argc > 8 && string( argv[8] ).compare( "%" ) != 0 ){
            maxDistance = SpecifiedNumber( atof( argv[8] ) ) ;
            if( maxDistance.number < 0. ){
                maxDistance.number = abs( maxDistance.number ) ;
                useFirstOccurence = true ;
            }
        }
        if( argc > 9 && string( argv[9] ).compare( "%" ) != 0 ){
            diffBins = getNumberWithRange(
                                            string( argv[9] ) ,
                                            diffRange[0] ,
                                            diffRange[1]
                                        ) ;
            if(
                !( toDiscard( diffBins     ) )
                &&
                !( toDiscard( diffRange[0] ) )
                &&
                !( toDiscard( diffRange[1] ) )
            )
                calculateDifferences = true ;
        }

        for(unsigned int a=10; a<argc; a++){
            name = argv[a] ;
            if( name.BeginsWith("[") && name.EndsWith("]") ){
                value = getNumberWithRange( string( argv[a] ) , low , high ) ;
                if( toDiscard( low ) )
                     specVecDummy.push_back( SpecifiedNumber() ) ;
                else specVecDummy.push_back( SpecifiedNumber( low )  ) ;
                if( toDiscard( high ) )
                     specVecDummy.push_back( SpecifiedNumber() ) ;
                else specVecDummy.push_back( SpecifiedNumber( high )  ) ;
                intervals.push_back( specVecDummy ) ;
                specVecDummy.clear() ;
            }
        }

        if( filesNdata[1][0] == "%" && filesNdata[1][1] == "%" ){
            cout << " ERROR : input not well specified " << endl ;
            return 2 ;
        }
        if( filesNdata[1][0] == "%" ) name = filesNdata[0][0] ;
        else                          name = filesNdata[1][0] ;
        if( name.Contains("/") ) 
            name = name( name.Last('/')+1 , name.Sizeof() ) ;
        outname = name ;
        if( filesNdata[1][1] != "%" ){
            outname += "_" ;
            outname += filesNdata[1][1] ;
        }
        outname += "_VS" ;
        name = filesNdata[0][0] ;
        if( name.Contains("/") )                                                
            name = name( name.Last('/')+1 , name.Sizeof() ) ;
        if( filesNdata[1][0] != "%" ){
            outname += "_" ;
            outname += name ;
        }
        outname += "_" ;
        outname += filesNdata[0][1] ;
        outname.ReplaceAll( ".root" , "" ) ;
        outname = replaceBadChars( outname ) ;
        outname += ".root" ;
        if( filesNdata[1][0] == "%" ){
            filesNdata[1][0] = filesNdata[0][0] ;
            sameSource[0] = true ;
        }
        if( filesNdata[1][1] == "%" ){
            filesNdata[1][1] = filesNdata[0][1] ;
            sameSource[1] = true ;
        }

    }
    else{
        
        vector< vector<string> > parameter = getInput( filename.Data() ) ;

        if( argc > 2 ){
            name = argv[2] ;
            if( name == "skip" ) draw = false ;
            else if( name.Contains("skip") ) print = false ;
            if(      name.Contains("HIST") ) toDraw = "HIST" ;
            else if( name.Contains("DIFF") ) toDraw = "DIFF" ;
        }
        
        for(unsigned int r=0; r<parameter.size(); r++){

            if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//                 cout << " comment line " << r << endl ;
                continue ;
            }

            int specifier = -1 ;

            if( parameter.at(r).at(0).compare("FILE") == 0  ) 
                specifier = 0 ;
            else if( 
                parameter.at(r).at(0).compare("HIST") == 0  
                ||
                parameter.at(r).at(0).compare("GRAPH") == 0
            ) 
                specifier = 1 ;
            else if( parameter.at(r).at(0).compare("AXIS") == 0  ) 
                specifier = 2 ;

            if( specifier > -1 && parameter.at(r).size() > 2 ){
                if( specifier < 2 ){
                    preNsuffix[specifier][0] = parameter.at(r).at(1) ;
                    preNsuffix[specifier][1] = parameter.at(r).at(2) ;
                }
                else if( specifier == 2 ){
                    axisTitles[0] = parameter.at(r).at(1) ;
                    axisTitles[1] = parameter.at(r).at(2) ;
                    titlesDefined = true ;
                }
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
                titlesDefined = true ;
                continue ;
            }

            if( parameter.at(r).at(0).compare("NOERRORS") == 0 ){
                useErrors = false ;
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
                parameter.at(r).at(0).compare("MAXDISTANCE") == 0
                &&
                parameter.at(r).size() > 1
            ){
                maxDistance = SpecifiedNumber(
                                    atof( parameter.at(r).at(1).c_str() ) 
                                ) ;
                if( maxDistance.number < 0. ){
                    maxDistance.number = abs( maxDistance.number ) ;
                    useFirstOccurence = true ;
                }
                continue ;
            }

            if( parameter.at(r).at(0).compare("USEFIRSTOCCURENCE") == 0 ){
                useFirstOccurence = true ;
                continue ;
            }

            if(
                parameter.at(r).at(0).compare("DIFFERENCES") == 0
                &&
                parameter.at(r).size() > 3
            ){
                calculateDifferences = true ;
                diffRange[0] = atof( parameter.at(r).at(1).c_str() ) ;
                diffRange[1] = atof( parameter.at(r).at(2).c_str() ) ;
                diffBins     = atoi( parameter.at(r).at(3).c_str() ) ;
                continue ;
            }
            
            if(
                parameter.at(r).at(0).compare("INTERVAL") == 0  
                &&
                parameter.at(r).size() > 2
            ){
                for(unsigned int i=1; i<3; i++){
                    if( TString( parameter.at(r).at(i).c_str() ).IsFloat() ) 
                        specVecDummy.push_back( 
                            SpecifiedNumber( 
                                atof( 
                                    parameter.at(r).at(i).c_str() 
                                ) 
                            ) 
                        ) ;
                    else
                        specVecDummy.push_back( SpecifiedNumber() ) ;
                }
                intervals.push_back( specVecDummy ) ;
                specVecDummy.clear() ;
                continue ;
            }
    
            if( parameter.at(r).size() < 2 ) continue ;
            
            if( count > 1 ) continue ;
            
            filesNdata[count][0] = parameter.at(r).at(0) ;
            filesNdata[count][1] = parameter.at(r).at(1) ;
            
            if( parameter.at(r).size() > 3 ){
                if( parameter.at(r).at(2).compare( "%" ) != 0  )
                    ranges[count][0] = 
                        SpecifiedNumber( 
                            atof( parameter.at(r).at(2).c_str() ) 
                        ) ;
                if( parameter.at(r).at(3).compare( "%" ) != 0  )
                    ranges[count][1] = 
                        SpecifiedNumber( 
                            atof( parameter.at(r).at(3).c_str() ) 
                        ) ;
            }
            
            if( 
                parameter.at(r).size() > 4 
                && 
                parameter.at(r).at(4).compare( "%" ) != 0  
            )
                divisions[count] = 
                    SpecifiedNumber( 
                        atof( parameter.at(r).at(4).c_str() ) 
                    ) ;
            
            count++ ;
            
        }
        
        if( count != 2 ){
            cout << " ERROR : parameter-file not in correct format " << endl ;
            return 3 ;
        }
        
        outname = filename ;
        if( outname.Contains(".") ) 
            outname = outname( 0 , outname.Last('.') ) ;
        outname += ".root" ;
        if( outname.Contains("/") ) 
            outname = outname( outname.Last('/')+1 , outname.Sizeof() ) ;
        
    }

    for(unsigned int t=0; t<2; t++){
        for(unsigned int f=0; f<2; f++){
            if( 
                preNsuffix[t][f].compare( neverUse ) == 0 
                ||
                preNsuffix[t][f].compare( "%" ) == 0
            )
                preNsuffix[t][f] = "" ;
            if( filesNdata[t][f] == "%" )
                filesNdata[t][f] = "" ;
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

    if( toDraw == "HIST" ) gStyle->SetPadRightMargin( 0.18 ) ;
    
    count = 0 ; 
    unsigned int bins[2] ;   
 
    TH2D ** hists = new TH2D*[2]{ NULL , NULL } ;
    TGraphErrors ** graphs = new TGraphErrors*[2]{ NULL , NULL } ;
    TString sourceNames[2][2] ;
    
    for(unsigned int h=0; h<2; h++){

        name = preNsuffix[0][0] ;
        name += filesNdata[h][0] ;
        name += preNsuffix[0][1] ;
        TFile * input = new TFile(name,"READ") ;
        if( input->IsZombie() ){
            cout << " ERROR : opening " << input->GetName() << endl ;
            return 4 ;
        }
        sourceNames[h][0] = name ;

        name = preNsuffix[1][0] ;
        name += filesNdata[h][1] ;
        name += preNsuffix[1][1] ;
        if( input->Get(name) == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            return 5 ;
        }
        sourceNames[h][1] = name ;
        
        TString dataClass = input->Get( name )->ClassName() ;
        double min , max ;

        if( dataClass.Contains("TH") ){

            hists[h] = (TH2D*)input->Get( name ) ;
            hists[h]->SetDirectory(0) ;
            input->Close() ;

            if( h == 0 ){
                bins[0] = (unsigned int)hists[h]->GetNbinsY() ;
                bins[1] = (unsigned int)hists[h]->GetNbinsX() ;
            }
            else if( 
                bins[0] != hists[1]->GetNbinsY()
                ||
                bins[1] != hists[1]->GetNbinsX() 
            ){
                cout << " ERROR : histograms unequal sized " << endl ;
                return 6 ; 
            }   
            
            double mean , stdv , median ;
            unsigned int number ;
            vector<double> toSkip ;
            
            bool working = getStats(
                                        hists[h] ,
                                        mean ,
                                        stdv ,
                                        min ,
                                        max ,
                                        median ,
                                        number ,
                                        toSkip ,
                                        SpecifiedNumber() ,
                                        SpecifiedNumber() ,
                                        useRowsNcolumns ,
                                        pixelList ,
                                        exclude
                                    );
            
            if( ! working ) break ;

        }
        else if( dataClass.Contains("Graph") ){
            
            graphs[h] = (TGraphErrors*)input->Get( name ) ;
            bins[h] = graphs[h]->GetN() ;
            if( bins[h] < 1 ){
                cout << " ERROR : graph "<< h 
                     << " with too few points " << endl ;
                return 7 ;
            }
            
            double x , y ;
            for(unsigned int p=0; p<bins[h]; p++){
                graphs[h]->GetPoint( p , x , y ) ;
                if( p == 0 ){
                    min = y ;
                    max = y ;
                }
                else{
                    if(      min > y ) min = y ;
                    else if( max < y ) max = y ;
                }
            }

        }

        count++ ;

        if( ! ranges[h][0].setting )
            ranges[h][0].number = min ;
        if( ! ranges[h][1].setting )
            ranges[h][1].number = max ;
        
        if( ! divisions[h].setting ) divisions[h] = 2000 ;
    
    }
    
    if( count != 2 ){
        cout << " ERROR : wrong number of data samples " << endl ;
        return 8 ;
    }
    
    bool histData = false ;

    if(       hists[0] != NULL &&  hists[1] != NULL ) histData = true ;
    else if( graphs[0] == NULL || graphs[1] == NULL ){
        cout << " ERROR : mixed data (histogram and graph) " 
             << "can not be processed " << endl ;
        return 9 ;
    }
        
    TFile * outfile = new TFile( outname , "RECREATE" ) ;
    
    double upLimit[2] = { ranges[0][1].number , ranges[1][1].number } ;
    if( ! ranges[0][1].setting )
        upLimit[0] = ranges[0][1].number 
                        + (ranges[0][1].number-ranges[0][0].number)
                        / divisions[0].number ;
    if( ! ranges[1][1].setting )
        upLimit[1] = ranges[1][1].number 
                        + (ranges[1][1].number-ranges[1][0].number)
                        / divisions[1].number ;
    TH2I * h_correlation = new TH2I( 
                            "histogram" , "histogram" ,
                            (unsigned int)divisions[0].number , 
                            ranges[0][0].number , 
                            upLimit[0] ,
                            (unsigned int)divisions[1].number , 
                            ranges[1][0].number , 
                            upLimit[1]
                        ) ;
    
    TGraphErrors * g_correlation = new TGraphErrors() ; 
    g_correlation->SetName(  "graph" ) ;
    g_correlation->SetTitle( "graph" ) ;

    TH1I * h_differences ;
    if( calculateDifferences )
        h_differences = new TH1I(
                                "differences" , "differences" ,
                                diffBins , diffRange[0] , diffRange[1]
                              ) ;
  
    double a , b ;

    if( histData ){

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
            for(unsigned int b=0; b<bins[i]; b++)
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
        
        bool pixelSpecified = false ;
        if( !( pixelList.empty() ) ) pixelSpecified = true ;
     
        for(unsigned int x=1; x<=bins[1]; x++){
            for(unsigned int y=1; y<=bins[0]; y++){
                if( 
                    ! useRowsNcolumns["ROWS"   ][y] 
                    || 
                    ! useRowsNcolumns["COLUMNS"][x] 
                )
                    continue ;
                if( pixelSpecified ){
    //                if( exclude ){
    //                    if( 
    //                        pixelList.find(y) != pixelList.end() 
    //                        && 
    //                        std::find(
    //                                    pixelList[y].begin() ,
    //                                    pixelList[y].end() ,
    //                                    x
    //                                )
    //                            !=
    //                                pixelList[y].end()
    //                    )
    //                        continue ;
    //                }
    //                else{
    //                    if( 
    //                        pixelList.find(y) == pixelList.end() 
    //                        || 
    //                        std::find(
    //                                    pixelList[y].begin() ,
    //                                    pixelList[y].end() ,
    //                                    x
    //                                )
    //                            ==
    //                                pixelList[y].end()
    //                    )
    //                        continue ;
    //                }
                    if( 
                        pixelList.find(y) != pixelList.end() 
                        && 
                        std::find( 
                                    pixelList[y].begin() , 
                                    pixelList[y].end() , 
                                    x 
                                 )
                            !=
                                pixelList[y].end()
                    ){
                        if( exclude ) continue ;
                    }
                    else{
                        if( !( exclude ) ) continue ;
                    }
                }
                a = hists[0]->GetBinContent( x , y ) ;
                b = hists[1]->GetBinContent( x , y ) ;
                if( toDiscard( a ) ) continue ;
                if( toDiscard( b ) ) continue ;
                h_correlation->Fill( a , b ) ;
                if( ranges[0][0].setting && a < ranges[0][0].number ) continue ;
                if( ranges[0][1].setting && a > ranges[0][1].number ) continue ;
                if( ranges[1][0].setting && b < ranges[1][0].number ) continue ;
                if( ranges[1][1].setting && b > ranges[1][1].number ) continue ;
                g_correlation->SetPoint( g_correlation->GetN() , a , b ) ;
                if( calculateDifferences ) h_differences->Fill( b - a ) ;
                a = hists[0]->GetBinError( x , y )  ;
                b = hists[1]->GetBinError( x , y ) ;
                if( toDiscard( a ) || a == -1. ) a = 0. ;
                if( toDiscard( b ) || b == -1. ) b = 0. ;
                if( useErrors ) 
                    g_correlation->SetPointError( 
                                                    g_correlation->GetN()-1 , 
                                                    a , b 
                                                ) ;
            }
        }

    }
    else{
        
        bool intervalsSpecified = false ;
        unsigned int nIntervals = intervals.size() ;
        if( nIntervals > 0 ) intervalsSpecified = true ;
        bool pointInInterval , pointInLow , pointInHigh ;

        double x[2] , y[2] , e[2] ;
        unsigned int equalXpoint ;
        unsigned int startIndex , stops ;
        int useIndex ;
        for(unsigned int p=0; p<bins[0]; p++){
            graphs[0]->GetPoint( p , x[0] , y[0] ) ;
            if( intervalsSpecified ){
                pointInInterval = false ;
                for(unsigned int i=0; i<nIntervals; i++){
                    pointInLow  = false ;
                    pointInHigh = false ;
                    if( intervals.at(i).at(0).setting ){
                        if( intervals.at(i).at(0).number <= x[0] )
                            pointInLow = true ;
                    }
                    else pointInLow = true ;
                    if( intervals.at(i).at(1).setting ){
                        if( intervals.at(i).at(1).number >= x[0] )
                            pointInHigh = true ;
                    }
                    else pointInHigh = true ;
                    if( pointInLow && pointInHigh ){ 
                        pointInInterval = true ;
                        break ;
                    }
                }
                if( !( pointInInterval ) ) continue ;
            }
            e[0] = graphs[0]->GetErrorY( p ) ;
            if( p < bins[1] ){ 
                graphs[1]->GetPoint( p , x[1] , y[1] ) ;
                e[1] = graphs[1]->GetErrorY( p ) ;
            }
            if( p >= bins[1] || x[1] != x[0] ){
                count = 0 ;
                if( maxDistance.setting ){
                    if( p < bins[1] ) startIndex = p ;
                    else{
                        startIndex = (unsigned int)( 
                                (double)p / (double)bins[0] * (double)bins[1] 
                        ) ;
                    }
                    for(unsigned int o=0; o<bins[1]; o++){
                        stops = 0 ;
                        for(int s=-1; s<2; s+=2){
                            useIndex = (int)startIndex + s * (int)o ;
                            if( useIndex < 0 || useIndex >= bins[1] ){ 
                                stops++ ;
                                continue ;
                            }
                            graphs[1]->GetPoint( useIndex , x[1] , y[1] ) ;
                            if( 
                                x[1] == x[0] 
                                || 
                                abs( x[1] - x[0] ) < maxDistance.number
                            ){
                                equalXpoint = useIndex ;
                                count++ ;
                                if( useFirstOccurence ){
                                    stops = 2 ;
                                    break ;
                                }
                            }
                        }
                        if( stops > 1 ) break ;
                    }
                }
                else{
                    for(unsigned int o=0; o<bins[1]; o++){
                        graphs[1]->GetPoint( o , x[1] , y[1] ) ;
                        if( x[1] == x[0] ){
                            equalXpoint = o ;
                            count++ ;
                            if( useFirstOccurence ) break ;
                        }
                    }
                }
                if( count < 1 ) continue ;
                if( count > 1 ){
                    cout << " ERROR : data ambiguous " << endl ;
                    h_correlation->Delete() ;
                    outfile->Delete() ;
                    return 11 ;
                }
                graphs[1]->GetPoint( equalXpoint , x[1] , y[1] ) ;
                if( useErrors ) e[1] = graphs[1]->GetErrorY( equalXpoint ) ;
            } 
            if( intervalsSpecified ){
                pointInInterval = false ;
                for(unsigned int i=0; i<nIntervals; i++){
                    pointInLow  = false ;
                    pointInHigh = false ;
                    if( intervals.at(i).at(0).setting ){
                        if( intervals.at(i).at(0).number <= x[1] )
                            pointInLow = true ;
                    }
                    else pointInLow = true ;
                    if( intervals.at(i).at(1).setting ){
                        if( intervals.at(i).at(1).number >= x[1] )
                            pointInHigh = true ;
                    }
                    else pointInHigh = true ;
                    if( pointInLow && pointInHigh ){ 
                        pointInInterval = true ;
                        break ;
                    }
                }
                if( !( pointInInterval ) ) continue ;
            }
            a = y[0] ;
            b = y[1] ;
            if( toDiscard( a ) ) continue ;
            if( toDiscard( b ) ) continue ;
            h_correlation->Fill( a , b ) ;
            if( ranges[0][0].setting && a < ranges[0][0].number ) continue ;
            if( ranges[0][1].setting && a > ranges[0][1].number ) continue ;
            if( ranges[1][0].setting && b < ranges[1][0].number ) continue ;
            if( ranges[1][1].setting && b > ranges[1][1].number ) continue ;
            g_correlation->SetPoint( g_correlation->GetN() , a , b ) ;
            if( calculateDifferences ) h_differences->Fill( b - a ) ;
            a = e[0] ;
            b = e[1] ;
            if( toDiscard( a ) || a == -1. ) a = 0. ;
            if( toDiscard( b ) || b == -1. ) b = 0. ;
            if( useErrors ) 
                g_correlation->SetPointError( 
                                                g_correlation->GetN()-1 , 
                                                a , b 
                                            ) ;
        }

    }

    if( !( titlesDefined ) ){
        if(      sameSource[0] || sourceNames[0][0] == sourceNames[1][0] ){
            axisTitles[0] = sourceNames[0][1] ;
            axisTitles[1] = sourceNames[1][1] ;
        }
        else if( sameSource[1] || sourceNames[0][1] == sourceNames[1][1] ){
            axisTitles[0] = sourceNames[0][0] ;
            axisTitles[1] = sourceNames[1][0] ;
        }
        else{
            axisTitles[0] = sourceNames[0][0]+" "+sourceNames[0][1] ;
            axisTitles[1] = sourceNames[1][0]+" "+sourceNames[1][1] ;
        }
        for(unsigned int a=0; a<2; a++){
            name = axisTitles[a] ;
            name = name.ReplaceAll( ".root" , "" ) ;
            if( name.Contains("/") )
                name = name( name.Last('/')+1 , name.Sizeof() ) ;
            axisTitles[a] = name.Data() ;
        }
    }

    outfile->cd() ;
    
    h_correlation->Write() ;
    g_correlation->Write() ;
    if( calculateDifferences ) h_differences->Write() ;
        
    if( draw ){

        if( toDraw == "DIFF" && !( calculateDifferences) ){
            cout << " WARNING : no difference-range specified " << endl ;
            toDraw = "GRAPH" ;
        }
   
        if(
            ( toDraw == "GRAPH" && g_correlation->GetN() < 1 )
            ||
            ( toDraw == "DIFF"  && h_differences->GetEntries() < 1 )
        ){
            cout << " WARNING : no compatible data found " << endl ;
            if( h_correlation->GetEntries() > 1 ){
                gStyle->SetOptStat(10000) ;
                toDraw = "HIST" ;
            }
            else{
                outfile->Close() ;
                return -1 ;
            }
        }
 
        TApplication app("app", &argc, argv) ; 
        name = outname ;
        name = name.ReplaceAll( ".root" , "" ) ;
        TCanvas * can = new TCanvas( name , name , 700 , 600 ) ;
        
        if( toDraw == "HIST" ){

            h_correlation->GetXaxis()->SetTitle( axisTitles[0].c_str() ) ;
            h_correlation->GetYaxis()->SetTitle( axisTitles[1].c_str() ) ;
            
            h_correlation->Draw("COLZ") ;

        }
        else if( toDraw == "DIFF" ){

            if( titlesDefined ){
                h_differences->GetXaxis()->SetTitle( axisTitles[0].c_str() ) ;
                h_differences->GetYaxis()->SetTitle( axisTitles[1].c_str() ) ;
            }
            else{
                name = axisTitles[1] + " - " + axisTitles[0] ;
                h_differences->GetXaxis()->SetTitle( name ) ;
                h_differences->GetYaxis()->SetTitle( "#" ) ;
            }
            gStyle->SetOptStat(1111110) ;

            h_differences->Draw() ;

        }
        else{

            g_correlation->GetXaxis()->SetTitle( axisTitles[0].c_str() ) ;
            g_correlation->GetYaxis()->SetTitle( axisTitles[1].c_str() ) ;
            g_correlation->SetMarkerStyle(20) ;
            if( g_correlation->GetN() < 100 )
                 g_correlation->SetMarkerSize( 1.5 ) ;
            else g_correlation->SetMarkerSize( 1. ) ;

            g_correlation->Draw("AP") ;

        }
        
        if( print ){
            showing() ;
            name = can->GetName() ;
            name += ".pdf" ;
            can->Print(name);
            can->Close() ;
        }
        else padWaiting() ;

    }
    
    outfile->Close() ;
    
    return 0 ;

}
