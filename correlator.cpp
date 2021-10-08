#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 2 ){ 
        cout << " usage : " << argv[0] << endl ;
        cout << " append either single text-file-name " << endl ;
        cout << " or : <file1> <hist1> <file2> <hist2> " << endl ;
        cout << " optional : <low1> <high1> <low2> <hihg2> " << endl ;
        return 1 ;
    }
    
    plotOptions() ;

    gStyle->SetPadTopMargin(    0.055 ) ;
    gStyle->SetPadRightMargin(  0.16 ) ;
    gStyle->SetPadBottomMargin( 0.10 ) ;
    gStyle->SetPadLeftMargin(   0.14 ) ;

    gStyle->SetTitleOffset( 1.0 , "x" ) ;
    gStyle->SetTitleOffset( 1.4 , "y" ) ;

    TString filename = argv[1] ;
    TString filesNhists[2][2] ;
    SpecifiedNumber ranges[2][2] ;
    SpecifiedNumber divisions[2] ;
    TString outname ;
    TString name ;

    string neverUse = "neverUseThisPhrase" ;
    string preNsuffix[2][2] = { 
                                { neverUse , neverUse } ,
                                { neverUse , neverUse }
                            };
    string axisTitles[2] = { neverUse , neverUse } ;
    map< string , map< unsigned int , bool > > useRowsNcolumns ;
    map< string , vector<int> > givenRowsNcolumns ;

    unsigned int count = 0 ;
    
    if( argc > 4 && filename.EndsWith(".root") ){
        filesNhists[0][0] = argv[1] ;
        filesNhists[0][1] = argv[2] ;
        filesNhists[1][0] = argv[3] ;
        filesNhists[1][1] = argv[4] ;
        if( argc > 8 ){
            if( string( argv[5] ).compare( "%" ) != 0  )
                ranges[0][0] = SpecifiedNumber( atof( argv[5] ) ) ;
            if( string( argv[6] ).compare( "%" ) != 0  )
                ranges[0][1] = SpecifiedNumber( atof( argv[6] ) ) ;
            if( string( argv[7] ).compare( "%" ) != 0  )
                ranges[1][0] = SpecifiedNumber( atof( argv[7] ) ) ;
            if( string( argv[8] ).compare( "%" ) != 0  )
                ranges[1][1] = SpecifiedNumber( atof( argv[8] ) ) ;
        }
        if( argc > 10 ){
            if( string( argv[9] ).compare( "%" ) != 0  )
                divisions[0] = SpecifiedNumber( atof( argv[9] ) ) ;
            if( string( argv[10] ).compare( "%" ) != 0  )
                divisions[1] = SpecifiedNumber( atof( argv[10] ) ) ;
        }
        outname = filesNhists[0][0] ;
        outname += "_" ;
        outname += filesNhists[0][1] ;
        outname += "_VS_" ;
        outname += filesNhists[1][0] ;
        outname += "_" ;
        outname = filesNhists[1][1] ;
        outname.ReplaceAll( ".root" , "" ) ;
        outname += ".root" ;
    }
    else{
        
        vector< vector<string> > parameter = getInput( filename.Data() ) ;
        
        for(unsigned int r=0; r<parameter.size(); r++){

            if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//                 cout << " comment line " << r << endl ;
                continue ;
            }

            int specifier = -1 ;

            if( parameter.at(r).at(0).compare("FILE") == 0  ) 
                specifier = 0 ;
            else if( parameter.at(r).at(0).compare("HIST") == 0  ) 
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
                }
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
            
            if( parameter.at(r).size() < 2 ) continue ;
            
            if( count > 1 ) continue ;
            
            filesNhists[count][0] = parameter.at(r).at(0) ;
            filesNhists[count][1] = parameter.at(r).at(1) ;
            
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
            if( filesNhists[t][f] == "%" )
                filesNhists[t][f] = "" ;
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
    
    count = 0 ; 
    
    TH2D ** hists = new TH2D*[2] ;
    
    for(unsigned int h=0; h<2; h++){

        name = preNsuffix[0][0] ;
        name += filesNhists[h][0] ;
        name += preNsuffix[0][1] ;
        TFile * input = new TFile(name,"READ") ;
        if( input->IsZombie() ){
            cout << " ERROR : opening " << input->GetName() << endl ;
            return 4 ;
        }

        name = preNsuffix[1][0] ;
        name += filesNhists[h][1] ;
        name += preNsuffix[1][1] ;
        if( input->Get(name) == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            return 5 ;
        }
        
        hists[h] = (TH2D*)input->Get( name ) ;
        if( hists[h] == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            return 6 ;
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
        
        if( ! divisions[h].setting ) divisions[h] = 2000 ;
    
    }
    
    unsigned int bins[2] = {
        (unsigned int)hists[0]->GetNbinsY() ,
        (unsigned int)hists[0]->GetNbinsX() 
    } ;
    
    if( count != 2 
        ||
        bins[0] != hists[1]->GetNbinsY()
        ||
        bins[1] != hists[1]->GetNbinsX()
    ){
        cout << " ERROR : histograms ill-defined " << endl ;
        return 7 ;
    }
            
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
    
    TFile * outfile = new TFile( outname , "RECREATE" ) ;
    
    name = "h_" ;
    name += filesNhists[1][1] ;
    name += "_VS_" ;
    name += filesNhists[0][1] ;
    TH2I * h_correlation = new TH2I( 
                            name , name ,
                            (unsigned int)divisions[0].number , 
                            ranges[0][0].number , 
                            ranges[0][1].number 
                                + (ranges[0][1].number-ranges[0][0].number)
                                    /divisions[0].number ,
                            (unsigned int)divisions[1].number , 
                            ranges[1][0].number , 
                            ranges[1][1].number 
                                + (ranges[1][1].number-ranges[1][0].number)
                                    /divisions[1].number
                        ) ;
    
    name = "g_" ;
    name += filesNhists[1][1] ;
    name += "_VS_" ;
    name += filesNhists[0][1] ;
    TGraphErrors * g_correlation = new TGraphErrors() ; 
    g_correlation->SetName( name ) ;
    g_correlation->SetTitle( name ) ;
    
    for(unsigned int x=1; x<=bins[1]; x++){
        for(unsigned int y=1; y<=bins[0]; y++){
            if( 
                ! useRowsNcolumns["ROWS"   ][y] 
                || 
                ! useRowsNcolumns["COLUMNS"][x] 
            )
                continue ;
            double a = hists[0]->GetBinContent( y , x ) ;
            double b = hists[1]->GetBinContent( y , x ) ;
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
    
    h_correlation->GetXaxis()->SetTitle( axisTitles[0].c_str() ) ;
    h_correlation->GetYaxis()->SetTitle( axisTitles[1].c_str() ) ;
    
    TApplication app("app", &argc, argv) ; 
//     name = "hc_" ;
//     name += filesNhists[1][1] ;
//     name += "_VS_" ;
//     name += filesNhists[0][1] ;
    name = outname ;
    name = name.ReplaceAll( ".root" , "" ) ;
    TCanvas * can = new TCanvas( name , name , 700 , 600 ) ;
    
    h_correlation->Draw("COLZ") ;
        
    showing() ;
    
    name += ".pdf" ;
    can->Print(name);
    can->Close() ;
    
    outfile->Close() ;
    
//     name = "gc_" ;
//     name += filesNhists[1][1] ;
//     name += "_VS_" ;
//     name += filesNhists[0][1] ;
//     can = new TCanvas( name , name , 700 , 600 ) ;
//     
//     g_correlation->Draw("AP") ;
//         
//     showing() ;
//     
//     name = can->GetName() ;
//     name += ".pdf" ;
//     can->Print(name);
//     can->Delete() ;

    return 0 ;

}
