#include "postprocessing.h"

using namespace std;

bool lineTOexclude( unsigned int l , SpecifiedNumber specification ){
    bool toExclude = false ;
    if( specification.setting ){
        if( specification.number < 0 ){
            if( (unsigned int)(-1-(int)specification.number) == l ){
                toExclude = true ;
            }
        }
        else if( (unsigned int)specification.number != l ){
            toExclude = true ;
        }
    }
    return toExclude ;
}

bool useLine( unsigned int &l , SpecifiedNumber specification ){
    bool toUse = false ;
    if( specification.setting && !( specification.number < 0 ) ){
        l = specification.number ;
        toUse = true ;
    }
    return toUse ;
}

int main(int argc, char *argv[]){
    
    if( argc < 2 ) return 1 ;

    TString filename = argv[1] ;

    vector< vector<string> > parameter = getInput( filename.Data() );

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            } ;
                            
    vector< vector<string> > dataSpecifier ;
    
    unsigned int dimensions = 0 ;
    unsigned int binning = 0 ;
    unsigned int nbins[2] ;
    double ranges[2][2] ;
    
    bool useWeights = true ;
    bool useOutflow = false ;
    bool toFlip[2] = { false , false } ;
    bool parameterArguments = false ;
    bool writeErrors = false ;
    bool setErrors = false ;
    
    string formula = "x" ;
    
    SpecifiedNumber specCol[2] ;
    SpecifiedNumber specRow[2] ;

    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ) continue ;        
        
        int fileORhist = -1 ;
        if( parameter.at(r).at(0).compare("FILE") == 0  ) 
            fileORhist = 0 ;
        else if( parameter.at(r).at(0).compare("HIST") == 0  ) 
            fileORhist = 1 ;

        if( fileORhist > -1 && parameter.at(r).size() > 2 ){
            preNsuffix[fileORhist][0] = parameter.at(r).at(1) ;
            preNsuffix[fileORhist][1] = parameter.at(r).at(2) ;
            continue ;
        }
        
        if( parameter.at(r).at(0).compare("WEIGHTS") == 0 ){
            useWeights = true ;
            if( 
                parameter.at(r).size() > 1
                &&
                parameter.at(r).at(1).compare("OUTFLOW") == 0
            )
                useOutflow = true ;
            continue ;
        }
        
        if( parameter.at(r).at(0).compare("VALUES") == 0 ){
            useWeights = false ;
            if( 
                parameter.at(r).size() > 1
                &&
                parameter.at(r).at(1).compare("OUTFLOW") == 0
            )
                useOutflow = true ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("COLUMN") == 0  
            &&
            parameter.at(r).size() > 1
        ){
            specCol[0] = SpecifiedNumber( 
                                    atoi( parameter.at(r).at(1).c_str() ) ) ;
            if( parameter.at(r).size() > 2 ) 
                specCol[1] = SpecifiedNumber( 
                                    atoi( parameter.at(r).at(2).c_str() ) ) ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("ROW") == 0  
            &&
            parameter.at(r).size() > 1
        ){
            specRow[0] = SpecifiedNumber( 
                                    atoi( parameter.at(r).at(1).c_str() ) ) ;
            if( parameter.at(r).size() > 2 ) 
                specRow[1] = SpecifiedNumber( 
                                    atoi( parameter.at(r).at(2).c_str() ) ) ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("DIMENSIONS") == 0 
            &&
            parameter.at(r).size() > 1
        ){
            dimensions = atoi( parameter.at(r).at(1).c_str() ) ;
            continue ;
        }
        
        if( 
            parameter.at(r).at(0).compare("BINNING") == 0 
            &&
            parameter.at(r).size() > 3
        ){
            binning = 1 ;
            nbins[0] = atoi( parameter.at(r).at(1).c_str() ) ;
            ranges[0][0] = atof( parameter.at(r).at(2).c_str() ) ;
            ranges[0][1] = atof( parameter.at(r).at(3).c_str() ) ;
            if( parameter.at(r).size() > 6 ){
                binning = 2 ;
                nbins[1] = atoi( parameter.at(r).at(4).c_str() ) ;
                ranges[1][0] = atof( parameter.at(r).at(5).c_str() ) ;
                ranges[1][1] = atof( parameter.at(r).at(6).c_str() ) ;
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
        
        if( parameter.at(r).at(0).compare("FLIP") == 0 ){
            if( parameter.at(r).size() > 2 ){
                if( parameter.at(r).at(1).compare("1") == 0 ) 
                    toFlip[0] = true ;
                if( parameter.at(r).at(2).compare("1") == 0 )
                    toFlip[1] = true ;
            }
            else{
                toFlip[0] = true ;
                toFlip[1] = true ;
            }
            continue ;
        }

        if( parameter.at(r).at(0).compare("WRITEERRORS") == 0 ){
            writeErrors = true ;
            continue ;
        }

        if( parameter.at(r).at(0).compare("SETERRORS") == 0 ){
            setErrors = true ;
            continue ;
        }
        
        if( parameter.at(r).at(0).compare("PARAMETERARGUMENTS") == 0 ){
            parameterArguments = true ;
            continue ;
        }
        
        if( parameter.at(r).size() > 1 )
            dataSpecifier.push_back( parameter.at(r) ) ;

    }
    
    if( dimensions < 1 ){
        cout << " ERROR : specify DIMENSIONS " << endl ;
        return 2 ;
    }
    
    if( writeErrors && setErrors ){
        cout << " ERROR : either write or set errors - not both " << endl ;
        return 2 ;
    }
    
    if( setErrors && parameterArguments ){
        cout << " ERROR : either set errors" ;
        cout << " or use parameter arguments - not both " << endl ;
        return 2 ;
    }
    
    if( setErrors && formula != "x" ){
        cout << " ERROR : " ;
        cout << "either set errors or use function - not both " << endl ;
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
    
    unsigned int nData = dataSpecifier.size() ;
    unsigned int nSpecifier ;
    unsigned int count = 0 ;
    TString name , title , word ;
    vector< vector<string> > textData ;
    unsigned int nrows , ncols , nlines ;
    int binOffsets[2] ;
    unsigned int linesTOuse[2] , lineTOskip ;
    double value , quantity ;
    unsigned int lineDirection ;
    
    TFile * infile ;
    TH1D ** hists1D = new TH1D*[nData] ;
    TH2D ** hists2D = new TH2D*[nData] ;
    for(unsigned int d=0; d<nData; d++){
        hists1D[d] = NULL ;
        hists2D[d] = NULL ;
    }
    
    for(unsigned int d=0; d<nData; d++){
        
        name =  preNsuffix[0][0] ;
        if( dataSpecifier.at(d).at(1) != "%" ) 
            name += dataSpecifier.at(d).at(1) ;
        name += preNsuffix[0][1] ;
        
        cout << " " << name << endl ;
        
        nSpecifier = dataSpecifier.size() ;
        
        if( name.EndsWith(".root") && nSpecifier > 2 ){
            
            infile = new TFile( name , "READ" ) ;
            if( infile->IsZombie() ){
                cout << " ERROR : opening " << name << endl ;
                infile->Close() ;
                continue ;
            }
            name  = preNsuffix[1][0] ;
            if( dataSpecifier.at(d).at(2) != "%" ) 
                name += dataSpecifier.at(d).at(2) ;
            name += preNsuffix[1][1] ;
            if( infile->Get(name) == NULL ){
                cout << " ERROR : reading " << name 
                     << " in " << infile->GetName() << endl ;
                infile->Close() ;
                continue ;
            }
            title = infile->Get(name)->ClassName() ;
            if( !( title.BeginsWith("TH") ) ){
                cout << " ERROR : wrong class for " << name 
                     << " in " << infile->GetName() << endl ;
                infile->Close() ;
                continue ;
            }
            if( atoi( title(2,1).Data() ) != dimensions ){
                cout << " ERROR : dimensions not fitting for " 
                     << name << " in " << infile->GetName() << endl ;
                infile->Close() ;
                continue ;
            }
            
            if( dimensions == 1 ){
                hists1D[d] = (TH1D*)infile->Get(name) ;
                hists1D[d]->SetDirectory(0) ;
                hists1D[d]->SetName(  dataSpecifier.at(d).at(0).c_str() ) ;
                hists1D[d]->SetTitle( dataSpecifier.at(d).at(0).c_str() ) ;
            }
            else if( dimensions == 2 ){
                hists2D[d] = (TH2D*)infile->Get(name) ;
                hists2D[d]->SetDirectory(0) ;
                hists2D[d]->SetName(  dataSpecifier.at(d).at(0).c_str() ) ;
                hists2D[d]->SetTitle( dataSpecifier.at(d).at(0).c_str() ) ;
            }
            infile->Close() ;
            
        }
        else{
            
            textData = getInput( name.Data() ) ;
            nrows = textData.size() ;
            if( nrows < 1 ){
                cout << " ERROR : no data in " << name << endl ;
                continue ;
            }
            
            if( dimensions == 1 ){
                
                if( binning > 0 || useWeights ){
                    lineDirection = 0 ;
                    if( binning < 1 && useWeights ){
                        nbins[0] = textData.size() ;
                        nbins[1] = textData.at(0).size() ;
                        if( specRow[0].setting && !( specRow[0].number < 0 ) ){
                            nbins[1] = textData.at(specRow[0].number).size() ;
                            lineDirection = 1 ;
                        }
                        else if( nbins[1] > nbins[0] ) lineDirection = 1 ;
                        if( specRow[0].setting && specRow[0].number < 0 ) 
                            nbins[0]-- ;
                        if( specCol[0].setting && specCol[0].number < 0 )
                            nbins[1]-- ;
                        if( useOutflow ){
                            if( nbins[lineDirection] > 2 ) 
                                nbins[lineDirection] -= 2 ;
                        }
                        if( nbins[lineDirection] < 1 ){
                            cout << " ERROR : specifications"
                                 << " not matching data " << endl ;
                            continue ;
                        }
                        ranges[0][0] = -0.5 ;
                        ranges[0][1] = nbins[lineDirection]-0.5 ;
                    }
                    hists1D[d] = new TH1D( 
                                            dataSpecifier.at(d).at(0).c_str() ,
                                            dataSpecifier.at(d).at(0).c_str() , 
                                            nbins[lineDirection] , 
                                            ranges[0][0] , 
                                            ranges[0][1] 
                                        ) ;
                }
                else{
                    cout << " ERROR : either provide BINNING " 
                         << " or not specify VALUES " << endl ;
                    continue ; 
                }
                
                if( useWeights ){
                    
                    binOffsets[0] = 1 ;
                    if( useOutflow ) binOffsets[0] = 0 ;
                    
                    linesTOuse[0] = 0 ;
                    linesTOuse[1] = 0 ;
                    if( lineTOexclude( linesTOuse[0] , specRow[0] ) ) 
                        linesTOuse[0]++ ;
                    if( lineTOexclude( linesTOuse[1] , specCol[0] ) ) 
                        linesTOuse[1]++ ;
                    
                    if( useLine( linesTOuse[1] , specCol[0] ) ){
                        ncols = textData.at(linesTOuse[0]).size() ;
                        if( specCol[0].number >= ncols ){
                            cout << " ERROR : specified column (" 
                                 << specCol[0].number 
                                 << ") not available " << endl ;
                            return 2 ;
                        }
                        for(unsigned int r=0; r<nrows; r++){
                            if( lineTOexclude( r , specRow[0] )  ){
                                binOffsets[0]-- ;
                                continue ;
                            }
                            if( linesTOuse[1] >= textData.at(r).size() )
                                continue ;
                            title = textData.at(r).at(linesTOuse[1]) ;
                            if( title.IsFloat() )
                                hists1D[d]->SetBinContent( 
                                    r+binOffsets[0] , atof( title.Data() ) 
                                ) ;
                            if(
                                specCol[1].setting 
                                && 
                                !( specCol[1].number < 0 )
                                &&
                                specCol[1].number < textData.at(r).size() 
                            ){
                                title = textData.at(r).at(specCol[1].number) ;
                                if( title.IsFloat() )
                                    hists1D[d]->SetBinError( 
                                        r+binOffsets[0] , atof( title.Data() ) 
                                    ) ;
                            }
                        }
                    }
                    else if( useLine( linesTOuse[0] , specRow[0] ) ){
                        if( specRow[0].number >= nrows ){
                            cout << " ERROR : specified row (" 
                                 << specRow[0].number 
                                 << ") not available " << endl ;
                            return 3 ;
                        }
                        ncols = textData.at( linesTOuse[0] ).size() ;
                        for(unsigned int c=0; c<ncols; c++){
                            if( lineTOexclude( c , specCol[0] ) ){ 
                                binOffsets[0]-- ;
                                continue ;
                            }
                            title = textData.at( linesTOuse[0] ).at(c) ;
                            if( title.IsFloat() )
                                hists1D[d]->SetBinContent( 
                                    c+binOffsets[0] , atof( title.Data() ) 
                                ) ;
                            if(
                                specRow[1].setting 
                                && 
                                !( specRow[1].number < 0 )
                                &&
                                !( specRow[1].number >= nrows )
                                &&
                                c < textData.at( specRow[1].number ).size()
                            ){
                                title = textData.at(specRow[1].number).at(c) ;
                                if( title.IsFloat() )
                                    hists1D[d]->SetBinError( 
                                        c+binOffsets[0] , atof( title.Data() ) 
                                    ) ;
                            }
                        }
                    }
                    else if( lineDirection == 1 ){
                        ncols = textData.at(linesTOuse[0]).size() ;
                        for(unsigned int c=0; c<ncols; c++){
                            if( lineTOexclude( c , specCol[0] ) ){ 
                                binOffsets[0]-- ;
                                continue ;
                            }
                            title = textData.at(linesTOuse[0]).at(c) ;
                            if( title.IsFloat() )
                                hists1D[d]->SetBinContent( 
                                    c+binOffsets[0] , atof( title.Data() )
                                ) ;
                        }
                    }
                    else{
                        for(unsigned int r=0; r<nrows; r++){
                            if( lineTOexclude( r , specRow[0] ) ){ 
                                binOffsets[0]-- ;
                                continue ;
                            }
                            title = textData.at(r).at(linesTOuse[1]) ;
                            if( title.IsFloat() )
                                hists1D[d]->SetBinContent( 
                                    r+binOffsets[0] , atof( title.Data() )
                                ) ;
                        }
                    }
                    
                }
                else{
                    for(unsigned int r=0; r<nrows; r++){
                        if( lineTOexclude( r , specRow[0] ) ) continue ;
                        ncols = textData.at(r).size() ;
                        for(unsigned int c=0; c<ncols; c++){
                            if( lineTOexclude( c , specCol[0] ) ) continue ;
                            title = textData.at(r).at(c) ;
                            if( title.IsFloat() ){
                                value = atof( title.Data() ) ;
                                if( 
                                    useOutflow 
                                    ||
                                    (
                                        value >= ranges[0][0]
                                        &&
                                        value <  ranges[0][1]
                                    )
                                )
                                    hists1D[d]->Fill( value ) ;
                            }
                        }
                    }
                }
                
            }
            else if( dimensions == 2 ){
                
                if( binning > 1  || useWeights ){
                    if( binning < 2 && useWeights ){
                        nbins[0] = textData.size() ;
                        nbins[1] = textData.at(0).size() ;
                        for(unsigned int r=0; r<nbins[0]; r++){
                            if( nbins[1] < textData.at(r).size() )
                                nbins[1] = textData.at(r).size() ;
                        }
                        if( specRow[0].setting && specRow[0].number < 0 ) 
                            nbins[1]-- ;
                        if( specCol[0].setting && specCol[0].number < 0 )
                            nbins[0]-- ;
                        if( useOutflow && nbins[0] > 2 && nbins[1] > 2 ){ 
                            nbins[0] -= 2 ;
                            nbins[1] -= 2 ;
                        }
                        if( nbins[0] < 1 || nbins[1] < 1 ){
                            cout << " ERROR : specifications"
                                 << " not matching data " << endl ;
                            continue ;
                        }
                        ranges[0][0] = -0.5 ;
                        ranges[0][1] = nbins[0]-0.5 ;
                        ranges[1][0] = -0.5 ;
                        ranges[1][1] = nbins[1]-0.5 ;
                    }
                    hists2D[d] = new TH2D( 
                                            dataSpecifier.at(d).at(0).c_str() ,
                                            dataSpecifier.at(d).at(0).c_str() , 
                                            nbins[0] , 
                                            ranges[0][0] , 
                                            ranges[0][1] , 
                                            nbins[1] , 
                                            ranges[1][0] , 
                                            ranges[1][1] 
                                        ) ;
                }
                else{
                    cout << " ERROR : either provide BINNING " 
                         << " or not specify VALUES " << endl ;
                    continue ; 
                }
                
                if( useWeights ){
                    
                    binOffsets[0] = 1 ;
                    binOffsets[1] = 1 ;
                    if( useOutflow ){ 
                        binOffsets[0] = 0 ;
                        binOffsets[1] = 0 ;
                    }
                                                 
                    for(unsigned int r=0; r<nrows; r++){
                        if( lineTOexclude( r , specRow[0] ) ){ 
                            binOffsets[0]-- ;
                            continue ;
                        }
                        ncols = textData.at(r).size() ;
                        for(unsigned int c=0; c<ncols; c++){
                            if( lineTOexclude( c , specCol[0] ) ){ 
                                binOffsets[1]-- ;
                                continue ;
                            }
                            title = textData.at(r).at(c) ;
                            if( title.IsFloat() ){
                                value = atof( title.Data() ) ;
                                hists2D[d]->SetBinContent( 
                                                            c+binOffsets[1] ,
                                                            r+binOffsets[0] ,
                                                            value 
                                                         ) ;
                            }
                        }
                    }
                    
                }
                else{
                    
                    linesTOuse[0] = 0 ;
                    linesTOuse[1] = 0 ;
                    
                    ncols = textData.at(0).size() ;
                    for(unsigned int r=0; r<nrows; r++){
                        if( ncols < textData.at(r).size() ){
                            ncols = textData.at(r).size() ;
                            linesTOuse[0] = r ;
                        }
                    }
                    
                    if( 
                        specCol[0].setting && !( specCol[0].number < 0 )
                        &&
                        specCol[1].setting && !( specCol[1].number < 0 )
                        &&
                        specCol[0].number < ncols && specCol[1].number < ncols
                    ){
                        
                        for(unsigned int r=0; r<nrows; r++){
                            if( 
                                lineTOexclude( r , specRow[0] ) 
                                ||
                                textData.at(r).size() <= specCol[0].number
                                ||
                                textData.at(r).size() <= specCol[1].number
                            ) 
                                continue ;
                            title = textData.at(r).at(specCol[0].number) ;
                            word  = textData.at(r).at(specCol[1].number) ;
                            if(
                                !( title.IsFloat() ) 
                                || 
                                !( word.IsFloat() )
                            ) 
                                continue ;
                            value    = atof( title.Data() ) ;
                            quantity = atof(  word.Data() ) ;
                            if(
                                useOutflow
                                ||
                                (
                                    value    >= ranges[0][0]
                                    &&
                                    value    <  ranges[0][1]
                                    &&
                                    quantity >= ranges[1][0]
                                    &&
                                    quantity <  ranges[1][1]
                                )
                            )
                                hists2D[d]->Fill( value , quantity ) ;
                            
                        }
                    
                    }
                    else if( 
                        specRow[0].setting && !( specRow[0].number < 0 )
                        &&
                        specRow[1].setting && !( specRow[1].number < 0 )
                        &&
                        specRow[0].number < nrows && specRow[1].number < nrows
                    ){
                        
                        ncols = textData.at(specRow[0].number).size() ;
                        if( ncols != textData.at(specRow[1].number).size() ){
                            cout << " ERROR : data not fitting" 
                                 << " for correlation " << endl ;
                            continue ;
                        }
                        
                        for(unsigned int c=0; c<ncols; c++){
                            if( 
                                lineTOexclude( c , specCol[0] ) 
                                ||
                                textData.at(specRow[0].number).size() <= c 
                                ||
                                textData.at(specRow[1].number).size() <= c 
                            ) 
                                continue ;
                            title = textData.at(specRow[0].number).at(c) ;
                            word  = textData.at(specRow[1].number).at(c) ;
                            if(
                                !( title.IsFloat() ) 
                                || 
                                !( word.IsFloat() )
                            ) 
                                continue ;
                            value    = atof( title.Data() ) ;
                            quantity = atof(  word.Data() ) ;
                            if(
                                useOutflow
                                ||
                                (
                                    value    >= ranges[0][0]
                                    &&
                                    value    <  ranges[0][1]
                                    &&
                                    quantity >= ranges[1][0]
                                    &&
                                    quantity <  ranges[1][1]
                                )
                            )
                                hists2D[d]->Fill( value , quantity ) ;
                            
                        }
                        
                    }
                    else if( ncols > nrows && nrows > 1 ){
                        
                        linesTOuse[1] = linesTOuse[0] ;
                        for(unsigned int r=0; r<nrows; r++){
                            if( ncols == textData.at(r).size() ){
                                linesTOuse[1] = r ;
                            }
                        }
                        if( linesTOuse[1] == linesTOuse[0] ){
                            cout << " ERROR : specify lines to use " << endl ;
                            continue ;
                        }
                        
                        for(unsigned int c=0; c<ncols; c++){
                            title = textData.at(linesTOuse[0]).at(c) ;
                            word  = textData.at(linesTOuse[1]).at(c) ;
                            if(
                                !( title.IsFloat() ) 
                                || 
                                !( word.IsFloat() )
                            ) 
                                continue ;
                            value    = atof( title.Data() ) ;
                            quantity = atof(  word.Data() ) ;
                            if(
                                useOutflow
                                ||
                                (
                                    value    >= ranges[0][0]
                                    &&
                                    value    <  ranges[0][1]
                                    &&
                                    quantity >= ranges[1][0]
                                    &&
                                    quantity <  ranges[1][1]
                                )
                            )
                                hists2D[d]->Fill( value , quantity ) ;
                        }
                        
                    }
                    else if( ncols > 1 ){
                        
                        for(unsigned int r=0; r<nrows; r++){
                            if( textData.at(r).size() < 2 ) continue ;
                            title = textData.at(r).at(0) ;
                            word  = textData.at(r).at(1) ;
                            if(
                                !( title.IsFloat() ) 
                                || 
                                !( word.IsFloat() )
                            ) 
                                continue ;
                            value    = atof( title.Data() ) ;
                            quantity = atof(  word.Data() ) ;
                            if(
                                useOutflow
                                ||
                                (
                                    value    >= ranges[0][0]
                                    &&
                                    value    <  ranges[0][1]
                                    &&
                                    quantity >= ranges[1][0]
                                    &&
                                    quantity <  ranges[1][1]
                                )
                            )
                                hists2D[d]->Fill( value , quantity ) ;
                        }
                        
                    }
                    else{
                        cout << " ERROR : data in " << name 
                             << " not suitable for correlation " << endl ;
                        continue ;
                    }
                    
                }
                
            }
            
            
        }
        
        count++ ;
        
    }
    
    if( count != nData ){
        cout << " ERROR : not all input data well specified " << endl ;
        return 4 ;
    }
    
    if( toFlip[0] || toFlip[1] ){
        
        for(unsigned int d=0; d<nData; d++){
            if( dimensions == 1 ){
                nbins[0] = hists1D[d]->GetXaxis()->GetNbins() + 2 ;
                for(unsigned int b=0; b<nbins[0]; b++){
                    binOffsets[0] = nbins[0] - b - 1 ;
                    if( binOffsets[0] <= b ) break ;
                    value    = hists1D[d]->GetBinContent( b             ) ;
                    quantity = hists1D[d]->GetBinContent( binOffsets[0] ) ;
                    hists1D[d]->SetBinContent( b             , quantity ) ;
                    hists1D[d]->SetBinContent( binOffsets[0] , value    ) ;
                }
            }
            else if( dimensions == 2 ){
                nbins[0] = hists2D[d]->GetXaxis()->GetNbins() + 2 ;
                nbins[1] = hists2D[d]->GetYaxis()->GetNbins() + 2 ;
                if( toFlip[0] ){
                    for(unsigned int c=0; c<nbins[0]; c++){
                        binOffsets[0] = nbins[0] - c - 1 ;
                        if( binOffsets[0] <= c ) break ;
                        for(unsigned int r=0; r<nbins[1]; r++){
                            value    = hists2D[d]->GetBinContent( c , r ) ;
                            quantity = hists2D[d]->GetBinContent( 
                                                            binOffsets[0] , r 
                                                        ) ;
                            hists2D[d]->SetBinContent( c , r , quantity ) ;
                            hists2D[d]->SetBinContent( 
                                                    binOffsets[0] , r , value 
                                                ) ;
                        }
                    }
                }
                if( toFlip[1] ){
                    for(unsigned int r=0; r<nbins[1]; r++){
                        binOffsets[1] = nbins[1] - r - 1 ;
                        if( binOffsets[1] <= r ) break ;
                        for(unsigned int c=0; c<nbins[0]; c++){
                            value    = hists2D[d]->GetBinContent( c , r ) ;
                            quantity = hists2D[d]->GetBinContent( 
                                                            c , binOffsets[1]
                                                        ) ;
                            hists2D[d]->SetBinContent( c , r , quantity ) ;
                            hists2D[d]->SetBinContent( 
                                                    c , binOffsets[1] , value 
                                                ) ;
                        }
                    }
                }
            }
            
        }
        
    }
    
    bool allCompliant = true ;
    
    if( dimensions == 1 ){
        nbins[0]     = hists1D[0]->GetXaxis()->GetNbins() ;
        ranges[0][0] = hists1D[0]->GetXaxis()->GetXmin()  ;
        ranges[0][1] = hists1D[0]->GetXaxis()->GetXmax()  ;
    }
    else if( dimensions == 2 ){
        nbins[0]     = hists2D[0]->GetXaxis()->GetNbins() ;
        ranges[0][0] = hists2D[0]->GetXaxis()->GetXmin()  ;
        ranges[0][1] = hists2D[0]->GetXaxis()->GetXmax()  ;
        nbins[1]     = hists2D[0]->GetYaxis()->GetNbins() ;
        ranges[1][0] = hists2D[0]->GetYaxis()->GetXmin()  ;
        ranges[1][1] = hists2D[0]->GetYaxis()->GetXmax()  ;
    }
    
    for(unsigned int d=1; d<nData; d++){
        if(
            (
                dimensions == 1 
                &&
                (
                    nbins[0]     != hists1D[d]->GetXaxis()->GetNbins() 
                    ||
                    ranges[0][0] != hists1D[d]->GetXaxis()->GetXmin()  
                    ||
                    ranges[0][1] != hists1D[d]->GetXaxis()->GetXmax()  
                )
            )
            ||
            (
                dimensions == 2 
                &&
                (
                    nbins[0]     != hists2D[d]->GetXaxis()->GetNbins() 
                    ||
                    ranges[0][0] != hists2D[d]->GetXaxis()->GetXmin()  
                    ||
                    ranges[0][1] != hists2D[d]->GetXaxis()->GetXmax()  
                    ||
                    nbins[1]     != hists2D[d]->GetYaxis()->GetNbins() 
                    ||
                    ranges[1][0] != hists2D[d]->GetYaxis()->GetXmin()  
                    ||
                    ranges[1][1] != hists2D[d]->GetYaxis()->GetXmax()  
                )
            )
        ){ 
            allCompliant = false ;
            break ;
        }
    }
    
    TF1 * function = new TF1( "function" , formula.c_str() ) ; 
    if( setErrors )
        function = new TF1( "function" , "x+0.*[0]" ) ;
    unsigned int binRange[2][2] , number ;
    double mean , stdv , min , max , median ;
    TH1D * result1D = NULL , * reference1D = NULL ;
    TH2D * result2D = NULL , * reference2D = NULL ;
    
    if( writeErrors ){
        if( dimensions == 1 ){
            for(unsigned int d=0; d<nData; d++){
                hists1D[d]->SetName( "old") ;
                hists1D[d]->SetTitle("old") ;
                result1D = new TH1D(
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        hists1D[d]->GetXaxis()->GetNbins() ,
                                        hists1D[d]->GetXaxis()->GetXmin()  ,
                                        hists1D[d]->GetXaxis()->GetXmax() 
                                    ) ;
                nbins[0] = hists1D[d]->GetNbinsX() ;
                binRange[0][0] = 1 ;
                binRange[0][1] = nbins[0] + 1 ;
                if( useOutflow ){ 
                    binRange[0][0] = 0 ;
                    binRange[0][1] = nbins[0] + 2 ;
                }
                for(unsigned int b=binRange[0][0]; b<binRange[0][1]; b++){
                    result1D->SetBinContent( b , hists1D[d]->GetBinError(b) ) ;
                }
                hists1D[d]->Delete() ;
                hists1D[d] = result1D ;
                result1D = NULL ;
            }
        }
        else if( dimensions == 2 ){
            for(unsigned int d=0; d<nData; d++){
                hists2D[d]->SetName( "old") ;
                hists2D[d]->SetTitle("old") ;
                result2D = new TH2D(
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        hists2D[d]->GetXaxis()->GetNbins() ,
                                        hists2D[d]->GetXaxis()->GetXmin()  ,
                                        hists2D[d]->GetXaxis()->GetXmax()  ,
                                        hists2D[d]->GetYaxis()->GetNbins() ,
                                        hists2D[d]->GetYaxis()->GetXmin()  ,
                                        hists2D[d]->GetYaxis()->GetXmax() 
                                    ) ;
                nbins[0] = hists2D[d]->GetNbinsX() ;
                binRange[0][0] = 1 ;
                binRange[0][1] = nbins[0] + 1 ;
                nbins[1] = hists2D[d]->GetNbinsY() ;
                binRange[1][0] = 1 ;
                binRange[1][1] = nbins[1] + 1 ;
                if( useOutflow ){ 
                    binRange[0][0] = 0 ;
                    binRange[0][1] = nbins[0] + 2 ;
                    binRange[1][0] = 0 ;
                    binRange[1][1] = nbins[1] + 2 ;
                }
                for(unsigned int r=binRange[1][0]; r<binRange[1][1]; r++){
                    for(unsigned int c=binRange[0][0]; c<binRange[0][1]; c++){
                        result2D->SetBinContent( 
                            c , r , hists2D[d]->GetBinError(c,r) 
                        ) ;
                    }
                }
                hists2D[d]->Delete() ;
                hists2D[d] = result2D ;
                result2D = NULL ;
            }
            
        }
    }
    else if( parameterArguments && function->GetNpar() > 0 ){
        if( dimensions == 1 ){
            for(unsigned int d=0; d<nData; d++){
                if( dataSpecifier.at(d).size() < function->GetNpar() + 3 ){
                    cout << " ERROR : not enough parameters provided for "
                         << dataSpecifier.at(d).at(0) << endl ;
                    continue ;
                }
                getStats( 
                    (TH2D*)hists1D[d] , mean , stdv , min , max , median , number 
                ) ;
                for(unsigned int a=3; a<dataSpecifier.at(d).size(); a++){
                    if(      dataSpecifier.at(d).at(a) == "MEAN"   )
                        function->SetParameter( a-3 , mean ) ;
                    else if( dataSpecifier.at(d).at(a) == "STDV"   )
                        function->SetParameter( a-3 , stdv ) ;
                    else if( dataSpecifier.at(d).at(a) == "MIN"    )
                        function->SetParameter( a-3 , min ) ;
                    else if( dataSpecifier.at(d).at(a) == "MAX"    )
                        function->SetParameter( a-3 , max ) ;
                    else if( dataSpecifier.at(d).at(a) == "MEDIAN" )
                        function->SetParameter( a-3 , median ) ;
                    else
                        function->SetParameter( 
                            a-3 , atof( dataSpecifier.at(d).at(a).c_str() ) 
                        ) ;
                }
                hists1D[d]->SetName( "old") ;
                hists1D[d]->SetTitle("old") ;
                result1D = new TH1D(
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        hists1D[d]->GetXaxis()->GetNbins() ,
                                        hists1D[d]->GetXaxis()->GetXmin()  ,
                                        hists1D[d]->GetXaxis()->GetXmax() 
                                    ) ;
                nbins[0] = hists1D[d]->GetNbinsX() ;
                binRange[0][0] = 1 ;
                binRange[0][1] = nbins[0] + 1 ;
                if( useOutflow ){ 
                    binRange[0][0] = 0 ;
                    binRange[0][1] = nbins[0] + 2 ;
                }
                for(unsigned int b=binRange[0][0]; b<binRange[0][1]; b++){
                    value = function->Eval( 
                                    (double)hists1D[d]->GetBinContent( b ) 
                                ) ;
                    if( toDiscard( value ) ) continue ;
                    result1D->SetBinContent( b ,  value ) ;
                }
                hists1D[d]->Delete() ;
                hists1D[d] = result1D ;
                result1D = NULL ;
            }
        }
        else if( dimensions == 2 ){
            for(unsigned int d=0; d<nData; d++){
                if( dataSpecifier.at(d).size() < function->GetNpar() + 3 ){
                    cout << " ERROR : not enough parameters provided for "
                         << dataSpecifier.at(d).at(0) << endl ;
                    continue ;
                }
                getStats( 
                    hists2D[d] , mean , stdv , min , max , median , number 
                ) ;
                for(unsigned int a=3; a<dataSpecifier.at(d).size(); a++){
                    if(      dataSpecifier.at(d).at(a) == "MEAN"   )
                        function->SetParameter( a-3 , mean ) ;
                    else if( dataSpecifier.at(d).at(a) == "STDV"   )
                        function->SetParameter( a-3 , stdv ) ;
                    else if( dataSpecifier.at(d).at(a) == "MIN"    )
                        function->SetParameter( a-3 , min ) ;
                    else if( dataSpecifier.at(d).at(a) == "MAX"    )
                        function->SetParameter( a-3 , max ) ;
                    else if( dataSpecifier.at(d).at(a) == "MEDIAN" )
                        function->SetParameter( a-3 , median ) ;
                    else
                        function->SetParameter( 
                            a-3 , atof( dataSpecifier.at(d).at(a).c_str() ) 
                        ) ;
                }
                hists2D[d]->SetName( "old") ;
                hists2D[d]->SetTitle("old") ;
                result2D = new TH2D(
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        hists2D[d]->GetXaxis()->GetNbins() ,
                                        hists2D[d]->GetXaxis()->GetXmin()  ,
                                        hists2D[d]->GetXaxis()->GetXmax()  ,
                                        hists2D[d]->GetYaxis()->GetNbins() ,
                                        hists2D[d]->GetYaxis()->GetXmin()  ,
                                        hists2D[d]->GetYaxis()->GetXmax() 
                                    ) ;
                nbins[0] = hists2D[d]->GetNbinsX() ;
                binRange[0][0] = 1 ;
                binRange[0][1] = nbins[0] + 1 ;
                nbins[1] = hists2D[d]->GetNbinsY() ;
                binRange[1][0] = 1 ;
                binRange[1][1] = nbins[1] + 1 ;
                if( useOutflow ){ 
                    binRange[0][0] = 0 ;
                    binRange[0][1] = nbins[0] + 2 ;
                    binRange[1][0] = 0 ;
                    binRange[1][1] = nbins[1] + 2 ;
                }
                for(unsigned int r=binRange[1][0]; r<binRange[1][1]; r++){
                    for(unsigned int c=binRange[0][0]; c<binRange[0][1]; c++){
                        value = function->Eval( 
                                    (double)hists2D[d]->GetBinContent( c , r ) 
                                ) ;
                        if( toDiscard( value ) ) continue ;
                        result2D->SetBinContent( c , r ,  value ) ;
                    }
                }
                hists2D[d]->Delete() ;
                hists2D[d] = result2D ;
                result2D = NULL ;
            }
        }
    }
    else if( 
        nData > 1 
        && 
        allCompliant 
        && 
        nData == function->GetNpar() + 1 
        &&
        !( setErrors )
    ){
        if( dimensions == 1 ){
            result1D = new TH1D( 
                                    "result" , "result" , 
                                    nbins[0] , ranges[0][0] , ranges[0][1] 
                               );
            binRange[0][0] = 1 ;
            binRange[0][1] = nbins[0] + 1 ;
            if( useOutflow ){ 
                binRange[0][0] = 0 ;
                binRange[0][1] = nbins[0] + 2 ;
            }
            for(unsigned int b=binRange[0][0]; b<binRange[0][1]; b++){
                for(unsigned int d=1; d<nData; d++){
                    function->SetParameter( 
                        d-1 , (double)hists1D[d]->GetBinContent( b ) 
                    ) ;
                }
                value = function->Eval( 
                                (double)hists1D[0]->GetBinContent( b ) 
                            ) ;
                if( toDiscard( value ) ) continue ;
                result1D->SetBinContent( b , value ) ;
            }
        }
        else if( dimensions == 2 ){
            result2D = new TH2D( 
                                    "result" , "result" , 
                                    nbins[0] , ranges[0][0] , ranges[0][1] , 
                                    nbins[1] , ranges[1][0] , ranges[1][1] 
                               );
            binRange[0][0] = 1 ;
            binRange[0][1] = nbins[0] + 1 ;
            binRange[1][0] = 1 ;
            binRange[1][1] = nbins[1] + 1 ;
            if( useOutflow ){ 
                binRange[0][0] = 0 ;
                binRange[0][1] = nbins[0] + 2 ;
                binRange[1][0] = 0 ;
                binRange[1][1] = nbins[1] + 2 ;
            }
            for(unsigned int r=binRange[1][0]; r<binRange[1][1]; r++){
                for(unsigned int c=binRange[0][0]; c<binRange[0][1]; c++){
                    for(unsigned int d=0; d<nData; d++){
                        function->SetParameter( 
                            d-1 , (double)hists2D[d]->GetBinContent( c , r ) 
                        ) ;
                    }
                    value = function->Eval( 
                                    (double)hists2D[0]->GetBinContent( c , r ) 
                                ) ;
                    if( toDiscard( value ) ) continue ;
                    result2D->SetBinContent( c , r , value ) ;
                }
            }
        }
    }
    else if( formula != "x" && function->GetNpar() == 0 ){
        if( dimensions == 1 ){
            for(unsigned int d=0; d<nData; d++){
                hists1D[d]->SetName( "old") ;
                hists1D[d]->SetTitle("old") ;
                result1D = new TH1D(
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        hists1D[d]->GetXaxis()->GetNbins() ,
                                        hists1D[d]->GetXaxis()->GetXmin()  ,
                                        hists1D[d]->GetXaxis()->GetXmax() 
                                    ) ;
                nbins[0] = hists1D[d]->GetNbinsX() ;
                binRange[0][0] = 1 ;
                binRange[0][1] = nbins[0] + 1 ;
                if( useOutflow ){ 
                    binRange[0][0] = 0 ;
                    binRange[0][1] = nbins[0] + 2 ;
                }
                for(unsigned int b=binRange[0][0]; b<binRange[0][1]; b++){
                    value = function->Eval( 
                                    (double)hists1D[d]->GetBinContent( b ) 
                                ) ;
                    if( toDiscard( value ) ) continue ;
                    result1D->SetBinContent( b ,  value ) ;
                }
                hists1D[d]->Delete() ;
                hists1D[d] = result1D ;
                result1D = NULL ;
            }
        }
        else if( dimensions == 2 ){
            for(unsigned int d=0; d<nData; d++){
                hists2D[d]->SetName( "old") ;
                hists2D[d]->SetTitle("old") ;
                result2D = new TH2D(
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        hists2D[d]->GetXaxis()->GetNbins() ,
                                        hists2D[d]->GetXaxis()->GetXmin()  ,
                                        hists2D[d]->GetXaxis()->GetXmax()  ,
                                        hists2D[d]->GetYaxis()->GetNbins() ,
                                        hists2D[d]->GetYaxis()->GetXmin()  ,
                                        hists2D[d]->GetYaxis()->GetXmax() 
                                    ) ;
                nbins[0] = hists2D[d]->GetNbinsX() ;
                binRange[0][0] = 1 ;
                binRange[0][1] = nbins[0] + 1 ;
                nbins[1] = hists2D[d]->GetNbinsY() ;
                binRange[1][0] = 1 ;
                binRange[1][1] = nbins[1] + 1 ;
                if( useOutflow ){ 
                    binRange[0][0] = 0 ;
                    binRange[0][1] = nbins[0] + 2 ;
                    binRange[1][0] = 0 ;
                    binRange[1][1] = nbins[1] + 2 ;
                }
                for(unsigned int r=binRange[1][0]; r<binRange[1][1]; r++){
                    for(unsigned int c=binRange[0][0]; c<binRange[0][1]; c++){
                        value = function->Eval( 
                                    (double)hists2D[d]->GetBinContent( c , r ) 
                                ) ;
                        if( toDiscard( value ) ) continue ;
                        result2D->SetBinContent( c , r ,  value ) ;
                    }
                }
                hists2D[d]->Delete() ;
                hists2D[d] = result2D ;
                result2D = NULL ;
            }
        }
    }
    else if( 
        function->GetNpar() == 1  
        && 
        TString( preNsuffix[0][1] ).EndsWith( ".root" ) 
    ){
        for(unsigned int d=0; d<nData; d++){
            if(  dataSpecifier.at(d).size() < 5  ) continue ;
            name =  preNsuffix[0][0] ;
            if( dataSpecifier.at(d).at(3) != "%" ) 
                name += dataSpecifier.at(d).at(3) ;
            name += preNsuffix[0][1] ;
            infile = new TFile( name , "READ" ) ;
            if( infile->IsZombie() ){
                cout << " ERROR : opening " << name << endl ;
                infile->Close() ;
                continue ;
            }
            name  = preNsuffix[1][0] ;
            if( dataSpecifier.at(d).at(4) != "%" ) 
                name += dataSpecifier.at(d).at(4) ;
            name += preNsuffix[1][1] ;
            if( infile->Get(name) == NULL ){
                cout << " ERROR : reading " << name 
                     << " in " << infile->GetName() << endl ;
                infile->Close() ;
                continue ;
            }
            title = infile->Get(name)->ClassName() ;
            if( !( title.BeginsWith("TH") ) ){
                cout << " ERROR : wrong class for " << name 
                     << " in " << infile->GetName() << endl ;
                infile->Close() ;
                continue ;
            }
            if( atoi( title(2,1).Data() ) != dimensions ){
                cout << " ERROR : dimensions not fitting for " 
                     << name << " in " << infile->GetName() << endl ;
                infile->Close() ;
                continue ;
            }
            if( dimensions == 1 ){
                reference1D = (TH1D*)infile->Get(name) ;
                if(  
                    reference1D->GetXaxis()->GetNbins() 
                    != hists1D[d]->GetXaxis()->GetNbins()
                    ||
                    reference1D->GetXaxis()->GetXmin() 
                    != hists1D[d]->GetXaxis()->GetXmin()
                    ||
                    reference1D->GetXaxis()->GetXmax() 
                    != hists1D[d]->GetXaxis()->GetXmax()
                ){
                    cout << " ERROR : wrong binning for " 
                         << name << " in " << infile->GetName() << endl ;
                    infile->Close() ;
                    reference1D = NULL ;
                    continue ;
                }
                reference1D->SetDirectory(0) ;
                name = dataSpecifier.at(d).at(0).c_str() ;
                name += "_ref" ;
                reference1D->SetName(  name ) ;
                reference1D->SetTitle( name ) ;
            }
            else if( dimensions == 2 ){
                reference2D = (TH2D*)infile->Get(name) ;
                if(  
                    reference2D->GetXaxis()->GetNbins() 
                    != hists2D[d]->GetXaxis()->GetNbins()
                    ||
                    reference2D->GetXaxis()->GetXmin() 
                    != hists2D[d]->GetXaxis()->GetXmin()
                    ||
                    reference2D->GetXaxis()->GetXmax() 
                    != hists2D[d]->GetXaxis()->GetXmax()
                    ||
                    reference2D->GetYaxis()->GetNbins() 
                    != hists2D[d]->GetYaxis()->GetNbins()
                    ||
                    reference2D->GetYaxis()->GetXmin() 
                    != hists2D[d]->GetYaxis()->GetXmin()
                    ||
                    reference2D->GetYaxis()->GetXmax() 
                    != hists2D[d]->GetYaxis()->GetXmax()
                ){
                    cout << " ERROR : wrong binning for " 
                         << name << " in " << infile->GetName() << endl ;
                    infile->Close() ;
                    reference2D = NULL ;
                    continue ;
                }
                reference2D->SetDirectory(0) ;
                name = dataSpecifier.at(d).at(0).c_str() ;
                name += "_ref" ;
                reference2D->SetName(  name ) ;
                reference2D->SetTitle( name ) ;
            }
            infile->Close() ;
            if( dimensions == 1 ){
                hists1D[d]->SetName( "old") ;
                hists1D[d]->SetTitle("old") ;
                result1D = new TH1D(
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        hists1D[d]->GetXaxis()->GetNbins() ,
                                        hists1D[d]->GetXaxis()->GetXmin()  ,
                                        hists1D[d]->GetXaxis()->GetXmax() 
                                    ) ;
                nbins[0] = hists1D[d]->GetNbinsX() ;
                binRange[0][0] = 1 ;
                binRange[0][1] = nbins[0] + 1 ;
                if( useOutflow ){ 
                    binRange[0][0] = 0 ;
                    binRange[0][1] = nbins[0] + 2 ;
                }
                for(unsigned int b=binRange[0][0]; b<binRange[0][1]; b++){
                    function->SetParameter( 
                                    0 , (double)reference1D->GetBinContent( b ) 
                                ) ;
                    value = function->Eval( 
                                    (double)hists1D[d]->GetBinContent( b ) 
                                ) ;
                    if( toDiscard( value ) ) continue ;
                    result1D->SetBinContent( b ,  value ) ;
                    if( setErrors )
                        result1D->SetBinError( 
                            b , function->GetParameter(0) 
                        ) ;
                }
                reference1D->Delete() ;
                reference1D = NULL ;
                hists1D[d]->Delete() ;
                hists1D[d] = result1D ;
                result1D = NULL ;
            }
            else if( dimensions == 2 ){
                hists2D[d]->SetName( "old") ;
                hists2D[d]->SetTitle("old") ;
                result2D = new TH2D(
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        dataSpecifier.at(d).at(0).c_str()  ,
                                        hists2D[d]->GetXaxis()->GetNbins() ,
                                        hists2D[d]->GetXaxis()->GetXmin()  ,
                                        hists2D[d]->GetXaxis()->GetXmax()  ,
                                        hists2D[d]->GetYaxis()->GetNbins() ,
                                        hists2D[d]->GetYaxis()->GetXmin()  ,
                                        hists2D[d]->GetYaxis()->GetXmax() 
                                    ) ;
                nbins[0] = hists2D[d]->GetNbinsX() ;
                binRange[0][0] = 1 ;
                binRange[0][1] = nbins[0] + 1 ;
                nbins[1] = hists2D[d]->GetNbinsY() ;
                binRange[1][0] = 1 ;
                binRange[1][1] = nbins[1] + 1 ;
                if( useOutflow ){ 
                    binRange[0][0] = 0 ;
                    binRange[0][1] = nbins[0] + 2 ;
                    binRange[1][0] = 0 ;
                    binRange[1][1] = nbins[1] + 2 ;
                }
                for(unsigned int r=binRange[1][0]; r<binRange[1][1]; r++){
                    for(unsigned int c=binRange[0][0]; c<binRange[0][1]; c++){
                        function->SetParameter( 
                                0 , (double)reference2D->GetBinContent( c , r ) 
                            ) ;
                        value = function->Eval( 
                                    (double)hists2D[d]->GetBinContent( c , r ) 
                                ) ;
                        if( toDiscard( value ) ) continue ;
                        result2D->SetBinContent( c , r ,  value ) ;
                        if( setErrors )
                            result2D->SetBinError( 
                                c , r , function->GetParameter(0) 
                            ) ;
                    }
                }
                reference2D->Delete() ;
                reference2D = NULL ;
                hists2D[d]->Delete() ;
                hists2D[d] = result2D ;
                result2D = NULL ;
            }
            
        }
    }
    
    cout << " writing ... " ;

    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
    outfile->cd() ;
    
    if( result1D != NULL ) result1D->Write() ;
    if( result2D != NULL ) result2D->Write() ;
    
    for(unsigned int d=0; d<nData; d++){
        if( hists1D[d] != NULL ) hists1D[d]->Write() ;
        if( hists2D[d] != NULL ) hists2D[d]->Write() ;
    }
    
    outfile->Write() ;
    outfile->Close() ;
    
    cout << " done " << endl ;
    
    return 0 ;

}