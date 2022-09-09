#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ) return 1 ;

    TString filename = argv[1] ;
    
    bool writeTOfile = false ;
    
    SpecifiedNumber objectName ;
    if( argc > 2 ){
        if( string( argv[2] ) == "WRITE" ) writeTOfile = true ;
        else{
            objectName = SpecifiedNumber( 0 ) ;
            objectName.specifier = argv[2] ;
        }
    }
    
    if( argc > 3 && string( argv[3] ) == "WRITE" ) writeTOfile = true ;
    
    TFile * infile = new TFile( filename , "READ" ) ;
    if( infile->IsZombie() ){
        cout << " ERROR : opening " << filename << endl ;
        infile->Close() ;
        return 2 ;
    }    
    
    unsigned int nbins[2] ;
    TString name , title ;
    
    TString outname = filename , writename ;
    if( outname.Contains(".") ) 
        outname = outname( 0 , outname.Last('.') ) ;
    if( outname.Contains("/") ) 
        outname = outname( outname.Last('/')+1 , outname.Sizeof() ) ;
    
    TList * list = infile->GetListOfKeys() ;
    TIter next(list) ;
    TKey * key ;
    TObject * obj ;
    
    while( ( key = (TKey*)next() ) ){
        
        obj = key->ReadObj() ;
        if( obj == NULL ) continue ;
        name = obj->GetName();
        
        if( 
            (
                objectName.setting 
                && 
                !( name.EqualTo( objectName.specifier ) ) 
            )
            ||
            containsNonCommonCharacters( name.Data() )
        )
            continue ;
        
        title = obj->ClassName() ;
        
        if(
            !( title.BeginsWith( "TH1" ) )
            &&
            !( title.BeginsWith( "TH2" ) )
            &&
            !( title.BeginsWith( "TGraph" ) )
        )
            continue ;
        
        if( writeTOfile || !objectName.setting )
            cout << " " << title << " * " << name << " \t " ;
        
        if( !writeTOfile && !objectName.setting ){
            if( title.BeginsWith( "TH1" ) ){
                TH1D * hist = (TH1D*)obj ;
                cout << hist->GetNbinsX() << " [ " ;
                cout << hist->GetXaxis()->GetXmin() << " , " ;
                cout << hist->GetXaxis()->GetXmax() << " ] " ;
                cout << " \t # : " << hist->GetEntries() ;
            }
            else if( title.BeginsWith( "TH2" ) ){
                TH2D * hist = (TH2D*)obj ;
                cout << hist->GetNbinsX() << " [ " ;
                cout << hist->GetXaxis()->GetXmin() << " , " ;
                cout << hist->GetXaxis()->GetXmax() << " ] " ;
                cout << " \t " ;
                cout << hist->GetNbinsY() << " [ " ;
                cout << hist->GetYaxis()->GetXmin() << " , " ;
                cout << hist->GetYaxis()->GetXmax() << " ] " ;
                cout << " \t # : " << hist->GetEntries() ;
            }
            else if( title.BeginsWith( "TGraph" ) ){
                TGraphErrors * graph = (TGraphErrors*)obj ;
                cout << " \t # : " << graph->GetN() ;
            }
            cout << endl ;
            continue ;
        }
        
        streambuf * buffer ;
        ofstream fileTOstream ;

        if( writeTOfile ){
            writename  = outname ;
            writename += "_" ;
            writename += name ;
            writename += ".txt" ;
            fileTOstream.open( writename.Data() ) ;
            buffer = fileTOstream.rdbuf() ;
        }
        else buffer = cout.rdbuf() ;

        ostream outfile( buffer ) ;

        if( title.BeginsWith( "TH1" ) ){
            TH1D * hist = (TH1D*)obj ;
            nbins[0] = hist->GetNbinsX() ;
            double edges[ nbins[0] ] ;
            hist->GetXaxis()->GetLowEdge( edges ) ;
            outfile << "X"  ;
            for(unsigned int b=0; b<nbins[0]; b++){
                outfile << " " << edges[b] ;
            }
            outfile << " " << hist->GetXaxis()->GetBinLowEdge( nbins[0] + 1 ) ;
            outfile << endl << "Y" ;
            nbins[0] += 2 ;
            for(unsigned int b=0; b<nbins[0]; b++){
                outfile << " " << hist->GetBinContent( b ) ;
            }
        }
        else if( title.BeginsWith( "TH2" ) ){
            TH2D * hist = (TH2D*)obj ;
            nbins[0] = hist->GetNbinsX() ;
            nbins[1] = hist->GetNbinsY() ;
            double edgesX[ nbins[0] ] ;
            double edgesY[ nbins[1] ] ;
            hist->GetXaxis()->GetLowEdge( edgesX ) ;
            hist->GetYaxis()->GetLowEdge( edgesY ) ;
            outfile << "X"  ;
            for(unsigned int b=0; b<nbins[0]; b++){
                outfile << " " << edgesX[b] ;
            }
            outfile << " " << hist->GetXaxis()->GetBinLowEdge( nbins[0] + 1 ) ;
            outfile << endl << "Y" ;
            for(unsigned int b=0; b<nbins[1]; b++){
                outfile << " " << edgesY[b] ;
            }
            outfile << " " << hist->GetYaxis()->GetBinLowEdge( nbins[1] + 1 ) ;
            outfile << endl << endl ;
            nbins[0] += 2 ;
            nbins[1] += 2 ;
            for(unsigned int r=0; r<nbins[1]; r++){
                for(unsigned int c=0; c<nbins[0]; c++){
                    outfile << hist->GetBinContent( c , r ) << " " ;
                }
                outfile << endl ;
            }
        }
        else if( title.BeginsWith( "TGraph" ) ){
            bool withErrors = false ;
            if( title.Contains("Error") ) withErrors = true ;
            TGraphErrors * graph = (TGraphErrors*)obj ;
            unsigned int nPoints = graph->GetN() ;
            double x , y ;
            for(unsigned int p=0; p<nPoints; p++){
                graph->GetPoint( p , x , y ) ;
                outfile << x << " " << y ;
                if( withErrors ){
                    x = graph->GetErrorX( p ) ;
                    y = graph->GetErrorY( p ) ;
                    outfile << " " << x << " " << y ;
                }
                outfile << endl ;
            }
        }
        
        if( writeTOfile ){ 
            cout << " \t > \t " << writename << endl ;
            fileTOstream.close() ;
        }

        if( objectName.setting ) break ;

    }
    
    infile->Close() ;
    
    return 0 ;
    
}