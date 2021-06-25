#include "postprocessing.h"

using namespace std;

unsigned int secondsPERday = 86400 ; // 24 * 60 * 60

int main(int argc, char *argv[]){
    
    if( argc < 2 ) return 1 ;
        
    gStyle->SetOptTitle(0) ;
    gStyle->SetOptStat(0) ;
    gStyle->SetOptFit(0) ;

    gStyle->SetPadTopMargin(    0.05 ) ;
    gStyle->SetPadRightMargin(  0.03 ) ;
    gStyle->SetPadBottomMargin( 0.12 ) ;
    gStyle->SetPadLeftMargin(   0.08 ) ;

    gStyle->SetTitleOffset( 1.2 , "x" ) ;
    gStyle->SetTitleOffset( 0.8 , "y" ) ;
    
    double text_size = 0.05 ;
    int font = 42 ;

    gStyle->SetLabelFont(font,"x");
    gStyle->SetTitleFont(font,"x");
    gStyle->SetLabelFont(font,"y");
    gStyle->SetTitleFont(font,"y");

    gStyle->SetLabelSize(text_size,"x") ;
    gStyle->SetTitleSize(text_size,"x") ;
    gStyle->SetLabelSize(text_size,"y") ;
    gStyle->SetTitleSize(text_size,"y") ;

    TString filename = argv[1] ;
    
    TString outname = filename ;
    
    TString nameDefault = "logs.root" ;
    
    TString readname ;
    
    vector<TString> dataFileNames ;
    
    if( outname.EndsWith(".dat") ){
        dataFileNames.push_back( filename ) ;
        outname = outname.ReplaceAll( ".dat" , ".root" ) ;
        if( outname.Contains("/") ){
            TString prefix = outname( 0 , outname.Last('/') ) ;
            TString suffix = outname( outname.Last('/')+1 , outname.Sizeof() ) ;
            if( prefix.EndsWith("/logs") ){
                prefix = prefix.ReplaceAll( "/logs" , "" ) ;
                prefix = prefix( prefix.Last('/')+1 , prefix.Sizeof() ) ;
                outname = prefix ;
                outname += "_" ;
                outname += suffix ;
            }
            else outname = suffix ;
        }
    }
    else{
        if( outname.EndsWith("/") )
            outname = outname( 0 , outname.Length()-1 ) ;
        if( outname.EndsWith("logs") )
            outname = outname( 0 , outname.Length()-4 ) ;
        if( outname.EndsWith("/") )
            outname = outname( 0 , outname.Length()-1 ) ;
        if( outname.Length() == 0 ) outname = nameDefault ;
        else if( outname.Contains("/") ){
            outname 
                = outname( outname.Last('/')+1 , outname.Sizeof() ) ;
            outname += ".root" ;
        }
        else outname += ".root" ;
    }
    
    if( dataFileNames.size() < 1 ){
    
        TSystemDirectory mainDir( filename , filename ); 
        TList * files = mainDir.GetListOfFiles(); 
        
        if( !files ){
            cout << " ERROR : directory \"" 
                 << filename << "\" not found => EXIT " << endl ;
            return 2 ;
        }
        
        TSystemFile * readfile ; 
        TIter next( files ) ;
        
        while( ( readfile = (TSystemFile*)next() ) ){ 
            
            readname = readfile->GetName() ; 
            
            if( 
                !( readname.BeginsWith("housekeeping_") ) ||
                !( readname.EndsWith(".dat") )
            ) continue;
            
            dataFileNames.push_back( readname ) ;
            
        }
        
        if( dataFileNames.size() < 1 ){
            cout << " ERROR : no files found in \""
                 << filename << "\" => EXIT " << endl ;
            return 3 ;
        }
        
    }
    
    TFile * outfile = new TFile( outname , "RECREATE" ) ;
    
    if( outfile == NULL ){
        cout << " ERROR : can not create output \"" 
             <<  outname << "\" => EXIT " << endl ;
    }
    
    TTree * data ;
    
//     TBranch * b_unixtime ;
//     TBranch * b_quantity ;
//     TBranch * b_specifier ;
//     TBranch * b_value ;
//     TBranch * b_unit ;
    
    unsigned int unixtime ;
    TString      quantity ;
    TString      specifier ;
    double       value ;
    TString      unit ;
    
    data = new TTree( "data" , "housekeeping" );
    
    data->Branch( "unixtime"  , &unixtime  );
    data->Branch( "quantity"  , &quantity  );
    data->Branch( "specifier" , &specifier );
    data->Branch( "value"     , &value     );
    data->Branch( "unit"      , &unit      );
    
    unsigned int nFiles = dataFileNames.size() ;
    
    map<TString,TGraph*> plots ; 
    map<TString,bool> setAxisTitle ;
    vector<string> foundSpecifier ;
    TString name ;
    
    if( argc > 2 ){
        for(unsigned int a=2; a<argc; a++){
            plots[ argv[a] ] = new TGraph() ;
            name = "plot" ;
            name += a-2 ;
            plots[ argv[a] ]->SetName( name ) ;
            plots[ argv[a] ]->SetTitle( argv[a] ) ;
            setAxisTitle[ argv[a] ] = true ;
        }
    }
    
    for(unsigned int f=0; f<nFiles; f++){
        
        if( nFiles == 1 ) readname = dataFileNames.at(f) ;
        else{
            readname = filename ;
            if( ! readname.EndsWith("/") ) readname += "/" ;
            readname += dataFileNames.at(f) ;
        }
        
        cout << " " << f+1 << "/" << nFiles << " : " << dataFileNames.at(f) ;
        cout << flush ;
        
        vector< vector<string> > textData 
            = getInput( readname.Data() ) ;
            
        unsigned int rows = textData.size() ;
        
        cout << " => # rows : " << rows << endl ;
        
        for(unsigned int r=0; r<rows; r++){
            
            if( textData.at(r).size() < 5 ) continue;
            
            unixtime = (unsigned int)( atof( textData.at(r).at(0).c_str() ) );
            quantity = textData.at(r).at(1) ;
            specifier = textData.at(r).at(2) ;
            value = atof( textData.at(r).at(3).c_str() ) ;
            unit = textData.at(r).at(4) ;
            
            data->Fill();
            
            if( find( 
                        foundSpecifier.begin() , 
                        foundSpecifier.end() , 
                        specifier.Data() 
                    ) 
                == 
                foundSpecifier.end() 
            ){
                foundSpecifier.push_back( specifier.Data() ) ;
            }
            
            if( argc > 2 && nFiles == 1 ){
                if( plots.find( specifier ) == plots.end() ) continue ;
                plots[ specifier ]->SetPoint( 
                                                plots[ specifier ]->GetN() , 
                                                unixtime ,
                                                value
                                            ) ;
                if( setAxisTitle[ specifier ] ){
                    name = specifier ;
                    name += " [" ;
                    name += unit ;
                    name += "] " ;
                    plots[ specifier ]->GetYaxis()->SetTitle( name ) ;
                    setAxisTitle[ specifier ] = false ;
                }
            }
            
        }
        
    }
    
    outfile->cd();
    
    cout << " specifier : " ;
    for(unsigned int s=0; s<foundSpecifier.size(); s++)
        cout << " " << foundSpecifier.at(s) ;
    cout << endl ;
    
    cout << " writing ... " ;
    
    data->Write();
    
    if( argc > 2 && nFiles == 1 ){
    
        TApplication app("app", &argc, argv) ; 
        
        TCanvas * can = new TCanvas( "can" , "can" , 1500 , 500 ) ;
        
        for( auto p : plots ){
            
            
            can->SetName( p.first ) ;
            can->SetTitle( p.first ) ;
            
            name = outname ;
            name = name.ReplaceAll( ".root" , "_" ) ;
            name += p.first ;
            name += ".pdf" ;
            
            unsigned int dayStartTime = 
                (unsigned int)( 
                    TMath::MaxElement( p.second->GetN() , p.second->GetX() )
                ) / secondsPERday ;
            dayStartTime *= secondsPERday ;
            time_t startingTime = dayStartTime ;
            char dateArray[11] ;
            char dateFormat[] = "%d.%m.%Y" ;
            strftime( dateArray , sizeof( dateArray ) , dateFormat , gmtime( &startingTime ) ) ;
            dayStartTime -= 7200 ;
            
            p.second->SetMarkerStyle(7) ;
            p.second->GetXaxis()->SetTimeDisplay(1) ;
            p.second->GetXaxis()->SetTimeFormat("%H:%M%F1970-01-01 00:00:00") ;
//             p.second->GetXaxis()->SetTimeOffset( 0 , "GMT" ) ;
            p.second->GetXaxis()->SetNdivisions(515);
            p.second->Write() ;
            p.second->GetXaxis()->SetRangeUser(
                dayStartTime ,
                dayStartTime + secondsPERday 
            );
            p.second->GetXaxis()->SetTitle( dateArray ) ;
            
            p.second->Draw("AP") ;
            
            gPad->SetGridx() ;
            gPad->SetGridy() ;
            
//             gPad->Modified() ;
//             gPad->Update() ;
//             gPad->WaitPrimitive() ;
            gPad->Print( name ) ;
            
        }
        
    }
    
    cout << " closing ... " ;
    
    outfile->Close();
    
    cout << " done " << endl ;
    
    return 0 ;
    
}
