#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 5 ) return 1 ;

    TString filename      = argv[1] ;
    TString dataname      = argv[2] ;
    TString mode          = argv[3] ;
    TString firstArgument = argv[4] ;
    
    TString fileOpenMode = "UPDATE" ;
    if( gSystem->AccessPathName( filename.Data() ) ){
        fileOpenMode = "RECREATE" ;
        if( mode.Contains("REMOVE") ){
            cout << " ERROR : no file found for removing points " << endl ;
            return 2 ;
        }
        cout << " INFO : creating file " << filename << endl ;
    }
    
    TFile * file = new TFile( filename , fileOpenMode ) ;
    if( file->IsZombie() ){
        cout << " ERROR : opening " << file->GetName() << endl ;
        return 2 ;
    }
    
    if( mode.Contains("HIST") ){
        unsigned int nBins[2] = { 0 , 0 } ;
        double range[2][2] ;
        double value ;
        value = getNumberWithRange( 
                                    firstArgument.Data() ,
                                    range[0][0] , range[0][1]
                                  ) ;
        if( 
            toDiscard( value ) || value < 1.
            || 
            toDiscard( range[0][0] ) || toDiscard( range[0][1] )
        ){
            cout << " ERROR : histogram range ill-defined, " 
                         << " required -> nBins[lowEdge,highEdge] " << endl ;
            file->Close() ;
            return 3 ;
        }
        nBins[0] = (unsigned int)value ;
        if( argc > 5 ){
            value = getNumberWithRange( argv[5] , range[1][0] , range[1][1] ) ;
            if( 
                toDiscard( value ) || value < 1.
                || 
                toDiscard( range[1][0] ) || toDiscard( range[1][1] )
            ){
                cout << " ERROR : histogram range (2.) ill-defined, " 
                     << " required -> nBins[lowEdge,highEdge] " << endl ;
                file->Close() ;
                return 4 ;
            }
            nBins[1] = (unsigned int)value ;
        }
        if( file->Get(dataname) != NULL ){
            cout << " ERROR : '" << dataname << "' already found"
                                 << " in " << filename << endl ;
            file->Close() ;
            return 5 ;
        }
        file->cd() ;
        if( nBins[1] > 0 ){
            TH2D * hist = new TH2D( 
                                    dataname , dataname , 
                                    nBins[0] , range[0][0] , range[0][1] ,
                                    nBins[1] , range[1][0] , range[1][1] 
                                  ) ;
            hist->Write() ;
        }
        else{
            TH1D * hist = new TH1D( 
                                    dataname , dataname , 
                                    nBins[0] , range[0][0] , range[0][1] 
                                  ) ;
            hist->Write() ;
        }
    }
    else if( mode.Contains("FILL") || mode.Contains("WEIGHT") ){
        if( file->Get(dataname) == NULL ){
            cout << " ERROR : reading " << dataname 
                                        << " in " << filename << endl ;
            file->Close() ;
            return 3 ;
        }
        TObject * data = file->Get( dataname ) ;
        TString dataClass = data->ClassName() ;
        TString dimension = dataClass( 2 , 1 ) ;
        if( 
            !( dataClass.BeginsWith( "TH" ) )
            || 
            ( dimension != "1" && dimension != "2" )
        ){
            cout << " ERROR : '" << dataname 
                                 << "' no suitable histogram " << endl ;
            file->Close() ;
            return 4 ;
        }
        if( dimension == "1" ){
            TH1D * hist = (TH1D*)data ;
            double value = atof( firstArgument.Data() ) ;
            if( argc > 5 ){
                double weight = atof( argv[5] ) ;
                if( mode.Contains("WEIGHT") ){
                    unsigned int bin ;
                    if( mode.Contains("BIN") )
                        bin = atoi( firstArgument.Data() ) ;
                    else
                        bin = hist->GetXaxis()->FindBin( value ) ;
                    hist->SetBinContent( bin , weight ) ;
                    if( argc > 6 ){
                        double error = atof( argv[6] ) ;
                        hist->SetBinError( bin , error ) ;
                    }
                }
                else if( mode.Contains("FILL") )
                    hist->Fill( value , weight ) ;
            }
            else{
                if( mode.Contains("WEIGHT") ){
                    cout << " ERROR : no weight specified " << endl ;
                    file->Close() ;
                    return 5 ;
                }
                else if( mode.Contains("FILL") )
                    hist->Fill( value ) ;
            }
            hist->Write() ;
        }
        else if( dimension == "2" ){
            if( argc < 6 ){
                cout << " ERROR : two-dimensional histogram"
                     << " requires additional argument for filling " << endl ;
                file->Close() ;
                return 5 ;
            }
            TH2D * hist = (TH2D*)data ;
            double value[2] = {
                                atof( firstArgument.Data() ) , 
                                atof( argv[5] ) 
                            } ;
            if( argc > 6 ){
                double weight = atof( argv[6] ) ;
                if( mode.Contains("WEIGHT") ){
                    unsigned int bin[2] ;
                    if( mode.Contains("BIN") ){
                        bin[0] = atoi( firstArgument.Data() ) ;
                        bin[1] = atoi( argv[5] ) ;
                    }
                    else{
                        bin[0] = hist->GetXaxis()->FindBin( value[0] ) ;
                        bin[1] = hist->GetYaxis()->FindBin( value[1] ) ;
                    }
                    hist->SetBinContent( bin[0] , bin[1] , weight ) ;
                    if( argc > 7 ){
                        double error = atof( argv[7] ) ;
                        hist->SetBinError( bin[0] , bin[1] , error ) ;
                    }
                }
                else if( mode.Contains("FILL") ){
                    hist->Fill( value[0] , value[1] , weight ) ;
                }
            }
            else{
                if( mode.Contains("WEIGHT") ){
                    cout << " ERROR : no weight specified" 
                                 << " for 2D-histogram " << endl ;
                    file->Close() ;
                    return 6 ;
                }
                else if( mode.Contains("FILL") )
                    hist->Fill( value[0] , value[1] ) ;
            }
            hist->Write() ;
        }
    }
    else if( mode.Contains("POINT") ){
        if( argc < 6 ){
            cout << " ERROR : requires additional argument"
                         << " for adding a point " << endl ;
            file->Close() ;
            return 3 ;
        }
        bool toCreate = false ;
        if( file->Get(dataname) == NULL ){
            cout << " INFO : creating new graph '" << dataname 
                                        << "' in " << filename << endl ;
            toCreate = true ;
        }
        TGraphErrors * graph ;
        bool withErrors[2] = { true , false } ;
        if( toCreate ){
            file->cd() ;
            graph = new TGraphErrors() ;
            graph->SetName(  dataname ) ;
            graph->SetTitle( dataname ) ;
        }
        else{
            TObject * data = file->Get( dataname ) ;
            TString dataClass = data->ClassName() ;
            if( !( dataClass.BeginsWith( "TGraph" ) ) ){
                cout << " ERROR : '" << dataname 
                                     << "' no suitable graph " << endl ;
                file->Close() ;
                return 4 ;
            }
            graph = (TGraphErrors*)data ;
            if( !( dataClass.Contains( "Errors" ) ) ) withErrors[0] = false ;
        }
        unsigned int index = graph->GetN() ;
        double xNy[2] = { atof( argv[4] ) , atof( argv[5] ) } ;
        double errors[2] = { 0. , 0. } ;
        if( mode.Contains("INDEX") ){
            if( argc < 7 ){
                cout << " ERROR : additonal argument required"
                             << " for changing point with index " << endl ;
                file->Close() ;
                return 5 ;
            }
            index  = atoi( argv[4] ) ;
            xNy[0] = atof( argv[5] ) ;
            xNy[1] = atof( argv[6] ) ;
            if( argc == 8 ){
                withErrors[1] = true ;
                errors[1] = atof( argv[7] ) ;
            }
            else if( argc > 8 ){
                withErrors[1] = true ;
                errors[0] = atof( argv[7] ) ;
                errors[1] = atof( argv[8] ) ;
            }
        }
        else{
            if( argc == 7 ){
                withErrors[1] = true ;
                errors[1] = atof( argv[7] ) ;
            }
            else if( argc > 8 ){
                withErrors[1] = true ;
                errors[0] = atof( argv[7] ) ;
                errors[1] = atof( argv[8] ) ;
            }
        }
        graph->SetPoint( index , xNy[0] , xNy[1] ) ;
        if( withErrors[0] && withErrors[1] )
            graph->SetPointError( index , errors[0] , errors[1] ) ;
        file->cd() ;
        graph->Write() ;
    }
    else if( mode.Contains("REMOVE") ){
        if( file->Get(dataname) == NULL ){
            cout << " ERROR : reading " << dataname 
                              << " in " << filename << endl ;
            return 4 ;
        }
        TObject * data = file->Get( dataname ) ;
        TString dataClass = data->ClassName() ;
        if( !( dataClass.BeginsWith( "TGraph" ) ) ){
            cout << " ERROR : '" << dataname 
                                 << "' no suitable graph " << endl ;
            file->Close() ;
            return 5 ;
        }
        TGraphErrors * graph = (TGraphErrors*)data ;
        if( mode.Contains("INDEX") ){
            unsigned int index = atoi( firstArgument.Data() ) ;
            unsigned int nPoints = graph->GetN() ;
            if( index >= nPoints ){
                cout << " ERROR : graph contains only " 
                               << nPoints << " points" << endl ;
                file->Close() ;
                return 6 ;
            }
            graph->RemovePoint( index ) ;
        }
        else if( 
            mode( 6 , 1 ) == "X" 
            || 
            mode( 6 , 1 ) == "Y" 
            || 
            argc > 5
        ){
            bool checkAxis[2] = { true , false } ;
            unsigned int axis = 0 ;
            if( mode( 6 , 1 ) == "Y" ){
                checkAxis[0] = false ;
                checkAxis[1] = true ;
                axis = 1 ;
            }
            else if( argc > 5 ){
                checkAxis[1] = true ;
            }
            SpecifiedNumber toCheck[2][3] ;
            double range[2][2] ;
            double value[2] ;
            value[axis] = getNumberWithRange( 
                                                firstArgument.Data() , 
                                                range[axis][0] , 
                                                range[axis][1] 
                                              ) ;
            if( !( toDiscard( value[axis] ) ) )
                toCheck[axis][0] = SpecifiedNumber( value[axis] ) ;
            if( !( toDiscard( range[axis][0] ) ) )
                toCheck[axis][1] = SpecifiedNumber( range[axis][0] ) ;
            if( !( toDiscard( range[axis][1] ) ) )
                toCheck[axis][2] = SpecifiedNumber( range[axis][1] ) ;
            if( argc > 5 && axis != 1 ){
                value[1] = getNumberWithRange( 
                                                    firstArgument.Data() , 
                                                    range[1][0] , 
                                                    range[1][1] 
                                                ) ;
                if( !( toDiscard( value[1] ) ) )
                    toCheck[1][0] = SpecifiedNumber( value[1] ) ;
                if( !( toDiscard( range[1][0] ) ) )
                    toCheck[1][1] = SpecifiedNumber( range[1][0] ) ;
                if( !( toDiscard( range[1][1] ) ) )
                    toCheck[1][2] = SpecifiedNumber( range[1][1] ) ;
            }
            unsigned int nPoints = graph->GetN() ;
            double xNy[2] ;
            for(unsigned int p=0; p<nPoints; p++){
                graph->GetPoint( p , xNy[0] , xNy[1] ) ;
                for(unsigned int a=0; a<2; a++){
                    if(
                        checkAxis[a]
                        &&
                        (
                            (
                                toCheck[a][0].setting
                                &&
                                toCheck[a][0].number == xNy[a]
                            )
                            ||
                            (
                                ( 
                                    toCheck[a][1].setting 
                                    && 
                                    toCheck[a][2].setting 
                                )
                                &&
                                (
                                    toCheck[a][1].number < xNy[a]
                                    && 
                                    toCheck[a][2].number > xNy[a]
                                )
                            )
                            ||
                            (
                                ( 
                                    toCheck[a][1].setting 
                                    && 
                                    ! toCheck[a][2].setting 
                                )
                                &&
                                toCheck[a][1].number < xNy[a]
                            )
                            ||
                            (
                                ( 
                                    ! toCheck[a][1].setting 
                                    && 
                                    toCheck[a][2].setting 
                                )
                                &&
                                toCheck[a][2].number > xNy[a]
                            )
                        )
                    )
                        graph->RemovePoint( p ) ;
                }
            }
        }
        else{
            cout << " ERROR : specify which removal methode"
                 << " (by INDEX or X- and/or Y-range)" << endl ;
            file->Close() ;
            return 6 ;
        }
        file->cd() ;
        graph->Write() ;
    }
    else{
        cout << " ERROR : no availaible mode specified " << endl ;
        return 3 ;
    }
    
    file->Close() ;

    return 0 ;
    
}