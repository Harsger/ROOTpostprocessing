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
    SpecifiedNumber textDataFormat ;
    string formula = "x" ;
    vector<double> adjustX = { 1. , 0. } ;
    bool toFlip = false ;
    bool writeErrors[2] = { false , false } ;
    bool setErrors[2]   = { false , false } ;

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
    
    for(unsigned int s=0; s<nSets; s++){

        if( referenceGraphs.size() > 0 ){
            for(unsigned int r=0; r<referenceGraphs.size(); r++)
                referenceGraphs.at(r)->Delete() ;
            referenceGraphs.clear() ;
        }
        
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
            }
        }
        name  = title ;
        name += "_raw" ;
        sourceGraph->SetName(  name ) ;
        sourceGraph->SetTitle( name ) ;
        
        for(unsigned int c=nextIndex; c<nWords; c++){

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
        else if( referenceGraphs.size() != nParameter ){
            cout << " ERROR : at " << title 
                 << " unequal number of parameters and references "
                 << "(" << referenceGraphs.size() << ")" << endl ;
            continue ;
        }
        
        unsigned int nPoints = sourceGraph->GetN() ;
        cout << " " << title << " \t #: " << nPoints << endl ;
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
                referenceGraphs.at(r)->GetPoint( p , rX , rY ) ;
                if( sX != rX ){
                    differentXcount++ ;
                    skipPoint = true ;
                    break ;
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

        outfile->cd() ;
        sourceGraph->Write() ;
        for(r=0; r<nParameter; r++){
            referenceGraphs.at(r)->Write() ;
            referenceGraphs.at(r)->Delete() ;
        }
        if( nParameter > 0 ) referenceGraphs.clear() ;
        resultGraph->Write() ;
        
    }
    
    outfile->Close() ;
    
    return 0 ;
    
}

