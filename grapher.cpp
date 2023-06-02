#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){
    
    if( argc < 2 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }

    TString filename = argv[1] ;

    vector< vector<string> > parameter = getInput( filename.Data() );

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };
    SpecifiedNumber textDataFormat ;
    string formula = "x" ;
    vector<double> adjustX = { 1. , 0. } ;
    bool toFlip = false ;
    bool writeErrors[2] = { false , false } ;
    bool setErrors[2]   = { false , false } ;
    bool parameterArguments = false ;
    SpecifiedNumber averageOver ;
    bool writeRaw = true ;

    vector< vector<string> > filesNtitlesNreferences ;

    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ) continue ;

        int fileORgraph = -1 ;
        if( parameter.at(r).at(0).compare("FILE") == 0  ) 
            fileORgraph = 0 ;
        else if( parameter.at(r).at(0).compare("GRAPH") == 0  ) 
            fileORgraph = 1 ;

        if( fileORgraph > -1 && parameter.at(r).size() > 2 ){
            preNsuffix[fileORgraph][0] = parameter.at(r).at(1) ;
            preNsuffix[fileORgraph][1] = parameter.at(r).at(2) ;
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
            parameter.at(r).at(0).compare("FUNCTION") == 0 
            &&
            parameter.at(r).size() > 1
        ){
            formula = parameter.at(r).at(1) ;
            for(unsigned int c=2; c<parameter.at(r).size(); c++){               
                if( parameter.at(r).at(c).rfind("#",0) == 0 ) break ;
                formula += " " ;
                formula += parameter.at(r).at(c) ;
            }
            continue ;
        }

        if( 
            parameter.at(r).at(0).compare("ADJUSTX") == 0 
            &&
            parameter.at(r).size() > 2
        ){
            if( parameter.at(r).at(1).compare( "%" ) != 0 )
                adjustX.at(0) = atof( parameter.at(r).at(1).c_str() ) ;
            if( parameter.at(r).at(2).compare( "%" ) != 0 )
                adjustX.at(1) = atof( parameter.at(r).at(2).c_str() ) ;
            continue ;
        }

        if(
            parameter.at(r).at(0).compare("AVERAGE") == 0
            &&
            parameter.at(r).size() > 1
        ){
            averageOver = SpecifiedNumber( atoi(
                                                parameter.at(r).at(1).c_str()
                                        ) ) ;
            if( averageOver.number < 2 ){
                cout << " WARNING : averaging must be over at leat two values"
                     << " -> will be skipped " << endl ;
                averageOver.setting = false ;
            }
            continue ;
        }

        if( parameter.at(r).at(0).compare("FLIP") == 0 ){
            toFlip = true ;
            continue ;
        }

        if( parameter.at(r).at(0).compare("WRITEERRORS") == 0 ){
            if( parameter.at(r).size() > 1 ){
                if(      parameter.at(r).at(1) == "Y" ) writeErrors[1] = true ;
                else if( parameter.at(r).at(1) == "X" ) writeErrors[0] = true ;
                else                                    writeErrors[1] = true ;
            }
            else 
                writeErrors[1] = true ;
            continue ;
        }

        if( parameter.at(r).at(0).compare("SETERRORS") == 0 ){
            if( parameter.at(r).size() > 2 ){
                if( parameter.at(r).at(1) == "1" ) setErrors[0] = true ;
                if( parameter.at(r).at(2) == "1" ) setErrors[1] = true ;
            }
            else if( parameter.at(r).size() > 1 ){
                if(      parameter.at(r).at(1) == "X" ) setErrors[0] = true ;
                else if( parameter.at(r).at(1) == "Y" ) setErrors[1] = true ;
            }
            else{ 
                setErrors[0] = true ;
                setErrors[1] = true ;
            }
            continue ;
        }
        
        if( parameter.at(r).at(0).compare("PARAMETERARGUMENTS") == 0 ){
            parameterArguments = true ;
            continue ;
        }

        if( parameter.at(r).at(0).compare("SKIPRAW") == 0 ){
            writeRaw = false ;
            continue ;
        }

        if( parameter.at(r).size() > 1 ){
            filesNtitlesNreferences.push_back( parameter.at(r) ) ;
        }

    }
    
    if( 
        ( writeErrors[0] || writeErrors[1] ) 
        &&
        (   setErrors[0] ||   setErrors[1] ) 
    ){
        cout << " ERROR : either write or set errors - not both " << endl ;
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

    TString name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    outfile->cd() ;
    
    TF1 * function = new TF1( "function" , formula.c_str() ) ;
    unsigned int nParameter = function->GetNpar() ;
    
    if( setErrors[0] || setErrors[1] ){
        if( setErrors[0] && setErrors[1] ){
            nParameter = 2 ;
            function = new TF1( "function" , "x+0.*[0]+0.*[1]" ) ;
        }
        else{
            nParameter = 1 ;
            function = new TF1( "function" , "x+0.*[0]" ) ;
        }
    }

    TGraphErrors * sourceGraph ;
    TGraphErrors * fillGraph ;
    vector<TGraphErrors*> referenceGraphs ;
    TGraphErrors * resultGraph ;

    TString title ;    
    unsigned int nSets = filesNtitlesNreferences.size() ;
    unsigned int nWords , nextIndex ;
    vector<double> functionParameter ;
    
    for(unsigned int s=0; s<nSets; s++){

        if( referenceGraphs.size() > 0 ){
            for(unsigned int r=0; r<referenceGraphs.size(); r++)
                referenceGraphs.at(r)->Delete() ;
            referenceGraphs.clear() ;
        }
        functionParameter.clear() ;
        
        nWords = filesNtitlesNreferences.at(s).size() ;
        if( nWords < 2 ) continue ;
        nextIndex = 2 ;
        title = filesNtitlesNreferences.at(s).at(0) ;

        name  = preNsuffix[0][0] ;
        if( filesNtitlesNreferences.at(s).at(1) != "%" )
            name += filesNtitlesNreferences.at(s).at(1) ;
        name += preNsuffix[0][1] ;
        if( name.EndsWith(".root") ){
            if( nWords < 3 ) continue ; 
            nextIndex++ ;
            TFile * input = new TFile( name , "READ" ) ;
            if( input->IsZombie() ){
                cout << " ERROR : opening " << name << endl ;
                input->Close() ;
                continue ;
            }
            name  = preNsuffix[1][0] ;
            if( filesNtitlesNreferences.at(s).at(2) != "%" )
                name += filesNtitlesNreferences.at(s).at(2) ;
            name += preNsuffix[1][1] ;
            if( input->Get(name) == NULL ){
                cout << " ERROR : reading " << name 
                     << " in " << input->GetName() << endl ;
                input->Close() ;
                continue ;
            }
            sourceGraph = (TGraphErrors*)input->Get(name) ;
            input->Close() ;
        }
        else{
            if( textDataFormat.setting )
                sourceGraph = new TGraphErrors( 
                                            name , 
                                            textDataFormat.specifier.c_str() 
                                        ) ;
            else{
                sourceGraph = new TGraphErrors( name ) ;
                if( sourceGraph->GetN() < 1 )
                    sourceGraph = new TGraphErrors( name , "%lg,%lg" ) ;
                if( sourceGraph->GetN() < 1 ){
                    vector< vector<string> > data = getInput( name.Data() ) ;
                    if( data.size() > 1 ){
                        unsigned int nRows = data.size() ;
                        sourceGraph = new TGraphErrors() ;
                        for(unsigned int r=0; r<nRows; r++){
                            sourceGraph->SetPoint(
                                                r , r ,
                                                atof( data.at(r).at(0).c_str() )
                                           ) ;
                        }
                    }
                }
                if( sourceGraph->GetN() < 1 ){
                    cout << " ERROR : no data found in " << name << endl ;
                    continue ;
                }
            }
        }
        name  = title ;
        name += "_raw" ;
        sourceGraph->SetName(  name ) ;
        sourceGraph->SetTitle( name ) ;
        
        for(unsigned int c=nextIndex; c<nWords; c++){
            
            if( parameterArguments ){
                name = filesNtitlesNreferences.at(s).at(c) ;
                if( name.IsFloat() ) 
                    functionParameter.push_back( atof( name.Data() ) ) ;
                continue ;
            }

            name  = preNsuffix[0][0] ;
            if( filesNtitlesNreferences.at(s).at(c) != "%" )
                name += filesNtitlesNreferences.at(s).at(c) ;
            name += preNsuffix[0][1] ;
            if( name.EndsWith(".root") ){
                if( nWords < c+1 ) break ; 
                TFile * input = new TFile( name , "READ" ) ;
                if( input->IsZombie() ){
                    cout << " ERROR : opening " << name << endl ;
                    input->Close() ;
                    break ;
                }
                name  = preNsuffix[1][0] ;
                if( filesNtitlesNreferences.at(s).at(c+1) != "%" )
                    name += filesNtitlesNreferences.at(s).at(c+1) ;
                name += preNsuffix[1][1] ;
                if( input->Get(name) == NULL ){
                    cout << " ERROR : reading " << name 
                         << " in " << input->GetName() << endl ;
                    input->Close() ;
                    break ;
                }
                fillGraph = (TGraphErrors*)input->Get(name) ;
                input->Close() ;
                c++ ;
            }
            else{
                if( textDataFormat.setting )
                    fillGraph = new TGraphErrors( 
                                            name , 
                                            textDataFormat.specifier.c_str() 
                                        ) ;
                else{
                    fillGraph = new TGraphErrors( name ) ;
                    if( fillGraph->GetN() < 1 )
                        fillGraph = new TGraphErrors( name , "%lg,%lg" ) ;
                    if( fillGraph->GetN() < 1 ){
                        vector< vector<string> > data =
                                                    getInput( name.Data() ) ;
                        if( data.size() > 1 ){
                            unsigned int nRows = data.size() ;
                            fillGraph = new TGraphErrors() ;
                            for(unsigned int r=0; r<nRows; r++){
                                fillGraph->SetPoint(
                                    r , r , atof( data.at(r).at(0).c_str() )
                                ) ;
                            }
                        }
                    }
                    if( fillGraph->GetN() < 1 ){
                        cout << " ERROR : no data found in " << name << endl ;
                        continue ;
                    }
                }
            }
            name  = title ;
            name += "_ref" ;
            name += referenceGraphs.size() ;
            fillGraph->SetName(  name ) ;
            fillGraph->SetTitle( name ) ;
            referenceGraphs.push_back( fillGraph ) ;        

        }
        
        if( writeErrors[0] || writeErrors[1] ){
            nParameter = 0 ;
            referenceGraphs.clear() ;
        }
        else if( parameterArguments ){
            if( functionParameter.size() != nParameter ){
                cout << " ERROR : at " << title 
                    << " unequal number of parameters and arguments "
                    << "(" << functionParameter.size() << ")" << endl ;
                continue ;
            }
        }
        else if( referenceGraphs.size() != nParameter ){
            cout << " ERROR : at " << title 
                 << " unequal number of parameters and references "
                 << "(" << referenceGraphs.size() << ")" << endl ;
            continue ;
        }
        
        unsigned int nPoints = sourceGraph->GetN() ;
        cout << " " << title << " \t #: " << nPoints << endl ;
        if( !parameterArguments ){
            bool problematicReference = false ;
            for(unsigned int p=0; p<nParameter; p++){
                if( referenceGraphs.at(p)->GetN() != nPoints ){
                    cout << " ERROR : at " << title
                        << " unequal points in reference " << p
                        << "(nPoints=" << referenceGraphs.at(p)->GetN() 
                        << ")" << endl ;
                    problematicReference = true ;
                    break ;  
                }
            }
            if( problematicReference ) continue ;
        }
        
        resultGraph = new TGraphErrors() ;
        resultGraph->SetName(  title ) ;
        resultGraph->SetTitle( title ) ;
        
        double sX , sY , rX , rY , wX , wY ; 
        unsigned int r , differentXcount = 0 ;        
        bool skipPoint ;

        for(unsigned int p=0; p<nPoints; p++){
            
            sourceGraph->GetPoint( p , sX , sY ) ;
            
            skipPoint = false ;
            for(r=0; r<nParameter; r++){ 
                if( parameterArguments ) rY = functionParameter.at(r) ;
                else{
                    referenceGraphs.at(r)->GetPoint( p , rX , rY ) ;
                    if( sX != rX ){
                        differentXcount++ ;
                        skipPoint = true ;
                        break ;
                    }
                }
                function->SetParameter( r , rY ) ;
            }
            if( skipPoint ) continue ;
            
            wX = sX * adjustX.at(0) + adjustX.at(1) ;
            wY = function->Eval( sY ) ;
                            
            if( toFlip ) swap( wX , wY ) ;
            
            if(      writeErrors[1] )
                wY = sourceGraph->GetErrorY( p ) ;
            else if( writeErrors[0] )
                wY = sourceGraph->GetErrorX( p ) ;
            
            if( setErrors[0] || setErrors[1]  ) 
                wY = sY ;
            
            resultGraph->SetPoint( resultGraph->GetN() , wX , wY ) ;
            
            if( setErrors[0] || setErrors[1] ){
                if( setErrors[0] && setErrors[1] ){
                    resultGraph->SetPointError( 
                                                resultGraph->GetN()-1 , 
                                                function->GetParameter( 0 ) , 
                                                function->GetParameter( 1 ) 
                                              ) ;
                }
                else if( setErrors[0] ){
                    resultGraph->SetPointError( 
                                                resultGraph->GetN()-1 , 
                                                function->GetParameter( 0 ) , 
                                                sourceGraph->GetErrorY( p ) 
                                              ) ;
                }
                else if( setErrors[1] ){
                    resultGraph->SetPointError( 
                                                resultGraph->GetN()-1 , 
                                                sourceGraph->GetErrorX( p ) , 
                                                function->GetParameter( 0 ) 
                                              ) ;
                }
            }
            
        }
        
        if( differentXcount > 0 ) 
            cout << " # problematic points : " << differentXcount << endl ;

        if( averageOver.setting ){
            TGraphErrors * swapGraph = new TGraphErrors() ;
            resultGraph->SetName(  "oldResult" ) ;
            resultGraph->SetTitle( "oldResult" ) ;
            swapGraph->SetName(  title ) ;
            swapGraph->SetTitle( title ) ;
            TGraphErrors * stdvGraph = new TGraphErrors() ;
            name = title ;
            name += "_stdv" ;
            stdvGraph->SetName(  name ) ;
            stdvGraph->SetTitle( name ) ;
            unsigned int count = 0 ;
            unsigned int toAverage = (unsigned int)averageOver.number ;
            nPoints = resultGraph->GetN() ;
            double mean[2][2] = { { 0. , 0. } , { 0. , 0. } } ;
            double stdv = 0. ;
            for(unsigned int p=0; p<nPoints; p++){
                resultGraph->GetPoint( p , sX , sY ) ;
                rX = resultGraph->GetErrorX( p ) ;
                rY = resultGraph->GetErrorY( p ) ;
                if(
                    !(
                        toDiscard( sX ) || toDiscard( sY )
                        ||
                        toDiscard( rX ) || toDiscard( rY )
                    )
                ){
                    count++ ;
                    mean[0][0] += sX ;
                    mean[1][0] += sY ;
                    mean[0][1] += rX ;
                    mean[1][1] += rY ;
                    stdv += ( sY * sY ) ;
                }
                if( count >= toAverage || p+1 == nPoints ){
                    swapGraph->SetPoint(
                                            swapGraph->GetN() ,
                                            mean[0][0] / (double)count ,
                                            mean[1][0] / (double)count
                                       ) ;
                    swapGraph->SetPointError(
                                            swapGraph->GetN()-1 ,
                                            mean[0][1] / (double)count ,
                                            mean[1][1] / (double)count
                                       ) ;
                    stdvGraph->SetPoint(
                                            stdvGraph->GetN() ,
                                            mean[0][0] / (double)count ,
                                            sqrt(
                                                    (
                                                        stdv
                                                        - mean[1][0]
                                                        * mean[1][0]
                                                        / (double)count
                                                    )
                                                    / ( (double)count - 1. )
                                            )
                                    ) ;
                    count = 0 ;
                    mean[0][0] = 0. ;
                    mean[1][0] = 0. ;
                    mean[0][1] = 0. ;
                    mean[1][1] = 0. ;
                    stdv = 0. ;
                }
            }
            resultGraph->Delete() ;
            resultGraph = swapGraph ;
            outfile->cd() ;
            stdvGraph->Write() ;
            stdvGraph->Delete() ;
        }

        outfile->cd() ;
        if(
            !writeRaw
            ||
            formula == "x"
            &&
            !parameterArguments
            &&
            adjustX.at(0) == 1. && adjustX.at(1) == 0.
            &&
            !averageOver.setting
            &&
            !toFlip
            &&
            !writeErrors[0] && !writeErrors[1]
            &&
            !setErrors[0] && !setErrors[1]
        )
            sourceGraph->Delete() ;
        else
            sourceGraph->Write() ;
        if( !parameterArguments ){
            for(r=0; r<nParameter; r++){
                if( writeRaw ) referenceGraphs.at(r)->Write() ;
                referenceGraphs.at(r)->Delete() ;
            }
        }
        if( nParameter > 0 ){ 
            referenceGraphs.clear() ;
            functionParameter.clear() ;
        }
        resultGraph->Write() ;
        
    }
    
    outfile->Close() ;
    
    return 0 ;
    
}

