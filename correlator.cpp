#include "postprocessing.h"

using namespace std;

void plotOptions() ;

int main(int argc, char *argv[]){

    if( argc < 2 ){ 
        cout << " usage : " << argv[0] << endl ;
        cout << " append either single text-file-name " << endl ;
        cout << " or : <file1> <hist1> <file2> <hist2> " << endl ;
        cout << " optional : <low1> <high1> <low2> <hihg2> " << endl ;
        return 1 ;
    }
    
    plotOptions() ;

    TString filename = argv[1] ;
    TString filesNhists[2][2] ;
    SpecifiedNumber ranges[2][2] ;
    TString outname ;

    unsigned int count = 0 ;
    
    if( argc > 4 && filename.EndsWith(".root") ){
        filesNhists[0][0] = argv[1] ;
        filesNhists[0][1] = argv[2] ;
        filesNhists[1][0] = argv[3] ;
        filesNhists[1][1] = argv[4] ;
        if( argc > 8 ){
            ranges[0][0] = SpecifiedNumber( atof( argv[5] ) ) ;
            ranges[0][1] = SpecifiedNumber( atof( argv[6] ) ) ;
            ranges[1][0] = SpecifiedNumber( atof( argv[7] ) ) ;
            ranges[1][1] = SpecifiedNumber( atof( argv[8] ) ) ;
        }
        outname = filesNhists[0][1] ;
        outname += "_VS_" ;
        outname += filesNhists[1][1] ;
        outname += ".root" ;
    }
    else{
        
        vector< vector<string> > parameter = getInput( filename.Data() ) ;
        
        if( parameter.size() < 2 ){
            cout << " parameter-file should be of the format : " << endl ;
            cout << " file1 hist1 (low1) (high1) " << endl ;
            cout << " file2 hist2 (low2) (high2) " << endl ;
            cout << " bracket values are optional " << endl ;
            return 2 ;
        }
        
        for(unsigned int r=0; r<parameter.size(); r++){

            if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//                 cout << " comment line " << r << endl ;
                continue ;
            }
            
            if( parameter.at(r).size() < 2 ) continue ;
            
            if( count > 1 ) continue ;
            
            filesNhists[count][0] = parameter.at(r).at(0) ;
            filesNhists[count][1] = parameter.at(r).at(1) ;
            
            if( parameter.at(r).size() > 3 ){
                ranges[count][0] 
                    = SpecifiedNumber( atof( parameter.at(r).at(2).c_str() ) ) ;
                ranges[count][1] 
                    = SpecifiedNumber( atof( parameter.at(r).at(3).c_str() ) ) ;
            }
            
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
    
    count = 0 ; 
    
    map< string , map< unsigned int , bool > > useRowsNcolumns ;
    TH2D ** hists = new TH2D*[2] ;
    
    for(unsigned int h=0; h<2; h++){
    
        TFile * input = new TFile( filesNhists[h][0] , "READ" ) ;
        
        if( input->IsZombie() ){
            cout << " ERROR : opening " << input->GetName() << endl ;
            return 4 ;
        }
        
        hists[h] = (TH2D*)input->Get( filesNhists[h][1] ) ;
        
        if( hists[h] == NULL ){
            cout << " ERROR : reading " << filesNhists[h][1] 
                    << " in " << input->GetName() << endl ;
            return 5 ;
        }
        
        hists[h]->SetDirectory(0) ;
        input->Close() ;
        
        double mean , stdv , min , max , median ;
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
                                    useRowsNcolumns
                                );
        
        if(working){
            count++ ;
            if( ! ranges[h][0].setting )
                ranges[h][0].number = min ;
            if( ! ranges[h][1].setting )
                ranges[h][1].number = max ;
        }
    
    }
    
    unsigned int bins[2] = {
        (unsigned int)hists[0]->GetNbinsX() ,
        (unsigned int)hists[0]->GetNbinsY() 
    } ;
    
    if( count != 2 
        ||
        bins[0] != hists[1]->GetNbinsX()
        ||
        bins[1] != hists[1]->GetNbinsY()
    ){
        cout << " ERROR : histograms ill-defined " << endl ;
        return 6 ;
    }
    
    TFile * outfile = new TFile( outname , "RECREATE" ) ;
    
    TString name = "h_" ;
    name += filesNhists[1][1] ;
    name += "_VS_" ;
    name += filesNhists[0][1] ;
    TH2I * h_correlation = new TH2I( 
                            name , name ,
                            2000 , ranges[0][0].number , ranges[0][1].number 
                            + (ranges[0][1].number-ranges[0][0].number)/2000. ,
                            2000 , ranges[1][0].number , ranges[1][1].number 
                            + (ranges[1][1].number-ranges[1][0].number)/2000. 
                        ) ;
    
    name = "g_" ;
    name += filesNhists[1][1] ;
    name += "_VS_" ;
    name += filesNhists[0][1] ;
    TGraphErrors * g_correlation = new TGraphErrors() ; 
    g_correlation->SetName( name ) ;
    g_correlation->SetTitle( name ) ;
    
    for(unsigned int x=1; x<=bins[0]; x++){
        for(unsigned int y=1; y<=bins[1]; y++){
            double a = hists[0]->GetBinContent( x , y ) ;
            double b = hists[1]->GetBinContent( x , y ) ;
            if( toDiscard( a ) ) continue ;
            if( toDiscard( b ) ) continue ;
            h_correlation->Fill( a , b ) ;
            if( ranges[0][0].setting && a < ranges[0][0].number ) continue ;
            if( ranges[0][1].setting && a > ranges[0][1].number ) continue ;
            if( ranges[1][0].setting && b < ranges[1][0].number ) continue ;
            if( ranges[1][1].setting && b < ranges[1][1].number ) continue ;
            g_correlation->SetPoint( g_correlation->GetN() , a , b ) ;
        }
    }
    
    outfile->cd() ;
    
    h_correlation->Write() ;
    g_correlation->Write() ;
    
    TApplication app("app", &argc, argv) ; 
    name = "hc_" ;
    name += filesNhists[1][1] ;
    name += "_VS_" ;
    name += filesNhists[0][1] ;
    TCanvas * can = new TCanvas( name , name , 700 , 600 ) ;
    
    h_correlation->Draw("COLZ") ;
        
    showing() ;
    
    name = can->GetName() ;
    name += ".pdf" ;
    can->Print(name);
    can->Close() ;
    
    outfile->Close() ;
    
    name = "gc_" ;
    name += filesNhists[1][1] ;
    name += "_VS_" ;
    name += filesNhists[0][1] ;
    can = new TCanvas( name , name , 700 , 600 ) ;
    
    g_correlation->Draw("AP") ;
        
    showing() ;
    
    name = can->GetName() ;
    name += ".pdf" ;
    can->Print(name);
    can->Delete() ;

    return 0 ;

}
