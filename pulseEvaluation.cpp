#include "postprocessing.h"

using namespace std;

class Measurement{
public :
    
    string name , file , curve ;
    vector<string> settings ;
    
    Extremum signal[2] ;
    Extremum baseline ;
    
    map< TString , Statistics > results ;
    
    Measurement( string n , string f , string c ){
        name  = n ;
        file  = f ;
        curve = c ;
        signal[0] = Extremum{string{"min"}} ;
        signal[1] = Extremum{string{"max"}} ;
        baseline  = Extremum{string{"max"}} ;
        results["signal"   ] = Statistics() ;
        results["offfit"   ] = Statistics() ;
        results["offset"   ] = Statistics() ;
        results["variation"] = Statistics() ;
        results["maxfit"   ] = Statistics() ;
        results["maximum"  ] = Statistics() ;
        results["fallfit"  ] = Statistics() ;
        results["falltime" ] = Statistics() ;
        results["risefit"  ] = Statistics() ;
        results["risetime" ] = Statistics() ;
    }
    
} ;

unsigned int fitting( TGraphErrors * data , TF1 * function ){
    
    if( data == NULL || function == NULL || data->GetN() < 1 ){
        cout << " ERROR : data not fittable " << endl ;
        return 0 ;
    }
    
    data->Fit( function , "RQB" ) ;
    
    unsigned int count = 1 ;
    double chi2ndf = function->GetChisquare() / function->GetNDF() ;
    
    while( count < 10 && ( chi2ndf < 0.5 || chi2ndf > 2. ) ){
        data->Fit( function , "RQB" ) ;
        chi2ndf = function->GetChisquare() / function->GetNDF() ;
        count++ ;
    }
    
    return count ;
}

