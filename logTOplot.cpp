#include "postprocessing.h"

using namespace std;

class Quantity{
public :

    string identifier ;
    string name ;
    string unit ;
    SpecifiedNumber range[2] ;
    bool logarithmic ;
    unsigned int nDivisions ;

    Quantity(){
        identifier = "" ;
        name = "" ;
        unit = "" ;
        range[0] = SpecifiedNumber() ;
        range[1] = SpecifiedNumber() ;
        logarithmic = false ;
        nDivisions = 515 ;
    }

    Quantity( string i ){
        *this = Quantity() ;
        this->identifier = i ;
    }

} ;

int main(int argc, char *argv[]){
    
    if( argc < 2 ){
        printUsage( argv[0] ) ;
        return 1 ;
    }
 
    plotOptions() ;
    
    gStyle->SetOptStat(0) ;

    gStyle->SetPadTopMargin(    0.022 ) ;
    gStyle->SetPadRightMargin(  0.085 ) ;
    gStyle->SetPadBottomMargin( 0.020 ) ;
    gStyle->SetPadLeftMargin(   0.080 ) ;

    gStyle->SetTitleOffset( 1.1 , "x" ) ;
    gStyle->SetTitleOffset( 0.9 , "y" ) ;

    TString filename = argv[1] ;
    
    bool rootData = false ;
    if( filename.EndsWith(".root") ) rootData = true ;
    
    SpecifiedNumber plotTime , duration ;
    bool show  = true ;
    bool print = true ;
    bool write = true ;
    TString saveAS = "pdf" ;
    
    if( argc > 3 ){
        for(unsigned int a=2; a<4; a++){
            string tester = argv[a] ;
            if( tester == "%" ) continue ;
            string timeUnit ;
            SpecifiedNumber timeInput 
                = SpecifiedNumber( getNumberWithUnit( tester , timeUnit ) ) ;
            timeInput.specifier = timeUnit ;
            if( a == 2 ) plotTime = timeInput ;
            else if ( a == 3 ) duration = timeInput ;
        }
    }

    TString parameterFileName = "" ;
    bool specificSpecifier = false ;
    vector< vector<string> > specifiersNquantities ;
    vector< vector<SpecifiedNumber> > markerNcolorNline ;
    vector< string > labels ;
    vector<SpecifiedNumber> specVecDummy ;
    unsigned int nSpecific = 0 ;
    vector<string> quantities ;
    unsigned int nQuant = 0 ;

    vector<Quantity> rangedQuantityPads ;
    unsigned int nDefinedPads = 0 ;
    vector<unsigned int> quantityPadMap ;
    vector<unsigned int> specifierPadMap ;

    bool tableData = false ;
    vector<SpecifiedNumber> specifierColumns ;
    SpecifiedNumber delimiter ;
    delimiter.specifier = " " ;
    unsigned int unixColumn = 0 ;
    unsigned int maxColumns = 1 ;
    
    bool abscissaInteger = true ;
    TString xAxisTitle = "" ;
    SpecifiedNumber xLabelFormat ;
    unsigned int nXdivisions = 525 ;
    
    if( argc > 4 && string( argv[4] ) != "%" ){
        parameterFileName = argv[4] ;
        vector< vector<string> > specifierInput
                                    = getInput( parameterFileName.Data() ) ;
        specificSpecifier = true ;
        for(unsigned int s=0; s<specifierInput.size(); s++){
            if( specifierInput.at(s).at(0).rfind("#",0) == 0 )
                continue ;
            if(
                specifierInput.at(s).at(0).compare("QUANTITY") == 0
                &&
                specifierInput.at(s).size() > 1
            ){
                rangedQuantityPads.push_back(
                    Quantity( specifierInput.at(s).at(1)  )
                ) ;
                nDefinedPads = rangedQuantityPads.size() ;
                if(
                    specifierInput.at(s).size() > 2
                    &&
                    specifierInput.at(s).at(2) != "%"
                )
                    rangedQuantityPads.at(nDefinedPads-1).name =
                        specifierInput.at(s).at(2) ;
                if(
                    specifierInput.at(s).size() > 3
                    &&
                    specifierInput.at(s).at(3) != "%"
                )
                    rangedQuantityPads.at(nDefinedPads-1).unit =
                        specifierInput.at(s).at(3) ;
                if(
                    specifierInput.at(s).size() > 4
                    &&
                    specifierInput.at(s).at(4) != "%"
                )
                    rangedQuantityPads.at(nDefinedPads-1).range[0] =
                        SpecifiedNumber(
                            atof( specifierInput.at(s).at(4).c_str() )
                        ) ;
                if(
                    specifierInput.at(s).size() > 5
                    &&
                    specifierInput.at(s).at(5) != "%"
                )
                    rangedQuantityPads.at(nDefinedPads-1).range[1] =
                        SpecifiedNumber(
                            atof( specifierInput.at(s).at(5).c_str() )
                        ) ;
                if(
                    specifierInput.at(s).size() > 6
                    &&
                    (
                        specifierInput.at(s).at(6) == "1"
                        ||
                        specifierInput.at(s).at(6) == "log"
                    )
                )
                    rangedQuantityPads.at(nDefinedPads-1).logarithmic = true ;
                if(
                    specifierInput.at(s).size() > 7
                    &&
                    specifierInput.at(s).at(7) != "%"
                )
                    rangedQuantityPads.at(nDefinedPads-1).nDivisions =
                        atoi( specifierInput.at(s).at(7).c_str() ) ;
                continue ;
            }
            if( specifierInput.at(s).at(0).compare("TABLE") == 0 ){
                tableData = true ;
                if( 
                    specifierInput.at(s).size() > 1
                    &&
                    ( (TString)( specifierInput.at(s).at(1) ) ).IsDigit()
                )
                    unixColumn = atoi( specifierInput.at(s).at(1).c_str() ) ;
                continue ;
            }
            if( 
                specifierInput.at(s).at(0).compare("DELIMITER") == 0 
                &&
                specifierInput.at(s).size() > 1
            ){
                delimiter = 
                    SpecifiedNumber( specifierInput.at(s).at(1).length() ) ;
                delimiter.specifier = specifierInput.at(s).at(1) ;
                continue ;
            }
            if(
                specifierInput.at(s).at(0).compare("XTITLE") == 0
                &&
                specifierInput.at(s).size() > 1
            ){
                for(unsigned int c=1; c<specifierInput.at(s).size() ; c++){
                    xAxisTitle += specifierInput.at(s).at(c) ;
                    if( c < specifierInput.at(s).size()-1 )
                        xAxisTitle += " " ;
                }
                continue ;
            }
            if( specifierInput.at(s).at(0).compare("XAXIS") == 0 ){
                if(
                    specifierInput.at(s).size() > 1
                    &&
                    specifierInput.at(s).at(1) != "%"
                )
                    nXdivisions = atoi( specifierInput.at(s).at(1).c_str() ) ;
                if( specifierInput.at(s).size() > 2 ){
                    xLabelFormat =
                        SpecifiedNumber( specifierInput.at(s).size()-2 ) ;
                    for(unsigned int c=2; c<specifierInput.at(s).size() ; c++){
                        xLabelFormat.specifier += specifierInput.at(s).at(c) ;
                        if( c < specifierInput.at(s).size()-1 )
                            xLabelFormat.specifier += " " ;
                    }
                }
                continue ;
            }
            if( specifierInput.at(s).at(0).compare("XNOTINTEGER") == 0 ){
                abscissaInteger = false ;
                continue ;
            }
            if(
                specifierInput.at(s).at(0).compare("SAVEAS") == 0
                &&
                specifierInput.at(s).size() > 1
            ){
                saveAS = specifierInput.at(s).at(1).c_str() ;
                continue ;
            }
            if( specifierInput.at(s).size() < 2 ) continue ;
            specifiersNquantities.push_back( {
                                                specifierInput.at(s).at(0) ,
                                                specifierInput.at(s).at(1) 
                                            } ) ;
            specifierPadMap.push_back( rangedQuantityPads.size() ) ;
            specVecDummy.clear() ;
            if( 
                specifierInput.at(s).size() > 2 
                &&  
                specifierInput.at(s).at(2).compare("%") != 0
            )
                specVecDummy.push_back( SpecifiedNumber(
                    atof( specifierInput.at(s).at(2).c_str() )
                ) ) ;
            else specVecDummy.push_back( SpecifiedNumber() ) ;
            if( 
                specifierInput.at(s).size() > 3 
                &&  
                specifierInput.at(s).at(3).compare("%") != 0
            )
                specVecDummy.push_back( SpecifiedNumber(
                    atof( specifierInput.at(s).at(3).c_str() )
                ) ) ;
            else specVecDummy.push_back( SpecifiedNumber() ) ;
            if( 
                specifierInput.at(s).size() > 4 
                &&  
                specifierInput.at(s).at(4).compare("%") != 0
            )
                specVecDummy.push_back( SpecifiedNumber(
                    atof( specifierInput.at(s).at(4).c_str() )
                ) ) ;
            else specVecDummy.push_back( SpecifiedNumber() ) ;
            markerNcolorNline.push_back( specVecDummy ) ;
            specVecDummy.clear() ;
            if( 
                specifierInput.at(s).size() > 5 
                &&  
                specifierInput.at(s).at(5).compare("%") != 0
            ){
                specifierColumns.push_back( SpecifiedNumber(
                    atof( specifierInput.at(s).at(5).c_str() )
                ) ) ;
            }
            else specifierColumns.push_back( SpecifiedNumber() ) ;
            if(
                specifierInput.at(s).size() > 6
                &&
                specifierInput.at(s).at(6).compare("%") != 0
            ){
                labels.push_back( specifierInput.at(s).at(6).c_str() ) ;
            }
            else labels.push_back( "" ) ;
        }
        nSpecific = specifiersNquantities.size() ;
        if( nSpecific < 1 ){
            cout << " ERROR :" 
                 << " no specifier found in specifier-file ( argv[4] ) " 
                 << endl;
            return 2 ;
        }
    }
    
    TString title = "" ;
    if( argc > 5 ){
        for(unsigned int a=5; a<argc; a++) title += argv[a] ;
    }
    
    if( title != "" ){
        show  = false ;
        print = false ;
        write = false ;
        if( title.Contains("show") )  show  = true ;
        if( title.Contains("print") ) print = true ;
        if( title.Contains("write") ) write = true ;
    }

    title = "" ;
    TString name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    if( plotTime.setting ){
        name += "_start" ;
        title = argv[2] ;
        title = title.ReplaceAll( "." , "p" ) ;
        name += title ;
    }
    if( duration.setting ){
        name += "_end" ;
        title = argv[3] ;
        title = title.ReplaceAll( "." , "p" ) ;
        name += title ;
    }
    if( parameterFileName.Length() > 0 ){
        name += "_" ;
        title = parameterFileName ;
        if( title.Contains(".") ) title = title( 0 , title.Last('.') ) ;
        if( title.Contains("/") ) title = title(
                                                    title.Last('/')+1 ,
                                                    title.Sizeof()
                                            ) ;
        name += title ;
    }
    name += ".root" ;
    
    TFile * outfile ;
    if( write ) outfile = new TFile( name , "RECREATE" ) ;
    
    map< TString , map< TString , TGraphErrors * > > plots ;
    map< TString , SpecifiedNumber > extrema[2] ;
    SpecifiedNumber firstTime , lastTime ;
    map< string , string > quantityUnits ;
    TString worker ;
    unsigned int rows ;
    
    vector< vector<string> > textData ;
    
    TFile * infile ;
    
    TTree * data ;
    
    TBranch * b_unixtime ;
    TBranch * b_quantity ;
    TBranch * b_specifier ;
    TBranch * b_value ;
    TBranch * b_unit ;
    
    unsigned int unixtime ;
    TString      quantity ;
    TString      specifier ;
    double       value ;
    TString      unit ;
    
    double       xvalue ;
    
    TString  * p_quantity  = 0 ;
    TString  * p_specifier = 0 ;
    TString  * p_unit      = 0 ;
    
    if( rootData ){
        
        infile = new TFile( filename , "READ" ) ;
        
        if( !( infile->IsOpen() ) ){
            cout << " ERROR: could not open file \"" 
                 << filename << "\"" << endl;
            return 3 ;
        }
        
        infile->GetObject( "data" , data ) ;
        
        if( data == NULL ){
            cout << " ERROR: not tree found " << endl ;
            return 4 ;
        }
        
        if( abscissaInteger )
            data->SetBranchAddress( "unixtime"  , &unixtime , &b_unixtime );
        else
            data->SetBranchAddress( "unixtime"  , &xvalue   , &b_unixtime  );
        data->SetBranchAddress( "quantity"  , &p_quantity  , &b_quantity  );
        data->SetBranchAddress( "specifier" , &p_specifier , &b_specifier );
        data->SetBranchAddress( "value"     , &value       , &b_value     );
        data->SetBranchAddress( "unit"      , &p_unit      , &b_unit      );
        
        rows = data->GetEntries() ;
    
    }
    else if( tableData ){
        textData = getInput( 
                                filename.Data() , 
                                delimiter.setting , 
                                delimiter.specifier 
                           ) ;
        rows = textData.size() ;
        for(unsigned int s=0; s<specifierColumns.size(); s++){
            if( maxColumns < specifierColumns.at(s).number + 1 )
                maxColumns = specifierColumns.at(s).number + 1 ;
            size_t bra = specifiersNquantities.at(s).at(1).find('[') ;
            size_t ket = specifiersNquantities.at(s).at(1).find(']') ;
            if( bra != std::string::npos && ket != std::string::npos ){
                string dummyQuantity = "" ;
                for(unsigned int c=0; c<bra; c++)
                    dummyQuantity += specifiersNquantities.at(s).at(1)[c] ;
                string dummyUnit = "" ;
                for(unsigned int c=bra+1; c<ket; c++)
                    dummyUnit += specifiersNquantities.at(s).at(1)[c] ;
                specifiersNquantities.at(s).at(1) = dummyQuantity ;
                quantityUnits[ dummyQuantity ] = dummyUnit ;
                
            }
            else
                quantityUnits[ specifiersNquantities.at(s).at(1) ] = "" ;
        }
        if( maxColumns < unixColumn + 1 )
            maxColumns = unixColumn + 1 ;
    }
    else{
        textData = getInput( filename.Data() ) ;
        rows = textData.size() ;
    }
        
    if( rows < 1 ){
        cout << " WARNING : no data found " << endl ;
        return 5 ;
    }
    
    for(unsigned int s=0; s<nSpecific; s++){
        bool toAdd = true ;
        for(unsigned int q=0; q<quantities.size(); q++ ){
            if(
                specifiersNquantities.at(s).at(1) == quantities.at(q)
                &&
                specifierPadMap.at(s)             == quantityPadMap.at(q)
            ){
                toAdd = false ;
                break ;
            }
        }
        if( toAdd ){
            quantities.push_back( specifiersNquantities.at(s).at(1) ) ;
            quantityPadMap.push_back( specifierPadMap.at(s) ) ;
        }
    }
    nQuant = quantities.size() ;
        
    cout << " => # rows : " << rows << endl ;
    cout << " first iteration  : " ;
    
    unsigned int moduloFactor = rows / 100 ;
    if( moduloFactor < 1 ) moduloFactor = 1 ;
        
    for(unsigned int r=0; r<rows; r++){
        
        if( r % moduloFactor == 0 ) cout << "*" << flush ;
        
        if(rootData){
            data->GetEntry( r ) ;
        }
        else if(tableData){
            if( textData.at(r).size() < unixColumn + 1 ) continue;
            if( abscissaInteger )
                unixtime = (unsigned int)( atof( 
                                textData.at(r).at(unixColumn).c_str() 
                            ) );
            else
                xvalue = atof( textData.at(r).at(unixColumn).c_str() );
        }
        else{
            if( textData.at(r).size() < 5 ) continue;
            if( abscissaInteger )
                unixtime = (unsigned int)( atof( 
                                textData.at(r).at(0).c_str() 
                            ) );
            else
                xvalue = atof( textData.at(r).at(0).c_str() );
        }
        
        if( abscissaInteger ){
        
            if( !( firstTime.setting ) ){
                firstTime = SpecifiedNumber( unixtime ) ;
                lastTime = SpecifiedNumber( unixtime ) ;
            }
            
            if( firstTime.number > unixtime )
                firstTime.number = unixtime ;
            if( lastTime.number < unixtime )
                lastTime.number = unixtime ;
            
        }
        else{
        
            if( !( firstTime.setting ) ){
                firstTime = SpecifiedNumber( xvalue ) ;
                lastTime = SpecifiedNumber( xvalue ) ;
            }
            
            if( firstTime.number > xvalue )
                firstTime.number = xvalue ;
            if( lastTime.number < xvalue )
                lastTime.number = xvalue ;
            
        }
        
    }
    
    cout << endl ;
        
    double startTime = firstTime.number ;
    if( plotTime.setting ) 
        startTime += ( 
                        plotTime.number 
                        * 
                        secondsPER[ plotTime.specifier ] 
                     ) ;
    double endTime = lastTime.number ;
    if( duration.setting )
        endTime = 
                    startTime 
                    + 
                    duration.number 
                    * 
                    secondsPER[ duration.specifier ] ;
    
    SpecifiedNumber dateString ;
    
    double midTime = 0.5 * ( startTime + endTime ) ;
    time_t displayTime = (unsigned int)( midTime ) ;
    char dateArray[11] ;
    const char * dateFormat ;
    string formatDate = "%d.%m.%Y" ;
    if( ( endTime - startTime ) > 2. * secondsPER["d"] )
        formatDate = "%Y" ;
    dateFormat = formatDate.c_str() ;
    strftime( 
                dateArray , 
                sizeof( dateArray ) , 
                dateFormat , 
                gmtime( &displayTime )
            ) ;
    dateString = SpecifiedNumber( displayTime ) ;
    dateString.specifier = dateArray ;
                    
    cout << " second iteration : " ;
    unsigned int currentSpecifier = 0 ;
    unsigned int currentColumn ;
        
    for(unsigned int r=0; r<rows; r++){
        
        if( currentSpecifier >= nSpecific ) currentSpecifier = 0 ;
        if( currentSpecifier == 0 && r % moduloFactor == 0 ) 
            cout << "*" << flush ;
        
        if(rootData){
            
            data->GetEntry( r ) ;
            
            quantity  = *p_quantity ;
            specifier = *p_specifier ;
            unit      = *p_unit ;
            
        }
        else if(tableData){
            
            if( textData.at(r).size() < maxColumns ) continue;
            
            currentColumn = specifierColumns.at( currentSpecifier ).number ;
            
            if( abscissaInteger )
                unixtime = (unsigned int)( atof( 
                                textData.at(r).at(unixColumn).c_str() 
                            ) );
            else
                xvalue = atof( textData.at(r).at(unixColumn).c_str() );
            
            quantity  = specifiersNquantities.at(currentSpecifier).at(1) ;
            specifier = specifiersNquantities.at(currentSpecifier).at(0) ;
            value = atof( textData.at(r).at(currentColumn).c_str() ) ;
            unit = quantityUnits[ quantity.Data() ] ;
            
            currentSpecifier++ ;
            if( currentSpecifier < nSpecific ) r-- ;
            
        }
        else{
        
            if( textData.at(r).size() < 5 ) continue;
            
            if( abscissaInteger )
                unixtime = (unsigned int)( atof( 
                                textData.at(r).at(0).c_str() 
                            ) );
            else
                xvalue = atof( textData.at(r).at(0).c_str() );
            
            quantity = textData.at(r).at(1) ;
            specifier = textData.at(r).at(2) ;
            value = atof( textData.at(r).at(3).c_str() ) ;
            unit = textData.at(r).at(4) ;
            
        }
        
        if( abscissaInteger ){
              if( unixtime < startTime || unixtime > endTime ) continue ; }
        else{ if( xvalue   < startTime || xvalue   > endTime ) continue ; }
        
        if( 
            plots.find( quantity ) == plots.end()  
            ||
                plots[quantity].find( specifier ) 
                == 
                plots[quantity].end() 
        ){
            plots[ quantity ][ specifier ] = new TGraphErrors() ;
            worker = specifier ;
            worker += "_" ;
            worker += quantity ;
            plots[ quantity ][ specifier ]->SetName( worker ) ;
            plots[ quantity ][ specifier ]->SetTitle( worker ) ;
        }
        
        if( abscissaInteger )
            plots[ quantity ][ specifier ]->SetPoint( 
                plots[ quantity ][ specifier ]->GetN() ,
                unixtime , value
            ) ;
        else
            plots[ quantity ][ specifier ]->SetPoint( 
                plots[ quantity ][ specifier ]->GetN() ,
                xvalue , value
            ) ;
        
        if( specificSpecifier ){
            bool toSkip = true ;
            for(unsigned int s=0; s<nSpecific; s++){
                if( 
                    specifiersNquantities.at(s).at(0) == specifier 
                    &&
                    specifiersNquantities.at(s).at(1) == quantity 
                ){
                    toSkip = false ;
                    break ;
                }
            }
            if( toSkip ) continue ;
        }
        else{
            
            bool toAdd = true ;
            
            for(unsigned int s=0; s<specifiersNquantities.size(); s++){
                if( 
                    specifiersNquantities.at(s).at(0) == specifier 
                    &&
                    specifiersNquantities.at(s).at(1) == quantity 
                ){
                    toAdd = false ;
                    break ;
                }
            }
            if( toAdd ) 
                specifiersNquantities.push_back( { 
                                                    specifier.Data() , 
                                                    quantity.Data() 
                                                } ) ;
            
            toAdd = true ;
            
            for(unsigned int q=0; q<quantities.size(); q++ ){
                if( quantity == quantities.at(q) ){
                    toAdd = false ;
                    break ;
                }
            }
            if( toAdd ) quantities.push_back( quantity.Data() ) ;
            
        }
        
        if( quantityUnits.find( quantity.Data() ) == quantityUnits.end() )
            quantityUnits[ quantity.Data() ] = unit ;
        else if( quantityUnits[ quantity.Data() ] != unit ){
            cout << " WARNING : different unit for " << specifier
                 << " -> " << unit 
                 << " \t using instead " << quantityUnits[ quantity.Data() ] 
                 << endl ;
        }
        
        if( abscissaInteger ){ 
              if( unixtime < startTime || unixtime > endTime ) continue ; }
        else{ if( xvalue   < startTime || xvalue   > endTime ) continue ; }
        
        if( extrema[0].find( quantity ) == extrema[0].end() ){
            extrema[0][quantity] = SpecifiedNumber( value ) ;
            extrema[1][quantity] = SpecifiedNumber( value ) ;
        }
        
        if( extrema[0][quantity].number > value )
            extrema[0][quantity].number = value ;
        if( extrema[1][quantity].number < value )
            extrema[1][quantity].number = value ;
        
    }
    
    cout << endl ;
    
    if( rootData ) infile->Close() ;
    
    nSpecific = specifiersNquantities.size() ;
    nQuant = quantities.size() ;
    
    if( markerNcolorNline.size() != nSpecific ){
        for(unsigned int s=0; s<nSpecific; s++)
            markerNcolorNline.push_back( {
                SpecifiedNumber() ,
                SpecifiedNumber() ,
                SpecifiedNumber() 
            } ) ;
    }

    if( specifierPadMap.size() != nSpecific ){
        specifierPadMap.clear() ;
        for(unsigned int s=0; s<nSpecific; s++){
            specifierPadMap.push_back( 0 ) ;
        }
    }
    
    if( quantityPadMap.size() != nQuant ){
        quantityPadMap.clear() ;
        for(unsigned int q=0; q<nQuant; q++){
            quantityPadMap.push_back( 0 ) ;
        }
    }

    unsigned int nEmpty = 0 ;
    vector<bool> emptyQuantity ;
    for(unsigned int q=0; q<nQuant; q++){
        if( 
            !( extrema[0][quantities.at(q)].setting )
            ||
            !( extrema[1][quantities.at(q)].setting )
        ){
            nEmpty++ ;
            emptyQuantity.push_back(true) ;
        }
        else emptyQuantity.push_back(false) ;
    }
    
    if( nEmpty >= nQuant ){
        cout << " ERROR : no data found " << endl ;
        return 6 ;
    }
    
    if( write ){

        outfile->cd();

        cout << " writing ... " << flush ;

        for( auto q : plots ){
            for( auto s : q.second ){
                bool toWrite = true ;
                if( specificSpecifier ){
                    toWrite = false ;
                    for(unsigned int i=0; i<nSpecific; i++){
                        if(
                            specifiersNquantities.at(i).at(0) == s.first
                            &&
                            specifiersNquantities.at(i).at(1) == q.first
                        ){
                            toWrite = true ;
                            if( labels.at(i).length() > 0 )
                                s.second->SetTitle( labels.at(i).c_str() ) ;
                            break ;
                        }
                    }
                }
                if( toWrite ) s.second->Write()  ;
                else          s.second->Delete() ;
            }
        }

    }
    
    if( !( show || print ) ){

        if( write ){
            cout << " closing ... " << flush ;
            outfile->Close();
            cout << " done " << endl ;
        }
        
        return 0 ;
    
    }
    
    cout << " drawing ... " << flush ;
    
    TApplication app("app", &argc, argv) ;
    
    name = name.ReplaceAll( ".root" , "" ) ; 
    
    TCanvas * can = new TCanvas( 
                                    name , name , 
                                    1400 , 400 * ( nQuant - nEmpty ) 
                                ) ;
    can->Divide(1,nQuant-nEmpty,0.001,0.001);
    
    TGraphErrors ** g_extrem = new TGraphErrors*[nQuant] ;
    
    unsigned int padTOuse = 0 ;
    unsigned int count = 0 ;
    for(unsigned int q=0; q<nQuant; q++){
        
        if( emptyQuantity.at(q) ) continue ;
        
        count++ ;
        
        can->cd( count ) ;
        
        double legendLowEdge = 0.025 ;
        
        if( count == nQuant-nEmpty ){ 
            legendLowEdge = 0.12 ;
            gPad->SetBottomMargin( legendLowEdge ) ;
            gStyle->SetLabelOffset( 0.005 );
        }
        else{
            gStyle->SetLabelOffset( 0.1 );
        }
       
        gPad->SetGridx() ;
        gPad->SetGridy() ;
        
        g_extrem[q] = new TGraphErrors() ;
        padTOuse = 0 ;
        title = quantities.at(q) ;
        if(
            quantityPadMap.at(q) > 0
            &&
            rangedQuantityPads.at(quantityPadMap.at(q)-1).identifier
                == quantities.at(q)
        ){
            padTOuse = quantityPadMap.at(q) ;
            title += padTOuse ;
        }
        g_extrem[q]->SetName(  title ) ;
        g_extrem[q]->SetTitle( title ) ;
        
        g_extrem[q]->SetPoint( g_extrem[q]->GetN() , 
            startTime , extrema[0][quantities.at(q)].number
        );

        g_extrem[q]->SetPoint( g_extrem[q]->GetN() ,                            
            endTime   , extrema[1][quantities.at(q)].number
        );

        if( padTOuse > 0 ){
            if( rangedQuantityPads.at(padTOuse-1).range[0].setting ){
                g_extrem[q]->SetPoint(
                    g_extrem[q]->GetN() ,
                    startTime ,
                    rangedQuantityPads.at(padTOuse-1).range[0].number
                );
            }
            if( rangedQuantityPads.at(padTOuse-1).range[1].setting ){
                g_extrem[q]->SetPoint(
                    g_extrem[q]->GetN() ,
                    endTime ,
                    rangedQuantityPads.at(padTOuse-1).range[1].number
                );
            }
            if(
                rangedQuantityPads.at(padTOuse-1).range[0].setting
                &&
                rangedQuantityPads.at(padTOuse-1).range[1].setting
            ){
                g_extrem[q]->GetYaxis()->SetRangeUser(
                    rangedQuantityPads.at(padTOuse-1).range[0].number ,
                    rangedQuantityPads.at(padTOuse-1).range[1].number
                ) ;
            }
            if( rangedQuantityPads.at(padTOuse-1).logarithmic )
                gPad->SetLogy(1) ;
            g_extrem[q]->GetYaxis()->SetNdivisions(
                rangedQuantityPads.at(padTOuse-1).nDivisions
            ) ;
        }

        g_extrem[q]->SetMarkerStyle(1) ;
        g_extrem[q]->SetMarkerColor(0) ;
        g_extrem[q]->SetLineColor(0) ;
        if( abscissaInteger ){
            g_extrem[q]->GetXaxis()->SetTimeDisplay(1) ;
            g_extrem[q]->GetXaxis()->SetTimeFormat(
                "%H:%M%F1970-01-01 00:00:00"
            ) ;
            if( ( endTime - startTime ) > 2. * secondsPER["d"] )
                g_extrem[q]->GetXaxis()->SetTimeFormat(
                    "%d.%m.%F1970-01-01 00:00:00"
                ) ;
            if( xLabelFormat.setting )
                g_extrem[q]->GetXaxis()
                           ->SetTimeFormat( xLabelFormat.specifier.c_str() ) ;
            g_extrem[q]->GetXaxis()->SetTitle( dateString.specifier.c_str() ) ;
        }
        if( xAxisTitle != "" )
            g_extrem[q]->GetXaxis()->SetTitle( xAxisTitle ) ;
        g_extrem[q]->GetXaxis()->SetNdivisions( nXdivisions ) ;
        g_extrem[q]->GetXaxis()->SetRangeUser( startTime , endTime ) ;
        g_extrem[q]->GetYaxis()->CenterTitle() ;
        TGaxis::SetExponentOffset( -0.08 , -0.05 , "y" ) ;
        TGaxis::SetMaxDigits( 3 ) ;
        
        name = quantities.at(q) ;
        if( padTOuse > 0 && rangedQuantityPads.at(padTOuse-1).name != "" )
            name = rangedQuantityPads.at(padTOuse-1).name ;
        name += " ( " ;
        if( padTOuse > 0 && rangedQuantityPads.at(padTOuse-1).unit != "" )
            name += rangedQuantityPads.at(padTOuse-1).unit ;
        else
            name += quantityUnits[quantities.at(q)] ;
        name += " ) " ;
        g_extrem[q]->GetYaxis()->SetTitle( name ) ;
        
        g_extrem[q]->Draw("AP") ;
        
        bool onlyOne = false ;
        unsigned int count = 0 ;
        for(unsigned int s=0; s<nSpecific; s++)
            if( specifiersNquantities.at(s).at(1) == quantities.at(q) ) 
                count++ ;
        if( count < 2 ) onlyOne = true ;
 
        for(unsigned int s=0; s<nSpecific; s++){

            if(
                specifiersNquantities.at(s).at(1) != quantities.at(q)
                ||
                specifierPadMap.at(s)             != quantityPadMap.at(q)
            )
                continue ;
            
            auto p = plots
                            [specifiersNquantities.at(s).at(1)]
                            [specifiersNquantities.at(s).at(0)] ;
                            
            if( p == NULL ) continue ;

            if( labels.size() == nSpecific && labels.at(s).length() > 0 )
                p->SetTitle( labels.at(s).c_str() ) ;
            else
                p->SetTitle( specifiersNquantities.at(s).at(0).c_str() ) ;

            TString toAdd = "" ;
            
            if( markerNcolorNline.at(s).at(0).setting )
                p->SetMarkerStyle( 
                    (unsigned int)markerNcolorNline.at(s).at(0).number 
                ) ;
            else if( onlyOne ) p->SetMarkerStyle( 7 ) ;
            else p->SetMarkerStyle( 6 ) ;
            
            if( markerNcolorNline.at(s).at(1).setting ){
                p->SetMarkerColor( 
                    (unsigned int)markerNcolorNline.at(s).at(1).number 
                ) ;
                p->SetLineColor( 
                    (unsigned int)markerNcolorNline.at(s).at(1).number 
                ) ;
            }
            else if( onlyOne ){
                p->SetMarkerColor(1) ;
                p->SetLineColor(1) ;
            }
            else toAdd += " PMC PLC " ;
            
            if( markerNcolorNline.at(s).at(2).setting ){ 
                p->SetLineStyle( 
                    (unsigned int)markerNcolorNline.at(s).at(2).number 
                ) ;
                name = "PLsame" ;
            }
            else name = "Psame" ;
            
            name += toAdd ;
            name += " X " ;

            p->Draw( name ) ;
            
        }
       
        TLegend * legend 
                = gPad->BuildLegend( 0.92 , legendLowEdge , 0.997 , 0.98 ) ;
        TList * legendEntries = legend->GetListOfPrimitives();
        TIter next(legendEntries);
        TObject * obj;
        TLegendEntry * le;
        
        while( ( obj = next() ) ){
            le = (TLegendEntry*)obj ;
            name = le->GetLabel() ;
            if( name.CompareTo( title ) == 0 )
                legendEntries->RemoveAt( legendEntries->IndexOf( obj ) ) ;
        }
 
    }

    cout << " showing ... " << flush ;
    
    if( show ){
        if( print ) showing() ;
        else        padWaiting() ;
    }

    if( print ){
        name = can->GetName() ;
        name += "." ;
        name += saveAS ;
        can->Print( name ) ;
    }

    if( write ){
        cout << " closing ... " << flush ;
        outfile->Close();
    }
    
    cout << " done " << endl ;
    
    return 0 ;
    
}
