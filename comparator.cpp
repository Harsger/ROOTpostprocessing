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

    vector< vector<string> > filesNhists ;
    vector<string> strVecDummy ;

    for(unsigned int r=0; r<parameter.size(); r++){

        if( parameter.at(r).at(0).rfind("#",0) == 0 ){ 
//            cout << " comment line " << r << endl ;
            continue ;
        }

        int fileORhist = -1 ;

        if( parameter.at(r).at(0).compare("FILE") == 0  ) fileORhist = 0 ;
        else if( parameter.at(r).at(0).compare("HIST") == 0  ) fileORhist = 1 ;

        if( fileORhist > -1 && parameter.at(r).size() > 2 ){
            preNsuffix[fileORhist][0] = parameter.at(r).at(1) ;
            preNsuffix[fileORhist][1] = parameter.at(r).at(2) ;
            continue ;
        }

        if( parameter.at(r).size() > 1 ){
            strVecDummy.push_back( parameter.at(r).at(0) );
            strVecDummy.push_back( parameter.at(r).at(1) );
        }

        filesNhists.push_back( strVecDummy ) ;
        strVecDummy.clear() ;

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

    for(unsigned int r=0; r<filesNhists.size(); r++){
        for(unsigned int c=0; c<2; c++){
            if( filesNhists.at(r).at(c).compare( "%" ) == 0 )
                filesNhists.at(r).at(c) = "" ;
        }
    }

    unsigned int nHists = filesNhists.size() ;
    TH2D** hists = new TH2D*[nHists] ;
    TString name ;
    unsigned int rowsNcolumns[2] = { 0 , 0 } ;
    unsigned int notFound = 0 ;
    bool useable[nHists] ;
    
    for(unsigned int r=0; r<nHists; r++){

        name = preNsuffix[0][0] ;
        name += filesNhists.at(r).at(0) ;
        name += preNsuffix[0][1] ;
        TFile * input = new TFile(name,"READ") ;
        if( input->IsZombie() ){
            cout << " ERROR : opening " << name << endl ;
            notFound++ ;
            useable[r] = false ;
            continue ;
        }

        name = preNsuffix[1][0] ;
        name += filesNhists.at(r).at(1) ;
        name += preNsuffix[1][1] ;
        if( input->Get(name) == NULL ){
            cout << " ERROR : reading " << name 
                 << " in " << input->GetName() << endl ;
            notFound++ ;
            useable[r] = false ;
            continue ;
        }
        hists[r] = (TH2D*)input->Get(name) ;

        hists[r]->SetDirectory(0) ;
        input->Close() ;

        if( rowsNcolumns[1] == 0 ){
            rowsNcolumns[1] = hists[r]->GetNbinsX() ;
            rowsNcolumns[0] = hists[r]->GetNbinsY() ;
        }        
        else if(  
            rowsNcolumns[1] != hists[r]->GetNbinsX()
            ||
            rowsNcolumns[0] != hists[r]->GetNbinsY()
        ){
            notFound++ ;
            useable[r] = false ;
            continue ;
        }

        useable[r] = true ;

    }

    if( rowsNcolumns[1] == 0 && rowsNcolumns[0] == 0 ){
        cout << " ERROR : empty histograms " << endl ;
        return 1 ;
    }

    unsigned int nToUse = nHists - notFound ;

    TApplication app("app", &argc, argv) ;

//    for(unsigned int h=0; h<hists.size(); h++){
//        hists.at(h)->Draw("COLZ") ;
//        gPad->Modified() ;
//        gPad->Update() ;
//        gPad->WaitPrimitive() ;
//        hists.at(h)->Delete() ;
//    }

    name = filename ;
    if( name.Contains(".") ) name = name( 0 , name.Last('.') ) ;
    name += ".root" ;
    TFile * outfile = new TFile( name , "RECREATE" ) ;

    vector<TString> histIdentifier ;
    bool tooManyDefault = false ;
    for(unsigned int h=0; h<nHists; h++){
        name = "" ;
        bool bothSpecified = true ;
        if( 
            filesNhists.at(h).at(0).empty() 
            ||
            filesNhists.at(h).at(1).empty() 
        )
            bothSpecified = false ;
        if( bothSpecified ) name += "file_" ;
        name += filesNhists.at(h).at(0) ;
        if( bothSpecified ) name += "_hist_" ;
        name += filesNhists.at(h).at(1) ;
        name = name.ReplaceAll( "."  , "" ) ;
        name = name.ReplaceAll( "-"  , "" ) ;
        if( name == "" ){
            name = "default" ;
            if( tooManyDefault ){
                cout << " ERROR : " 
                     << "histnames can not be properly specified " << endl ;
                return 1 ;
            }
            else tooManyDefault = true ;
        }
        TString test = name(0,1) ;
        if( test.IsDec() ) name = "h"+name ;
        histIdentifier.push_back( name ) ;
    }

    outfile->cd() ;

    TH2I ** differenceTOmean = new TH2I*[2] ;
    differenceTOmean[0] = new TH2I( 
                                    "differenceTOmean_relative" ,
                                    "differenceTOmean_relative" ,
                                    nHists , 0.5 , nHists+0.5 ,
                                    2000 , -1. , 1.
                                );

    TH2I *** differenceTOeach = new TH2I**[2] ;
    differenceTOeach[0] = new TH2I*[nHists] ;
    for(unsigned int h=0; h<filesNhists.size(); h++){
        name = histIdentifier.at( h ) ;
        name += "_diffRelative" ;
        differenceTOeach[0][h] = new TH2I(
                                            name , name ,
                                            nHists , 0.5 , nHists+0.5 ,
                                            2000 , -1. , 1.
                                        );
    }

    TH2I ** variation = new TH2I*[2] ;
    variation[0] = new TH2I(
                            "variation_relative" ,
                            "variation_relative" ,
                            nHists , 0.5 , nHists+0.5 ,
                            2000 , 0. , 1.
                        ) ;

    double mean , stdv , minMax[2][2] , content , reference , difference ; 
    bool setMinMax[2] = { true , true } ;

    for(unsigned int r=1; r<=rowsNcolumns[0]; r++){
        for(unsigned int c=1; c<=rowsNcolumns[1]; c++){
            for(unsigned int h=0; h<nHists; h++){
                if( ! useable[h] ) continue ;
                content = hists[h]->GetBinContent( c , r ) ;
                mean = 0. ;
                stdv = 0. ;
                for(unsigned int o=0; o<nHists; o++){
                    if( ! useable[o] ) continue ;
                    if( h == o ) continue ;
                    reference = hists[o]->GetBinContent( c , r ) ;
                    difference = content - reference ;
                    mean += reference ;
                    stdv += ( reference * reference ) ;
                    if( reference == 0. ) reference = 1. ;
                    differenceTOeach[0][h]->Fill( o+1 , difference/abs(reference) ) ;
                    if( setMinMax[0] ){
                        minMax[0][0] = difference ;
                        minMax[0][1] = difference ;
                        setMinMax[0] = false ;
                    }
                    if( minMax[0][0] > difference ) minMax[0][0] = difference ;
                    if( minMax[0][1] < difference ) minMax[0][1] = difference ;
                }
                mean /= ( nToUse - 1. ) ;
                stdv = sqrt( 
                                ( stdv - mean * mean * ( nToUse - 1. ) ) 
                                / 
                                ( ( nToUse - 1. ) -1. ) 
                            ) ; 
                difference = content - mean ;
                if( mean == 0. ) mean = 1. ;
                differenceTOmean[0]->Fill( h+1 , difference/abs(mean) ) ;
                variation[0]->Fill( h+1 , stdv/abs(mean) ) ;
                if( setMinMax[1] ){
                    minMax[1][0] = stdv ;
                    minMax[1][1] = stdv ;
                    setMinMax[1] = false ;
                }
                if( minMax[1][0] > stdv ) minMax[1][0] = stdv ;
                if( minMax[1][1] < stdv ) minMax[1][1] = stdv ;
            }
        }
    }

    cout << " diffrange " << minMax[0][0] << " to " << minMax[0][1] 
         << " STDVrange " << minMax[1][0] << " to " << minMax[1][1] << endl ;

    differenceTOmean[1] = new TH2I( 
                                    "differenceTOmean_absolute" ,
                                    "differenceTOmean_absolute" ,
                                    nHists , 0.5 , nHists+0.5 ,
                                    2000 , minMax[0][0] , minMax[0][1]
                                );

    differenceTOeach[1] = new TH2I*[nHists] ;
    for(unsigned int h=0; h<filesNhists.size(); h++){
        name = histIdentifier.at( h ) ;
        name += "_diffAbsolute" ;
        differenceTOeach[1][h] = new TH2I(
                                            name , name ,
                                            nHists , 0.5 , nHists+0.5 ,
                                            2000 , minMax[0][0] , minMax[0][1]
                                        );
    }

    variation[1] = new TH2I(
                            "variation_absolute" ,
                            "variation_absolute" ,
                            nHists , 0.5 , nHists+0.5 ,
                            2000 , minMax[1][0] , minMax[1][1]
                        ) ;

    for(unsigned int r=1; r<=rowsNcolumns[0]; r++){
        for(unsigned int c=1; c<=rowsNcolumns[1]; c++){
            for(unsigned int h=0; h<nHists; h++){
                if( ! useable[h] ) continue ;
                content = hists[h]->GetBinContent( c , r ) ;
                mean = 0. ;
                stdv = 0. ;
                for(unsigned int o=0; o<nHists; o++){
                    if( ! useable[o] ) continue ;
                    if( h == o ) continue ;
                    reference = hists[o]->GetBinContent( c , r ) ;
                    difference = content - reference ;
                    mean += reference ;
                    stdv += ( reference * reference ) ;
                    if( reference == 0. ) reference = 1. ;
                    differenceTOeach[1][h]->Fill( o+1 , difference ) ;
                }
                mean /= ( nToUse - 1. ) ;
                stdv = sqrt( 
                                ( stdv - mean * mean * ( nToUse - 1. ) ) 
                                / 
                                ( ( nToUse - 1. ) -1. ) 
                            ) ; 
                difference = content - mean ;
                if( mean == 0. ) mean = 1. ;
                differenceTOmean[1]->Fill( h+1 , difference ) ;
                variation[1]->Fill( h+1 , stdv ) ;
            }
        }
    }

    name = outfile->GetName() ;
    name = name.ReplaceAll( ".root" , "" ) ;
    name += "_difference_maps" ;
    TCanvas * can = new TCanvas( name , name ) ;
    can->Divide( nHists , nHists ) ;

    gStyle->SetOptStat(0) ;

    for(unsigned int h=0; h<nHists; h++){
        for(unsigned int o=0; o<nHists; o++){
            if( h == o ) continue ;
            can->cd( h * nHists + o + 1 ) ;
            TH2D * diffHist = (TH2D*)hists[h]->Clone() ;
            name = histIdentifier.at(h) ;
            name += "_minus_" ;
            name += histIdentifier.at(o) ;
            diffHist->SetTitle(name);
            diffHist->SetName(name);
            diffHist->Add( hists[h] , hists[o] , 1. , -1. ) ;
            diffHist->Draw("COLZ");
            diffHist->GetZaxis()->SetRangeUser( minMax[0][0] , minMax[0][1] );
        }
    }

    gPad->Modified() ;
    gPad->Update() ;
    gPad->WaitPrimitive() ;

    name = can->GetName() ;
    name += ".pdf" ;
    can->Print(name) ;

    for(unsigned int h=0; h<nHists; h++) hists[h]->Delete() ;

    cout << " writing ... " ;

    outfile->Write() ;
    outfile->Close() ;

    cout << " done " << endl ;

    return 0 ;

}