int main(int argc, char *argv[]){

    if( argc < 2 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }
    string parameterFileName = argv[1] ;
    
    bool show[2] = { false , false } ;
    if( argc > 2 ){ 
        show[0] = true ;
        if( ( (string)(argv[2]) ).compare("ALL") == 0 ) show[1] = true ;
    }
    TApplication app("app", &argc, argv) ;    
    plotOptions() ;
    gStyle->SetPadRightMargin( 0.03 ) ;
    
    TString name , title , mode ;
    vector< vector<string> > parameter = getInput( parameterFileName );
    
    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };

    vector< Measurement > measurements ;
    vector<string> labels ;
    vector<string> toPrint ;
    unsigned int nPeriods = 0 , nMeas = 0 , nLab = 0 ;
    double valueRange[2] = { 0. , 1. } ;
    double valueBins = 100 ;
    vector< vector<string> > toCorrelate = {
        { "offset"   , "offfit"  } ,
        { "maximum"  , "maxfit"  } ,
        { "falltime" , "fallfit" } ,
        { "risetime" , "risefit" } 
    } ;

    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ) continue ;

        int specifier = -1 ;

        if(      parameter.at(r).at(0).compare("FILE")  == 0 ) specifier = 0 ;
        else if( parameter.at(r).at(0).compare("GRAPH") == 0 ) specifier = 1 ;
        else if( parameter.at(r).at(0).compare("RANGE") == 0 ) specifier = 2 ;

        if( specifier > -1 && parameter.at(r).size() > 2 ){
            if( specifier < 2 ){
                preNsuffix[specifier][0] = parameter.at(r).at(1) ;
                preNsuffix[specifier][1] = parameter.at(r).at(2) ;
            }
            else if( specifier == 2 ){
                if( parameter.at(r).size() > 3 ){
                    valueRange[0] = atof( parameter.at(r).at(1).c_str() ) ;
                    valueRange[1] = atof( parameter.at(r).at(2).c_str() ) ;
                    valueBins     = atoi( parameter.at(r).at(3).c_str() ) ;
                }
            }
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("LABELS") == 0  
            &&
            parameter.at(r).size() > 1
        ){
            for(unsigned int c=1; c<parameter.at(r).size(); c++){
                labels.push_back( parameter.at(r).at(c) ) ;
            }
            nLab = labels.size() ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("PRINT") == 0  
            &&
            parameter.at(r).size() > 1
        ){
            for(unsigned int c=1; c<parameter.at(r).size(); c++){
                toPrint.push_back( parameter.at(r).at(c) ) ;
            }
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("CORRELATE") == 0  
            &&
            parameter.at(r).size() > 2
        ){
            toCorrelate.push_back(
                { parameter.at(r).at(1) , parameter.at(r).at(2) }
            ) ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("PERIODS") == 0  
            &&
            parameter.at(r).size() > 1
        ){
            nPeriods = atoi( parameter.at(r).at(1).c_str() ) ;
            continue ;
        }
        
        if( parameter.at(r).size() > 3 ){
            measurements.push_back(
                Measurement(
                                parameter.at(r).at(0) ,
                                parameter.at(r).at(1) ,
                                parameter.at(r).at(2)
                           )
            ) ;
            nMeas = measurements.size() ;
            for(unsigned int c=3; c<parameter.at(r).size(); c++){
                measurements.at( nMeas - 1 ).settings.push_back(
                    parameter.at(r).at(c).c_str()
                ) ;
            }
        }

    }
    
    if( nMeas < 2 || nLab < 1 || nPeriods < 1 ){
        cout << " ERROR : not enough specifications " << endl ;
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
    
    name = parameterFileName ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    name += ".root" ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    map< TString , TGraphErrors* > resultGraphs ;
    
    TFile * infile ;
    TGraphErrors * data ;
    unsigned int nPoints ;
    double x , y ;
    
    double timeOffset , sampleDuration , periodTime ;
    unsigned int pointsPERperiod , currentPeriod , startIndex , stopIndex ;
    double range[2] , riseRange[2][ nPeriods ] , fallRange[2][ nPeriods ] ;
    double signalThreshold[2] ;
    
    Statistics wholeSignal , periodBase ;
    
    Extremum peaks[nPeriods] ; 
    Extremum valleys[nPeriods+1] ;
    for(unsigned int p=0; p<nPeriods+1; p++){
        valleys[p] = Extremum{ string{"min"} } ;
    }
    
    TString formula = "[0] + ( [1] - [0] ) * exp( - ( x - [2] ) / [3] )" ;
    
    map< TString , TGraphErrors* > correlations ;
    for( auto c : toCorrelate ){
        Measurement test( "test" , "" , "" ) ;
        if(
            test.results.find( c.at(0) ) == test.results.end()
            ||
            test.results.find( c.at(1) ) == test.results.end()
        )
            continue ;
        name = c.at(1) ;
        name += "_VS_" ;
        name += c.at(0) ;
        correlations[name] = new TGraphErrors() ;
        correlations[name]->SetName(  name ) ;
        correlations[name]->SetTitle( name ) ;
    }
    
    TH1I * hist = new TH1I( 
                            "values" , "values" , 
                            valueBins , valueRange[0] , valueRange[1] 
                          ) ;
    
    for( auto &m : measurements ){
        
        cout << " " << m.name ;
        
        name  = preNsuffix[0][0] ;
        if( m.file != "%" ) name += m.file ;
        name += preNsuffix[0][1] ;
        infile = new TFile( name , "READ" ) ;
        if( infile->IsZombie() ){ 
            cout << " ERROR : file " << name << " can not be opened " << endl ;
            infile->Close() ;
            continue ;
        }
        
        name  = preNsuffix[1][0] ;
        if( m.curve != "%" ) name += m.curve ;
        name += preNsuffix[1][1] ;
        data = (TGraphErrors*)( infile->Get( name ) ) ;
        infile->Close() ;
        if( data == NULL ){ 
            cout << " ERROR : curve " << name << " can not be opened " << endl ;
            continue ;
        }
        
        nPoints = data->GetN() ;
        pointsPERperiod = nPoints / nPeriods ;
        data->GetPoint( 0               , timeOffset     , y ) ;
        data->GetPoint( nPoints-1       , sampleDuration , y ) ;
        data->GetPoint( pointsPERperiod , periodTime     , y ) ;
        sampleDuration -= timeOffset ;
        periodTime     -= timeOffset ;
        
        data->GetPoint( 0 , x , y ) ;
        hist->Fill( y ) ;
        wholeSignal = Statistics( y )  ;
        m.signal[0].put( y , x , 0 ) ;
        m.signal[1].put( y , x , 0 ) ;
        peaks[0].put(    y , x , 0 ) ;
        currentPeriod = 0 ;
        
        for(unsigned int p=1; p<nPoints; p++){
            data->GetPoint( p , x , y ) ;
            hist->Fill( y ) ;
            wholeSignal.fill( y ) ;
            m.signal[0].overwrite( y , x , p ) ;
            m.signal[1].overwrite( y , x , p ) ;
            if( p == ( currentPeriod + 1 ) * pointsPERperiod ){
                currentPeriod++ ; 
                if( currentPeriod < nPeriods ){
                    peaks[currentPeriod].put( y , x , p ) ;
                }
                else{ currentPeriod-- ; }
            }
            else{ peaks[currentPeriod].overwrite( y , x , p ) ; }
        }
        
        wholeSignal.calculate() ;
        m.results["signal"] = wholeSignal ;

        for(unsigned int p=0; p<nPeriods; p++){
            m.results["maximum"].fill( peaks[p].value ) ;
        }
        m.results["maximum"].calculate() ;
        
        stopIndex = hist->GetXaxis()->FindBin( m.results["signal"].mean ) ;
        m.baseline.put(
                    hist->GetBinContent( stopIndex ) ,
                    hist->GetBinCenter(  stopIndex ) ,
                    stopIndex
                );
        for(unsigned int b=1; b<stopIndex; b++){
            m.baseline.overwrite(
                    hist->GetBinContent( b ) , hist->GetBinCenter( b ) , b
            ) ;
        }
        hist->Reset() ;
        if( m.baseline.index == stopIndex )
            cout << " BASE " ;
        
        data->GetPoint( 0 , x , y ) ;
        currentPeriod = 0 ;
        valleys[currentPeriod].put( y , x , 0 ) ;
        for(unsigned int p=1; p<nPoints; p++){
            data->GetPoint( p , x , y ) ;
            if( currentPeriod < nPeriods && p == peaks[currentPeriod].index ){
                currentPeriod++ ;
                valleys[currentPeriod].put( y , x , p ) ;
            }
            else{ valleys[currentPeriod].overwrite( y , x , p ) ; }
        }
        
        signalThreshold[0] = 
                                0.1 * ( 
                                    m.results["maximum"].mean
                                    - 
                                    m.baseline.position
                                ) 
                                + m.baseline.position ;
        signalThreshold[1] = 
                                0.9 * ( 
                                    m.results["maximum"].mean
                                    - 
                                    m.baseline.position
                                ) 
                                + m.baseline.position ;

        if( 
            find( toPrint.begin() , toPrint.end() , "THRESHOLD" ) 
            != 
            toPrint.end() 
        ){
            cout << " Threshold[" << signalThreshold[0] << 
                              "," << signalThreshold[1] << "]" << endl ;
        }

        for(unsigned int p=0; p<nPeriods; p++){
            startIndex = p * pointsPERperiod ;
            stopIndex  = peaks[p].index ;
            for(unsigned int i=startIndex; i<stopIndex; i++){
                data->GetPoint( i , x , y ) ;
                if( y > signalThreshold[1] ) break ;
                range[1] = x ;
                if( y > signalThreshold[0] ) continue ;
                range[0] = x ;
                startIndex = i ;
            }
            m.results["risetime"].fill( range[1] - range[0] ) ;
            riseRange[0][p] = range[0] ;
            riseRange[1][p] = range[1] ;
            stopIndex  = startIndex + pointsPERperiod ;
            if( stopIndex > nPoints ) stopIndex = nPoints ;
            startIndex = peaks[p].index ;
            for(unsigned int i=startIndex; i<stopIndex; i++){
                data->GetPoint( i , x , y ) ;
                if( y < signalThreshold[0] ) break ;
                range[1] = x ;
                if( y < signalThreshold[1] ) continue ;
                range[0] = x ;
            }
            m.results["falltime"].fill( range[1] - range[0] ) ;
            fallRange[0][p] = range[0] ;
            fallRange[1][p] = range[1] ;
        }
        m.results["risetime"].calculate() ;
        m.results["falltime"].calculate() ;
        
        if( 
            find( toPrint.begin() , toPrint.end() , "RANGES" ) 
            != 
            toPrint.end() 
        ){
            cout << endl ;
            for(unsigned int p=0; p<nPeriods; p++){
                cout << " R[" << riseRange[0][p] 
                     << ","   << riseRange[1][p] << "]" << endl;
            }
            for(unsigned int p=0; p<nPeriods; p++){
                cout << " F[" << fallRange[0][p] 
                     << ","   << fallRange[1][p] << "]" << endl;
            }
        }
        
        TF1 ** riseFunction = new TF1*[nPeriods] ;
        for(unsigned int p=0; p<nPeriods; p++){
            title  = m.name ;
            title +=  "_risefit" ;
            title += p ;
            riseFunction[p] = new TF1( title , formula ) ;
            range[0] = riseRange[0][p] ;
            range[1] = 0.5 * ( riseRange[1][p] + fallRange[0][p] ) ;
            if( range[1] < peaks[p].position ) 
                range[1] = peaks[p].position ;
            riseFunction[p]->SetRange( range[0] , range[1] ) ;
            riseFunction[p]->FixParameter( 1 , m.baseline.position ) ;
            riseFunction[p]->SetParameter( 0 , m.signal[1].value ) ;
            riseFunction[p]->FixParameter( 2 , riseRange[0][p] ) ;
            riseFunction[p]->SetParameter( 3 , m.results["risetime"].mean ) ;
            riseFunction[p]->SetParLimits( 3 , 0. , periodTime ) ;
            riseFunction[p]->SetLineColor( p+2 ) ;
            fitting( data , riseFunction[p] ) ;
            m.results["maxfit" ].fill( riseFunction[p]->GetParameter( 0 ) ) ;
            m.results["risefit"].fill( riseFunction[p]->GetParameter( 3 ) ) ;
        }
        m.results["maxfit" ].calculate() ;
        m.results["risefit"].calculate() ;
        
        TF1 ** fallFunction = new TF1*[nPeriods] ;
        for(unsigned int p=0; p<nPeriods; p++){
            title  = m.name ;
            title +=  "_fallfit" ;
            title += p ;
            fallFunction[p] = new TF1( title , formula ) ;
            range[0] = fallRange[0][p] ;
            if( p == nPeriods-1 ) range[1] = sampleDuration+timeOffset ;
            else range[1] = 0.5 * ( fallRange[1][p] + riseRange[0][p+1] ) ;
            if( range[1] < valleys[p+1].position ) 
                range[1] = valleys[p+1].position ;
            fallFunction[p]->SetRange( range[0] , range[1] ) ;
            fallFunction[p]->SetParameter( 0 , m.baseline.position ) ;
            fallFunction[p]->FixParameter( 1 , m.signal[1].value ) ;
            fallFunction[p]->SetParameter( 2 , fallRange[0][p] ) ;
            fallFunction[p]->SetParameter( 3 , m.results["falltime"].mean ) ;
            fallFunction[p]->SetParLimits( 3 , 0. , periodTime ) ;
            fallFunction[p]->SetLineColor( p+2 ) ;
            fitting( data , fallFunction[p] ) ;
            m.results["offfit" ].fill( fallFunction[p]->GetParameter( 0 ) ) ;
            m.results["fallfit"].fill( fallFunction[p]->GetParameter( 3 ) ) ;
        }
        m.results["offfit" ].calculate() ;
        m.results["fallfit"].calculate() ;
        
        for(unsigned int p=0; p<nPeriods-1; p++){
            range[0] = fallRange[1][p  ] + m.results["falltime"].mean ;
            range[1] = riseRange[0][p+1] - m.results["risetime"].mean ;
            if( range[1] > range[0] ){
                startIndex = (unsigned int)( 
                                                (double)nPoints 
                                                * ( range[0] - timeOffset )
                                                / sampleDuration
                                           ) ;
                stopIndex = (unsigned int)( 
                                                (double)nPoints 
                                                * ( range[1] - timeOffset )
                                                / sampleDuration
                                           ) ;
            }
            else{
                cout << " 2! " ;
                startIndex = peaks[p].index + pointsPERperiod * 3 / 5 ;
                stopIndex  = peaks[p].index + pointsPERperiod * 4 / 5 ;
            }
            data->GetPoint( startIndex , x , y ) ;
            periodBase = Statistics( y ) ;
            for(unsigned int i=startIndex+1; i<stopIndex; i++){
                data->GetPoint( i , x , y ) ;
                periodBase.fill( y ) ;
            }
            periodBase.calculate() ;
            m.results["offset"   ].fill( periodBase.mean ) ;
            m.results["variation"].fill( periodBase.stdv ) ;
        }
        m.results["offset"   ].calculate() ;
        m.results["variation"].calculate() ;
        
        if( toPrint.size() > 0 ){
            for(unsigned int p=0; p<toPrint.size(); p++){
                if( m.results.find( toPrint.at(p) ) != m.results.end() ){
                    cout << " " << toPrint.at(p) ;
                    cout << " " << m.results[ toPrint.at(p) ].mean ;
                }
                else if( toPrint.at(p) == "ALL" ){
                    for( auto r : m.results ){
                        cout << " " << r.first << " " << r.second.mean ;
                    }
                    cout << " signal [" 
                                         << m.signal[0].value << ","
                                         << m.signal[1].value << "]" ;
                    cout << " baseline " << m.baseline.position ;
                }
                else if( toPrint.at(p) == "signal" ){
                    cout << " signal [" 
                                         << m.signal[0].value << ","
                                         << m.signal[1].value << "]" ;
                }
                else if( toPrint.at(p) == "baseline" ){
                    cout << " baseline " << m.baseline.position ;
                }
            }
        }
        cout << endl ;
        
        for( auto c : toCorrelate ){
            if(
                m.results.find( c.at(0) ) == m.results.end()
                ||
                m.results.find( c.at(1) ) == m.results.end()
            )
                continue ;
            name  = c.at(1) ;
            name += "_VS_" ;
            name += c.at(0) ;
            correlations[name]->SetPoint(
                                        correlations[name]->GetN() ,
                                        m.results[   c.at(0)].mean ,
                                        m.results[   c.at(1)].mean
                                    ) ;
        }
        
        if( show[0] ){
            data->SetMarkerSize(1.) ;
            data->Draw("AP") ;
            for(unsigned int p=0; p<nPeriods; p++){
                riseFunction[p]->Draw("same") ;
                fallFunction[p]->Draw("same") ;
                if( show[1] ){
                    data->GetPoint( 
                        p * pointsPERperiod , range[0] , y 
                    ) ;
                    range[1] = range[0] + periodTime ;
                    data->GetXaxis()->SetRangeUser( range[0] , range[1] ) ;
                    padWaiting() ;
                }
            }
            data->GetPoint( 0         , range[0] , y ) ;
            data->GetPoint( nPoints-1 , range[1] , y ) ;
            data->GetXaxis()->SetRangeUser( range[0] , range[1] ) ;
            padWaiting() ;
        }
        
        data->Delete() ;
        for(unsigned int p=0; p<nPeriods; p++){
            riseFunction[p]->Delete() ;
            fallFunction[p]->Delete() ;
        }
        
        outfile->cd() ;
        
        if( nLab > m.settings.size() ){
            cout << " ERROR : not enough settings " ;
            cout <<         "(" << m.settings.size() << ")" ;
            cout <<         " for this measurement " ;
            cout <<         " according to #labels = " << nLab << endl ;
            continue ;
        }
        
        for(unsigned int l=0; l<nLab; l++){
        
            x = atof( m.settings.at( l ).c_str() ) ;
            mode = "" ;
            for(unsigned int o=0; o<nLab; o++){
                if( o == l ) continue ;
                mode += labels.at(o) ;
                name = m.settings.at(o) ;
                name = name.ReplaceAll( "." , "d" ) ;
                name = name.ReplaceAll( "-" , "m" ) ;
                mode += name ;
                mode += "_" ;
            }
            name  = "VS_" ;
            name += labels.at(l) ;
            for( auto &r : m.results ){
                title  = mode ;
                title += r.first ;
                title += "_" ;
                title += name ;
                if( resultGraphs.find( title ) == resultGraphs.end() ){
                    resultGraphs[title] = new TGraphErrors() ;
                    resultGraphs[title]->SetName(  title ) ;
                    resultGraphs[title]->SetTitle( title ) ;
                }
                resultGraphs[title]->SetPoint(
                    resultGraphs[title]->GetN() , x , r.second.mean 
                ) ;
            }
            
            for(unsigned int s=0; s<2; s++){
            
                title = mode ;
                if( s == 1 ){ 
                    title += "baseline" ;
                    y = m.baseline.position ;
                }
                else{
                    title += "delta" ;
                    y = m.results["maxfit"].mean - m.results["offfit"].mean ;
                }
                title += name ;
                if( resultGraphs.find( title ) == resultGraphs.end() ){
                    resultGraphs[title] = new TGraphErrors() ;
                    resultGraphs[title]->SetName(  title ) ;
                    resultGraphs[title]->SetTitle( title ) ;
                }
                resultGraphs[title]->SetPoint(
                    resultGraphs[title]->GetN() , x , y
                ) ;
                
            }
            
        }
        
    }

    cout << " writing ... " ;
    
    outfile->cd() ;
    
    for( auto &g : resultGraphs ){
        g.second->Write() ;
    }
    
    for( auto &c : correlations ){
        c.second->Write() ;
    }
    
    outfile->Close() ;

    cout << " done " << endl ;
    
    return 0 ;
}
