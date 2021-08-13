#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){
    
    if( argc < 2 ) return 1 ;
        
    plotOptions() ;
    
    gStyle->SetOptStat(0) ;

    gStyle->SetPadTopMargin(    0.022 ) ;
    gStyle->SetPadRightMargin(  0.085 ) ;
    gStyle->SetPadBottomMargin( 0.020 ) ;
    gStyle->SetPadLeftMargin(   0.055 ) ;

    gStyle->SetTitleOffset( 1.1 , "x" ) ;
    gStyle->SetTitleOffset( 0.6 , "y" ) ;

    TString filename = argv[1] ;
    
    bool rootData = false ;
    if( filename.EndsWith(".root") ) rootData = true ;
    
    SpecifiedNumber plotTime , duration ;
    
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

    bool specificSpecifier = false ;
    vector< vector<string> > specifiersNquantities ;
    unsigned int nSpecific = 0 ;
    vector<string> quantities ;
    unsigned int nQuant = 0 ;

    if( argc > 4 ){
        specificSpecifier = true ;
        vector< vector<string> > specifierInput = getInput( argv[4] ) ;
        for(unsigned int s=0; s<specifierInput.size(); s++){
            if( specifierInput.at(s).size() != 2 ) continue ;
            specifiersNquantities.push_back( {
                                                specifierInput.at(s).at(0) ,
                                                specifierInput.at(s).at(1) 
                                            } ) ;
            bool toAdd = true ;
            for(unsigned int q=0; q<quantities.size(); q++ ){
                if( specifierInput.at(s).at(1) == quantities.at(q) ){
                    toAdd = false ;
                    break ;
                }
            }
            if( toAdd ) quantities.push_back( specifierInput.at(s).at(1) ) ;
        }
        nSpecific = specifiersNquantities.size() ;
        nQuant = quantities.size() ;
        if( nSpecific < 1 || nQuant < 1 ){
            cout << " ERROR :" 
                 << " no specifier found in specifier-file ( argv[4] ) " 
                 << endl;
            return 2 ;
        }
    }
    
    TString name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) name = name( name.Last('/')+1 , name.Sizeof() ) ;
    name += "_plots.root" ;
    
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
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
        
        data->SetBranchAddress( "unixtime"  , &unixtime    , &b_unixtime  );
        data->SetBranchAddress( "quantity"  , &p_quantity  , &b_quantity  );
        data->SetBranchAddress( "specifier" , &p_specifier , &b_specifier );
        data->SetBranchAddress( "value"     , &value       , &b_value     );
        data->SetBranchAddress( "unit"      , &p_unit      , &b_unit      );
        
        rows = data->GetEntries() ;
    
    }
    else{
        textData = getInput( filename.Data() ) ;
        rows = textData.size() ;
    }
        
    if( rows < 1 ){
        cout << " WARNING : no data found " << endl ;
        return 5 ;
    }
        
    cout << " => # rows : " << rows << endl ;
    cout << " first iteration  : " ;
    
    unsigned int moduloFactor = rows / 100 ;
        
    for(unsigned int r=0; r<rows; r++){
        
        if( r % moduloFactor == 0 ) cout << "*" << flush ;
        
        if(rootData){
            data->GetEntry( r ) ;
        }
        else{
            if( textData.at(r).size() < 5 ) continue;
            unixtime = (unsigned int)( atof( textData.at(r).at(0).c_str() ) );
        }
        
        if( !( firstTime.setting ) ){
            firstTime = SpecifiedNumber( unixtime ) ;
            lastTime = SpecifiedNumber( unixtime ) ;
        }
        
        if( firstTime.number > unixtime )
            firstTime.number = unixtime ;
        if( lastTime.number < unixtime )
            lastTime.number = unixtime ;
        
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
    
    time_t startingTime = (unsigned int)( startTime ) ;
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
                gmtime( &startingTime ) 
            ) ;
    dateString = SpecifiedNumber( startingTime ) ;
    dateString.specifier = dateArray ;
                    
    cout << " second iteration : " ;
        
    for(unsigned int r=0; r<rows; r++){
        
        if( r % moduloFactor == 0 ) cout << "*" << flush ;
        
        if(rootData){
            
            data->GetEntry( r ) ;
            
            quantity  = *p_quantity ;
            specifier = *p_specifier ;
            unit      = *p_unit ;
            
        }
        else{
        
            if( textData.at(r).size() < 5 ) continue;
            
            unixtime = (unsigned int)( atof( textData.at(r).at(0).c_str() ) );
            quantity = textData.at(r).at(1) ;
            specifier = textData.at(r).at(2) ;
            value = atof( textData.at(r).at(3).c_str() ) ;
            unit = textData.at(r).at(4) ;
            
        }
        
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
        
        plots[ quantity ][ specifier ]->SetPoint( 
            plots[ quantity ][ specifier ]->GetN() ,
            unixtime , value
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
        
        if( unixtime < startTime || unixtime > endTime ) continue ;
        
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
    
    nQuant = quantities.size() ;
    
    outfile->cd();
    
    cout << " writing ... " << flush ;
    
    for( auto q : plots ){
        for( auto s : q.second ){
            s.second->Write() ;
        }
    }
    
    cout << " drawing ... " << flush ;
    
    TApplication app("app", &argc, argv) ;
    
    name = name.ReplaceAll( ".root" , "" ) ; 
    
    TCanvas * can = new TCanvas( name , name , 1400 , 400 * nQuant ) ;
    can->Divide(1,nQuant,0.001,0.001);
    
    TGraphErrors ** g_extrem = new TGraphErrors*[nQuant] ;
    TString title ;
    
    for(unsigned int q=0; q<nQuant; q++){

        can->cd( q+1 ) ;
        
        double legendLowEdge = 0.025 ;
        
        if( q+1 == nQuant ){ 
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
        g_extrem[q]->SetName( quantities.at(q).c_str() ) ;
        g_extrem[q]->SetTitle( quantities.at(q).c_str() ) ;
        
        g_extrem[q]->SetPoint( g_extrem[q]->GetN() , 
            startTime , extrema[0][quantities.at(q)].number
        );

        g_extrem[q]->SetPoint( g_extrem[q]->GetN() ,                            
            endTime   , extrema[1][quantities.at(q)].number
        ); 

        g_extrem[q]->SetMarkerStyle(1) ;
        g_extrem[q]->SetMarkerColor(0) ;
        g_extrem[q]->SetLineColor(0) ;
        g_extrem[q]->GetXaxis()->SetTimeDisplay(1) ;
        g_extrem[q]->GetXaxis()->SetTimeFormat("%H:%M%F1970-01-01 00:00:00") ;
        if( ( endTime - startTime ) > 2. * secondsPER["d"] )
            g_extrem[q]->GetXaxis()->SetTimeFormat(
                "%d.%m.%F1970-01-01 00:00:00"
            ) ;
        g_extrem[q]->GetXaxis()->SetNdivisions(525) ;
        g_extrem[q]->GetXaxis()->SetTitle( dateString.specifier.c_str() ) ;
        g_extrem[q]->GetXaxis()->SetRangeUser( startTime , endTime ) ;
        g_extrem[q]->GetYaxis()->CenterTitle() ;
        TGaxis::SetExponentOffset( -0.05 , -0.05 , "y" ) ;
        TGaxis::SetMaxDigits( 3 ) ;
        
        title = quantities.at(q) ;
        title += " [ " ;
        title += quantityUnits[quantities.at(q)] ;
        title += " ] " ;
        g_extrem[q]->GetYaxis()->SetTitle( title ) ;
        
        g_extrem[q]->Draw("AP") ;
 
        for( auto p : plots[quantities.at(q)] ){
            
            if( specificSpecifier ){
                bool toSkip = true ;
                for(unsigned int s=0; s<nSpecific; s++){
                    if( 
                        specifiersNquantities.at(s).at(0) == p.first
                        &&
                        specifiersNquantities.at(s).at(1) == quantities.at(q)
                    ){
                        toSkip = false ;
                        break ;
                    }
                }
                if( toSkip ) continue ;
            }

            title = p.second->GetName() ;
            if( title.EndsWith( quantities.at(q).c_str() ) )
                title = title( 
                                0 , 
                                title.Length() 
                                - 
                                quantities.at(q).length() 
                            ) ;
            if( title.EndsWith( "_" ) )
                title = title( 0 , title.Length()-1 ) ;
            p.second->SetName( title ) ;
            p.second->SetTitle( title ) ;

            p.second->SetMarkerStyle(6) ; 
            p.second->Draw("SAME P PMC PLC") ;
            
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
            if( name.CompareTo(quantities.at(q)) == 0 )
                legendEntries->RemoveAt( legendEntries->IndexOf( obj ) ) ;
        }
 
    }

    cout << " showing ... " << flush ;
    
   showing() ;    

    name = can->GetName() ;
    name += ".pdf" ;
    can->Print( name ) ;

    cout << " closing ... " << flush ;
    
    outfile->Close();
    
    cout << " done " << endl ;
    
    return 0 ;
    
}
