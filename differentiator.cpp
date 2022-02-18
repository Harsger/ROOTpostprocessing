#include "postprocessing.h"

using namespace std;

int main(int argc, char *argv[]){

    if( argc < 3 ) return 1 ;

    TString filename = argv[1] ;
    TString dataname = argv[2] ;
    
//     bool graphData = false ;
//     bool histData  = false ;
    
    TFile * input = new TFile(filename,"READ") ;
    if( input->IsZombie() ){
        cout << " ERROR : opening " << input->GetName() << endl ;
        return 2 ;
    }
    
    if( input->Get(dataname) == NULL ){
        cout << " ERROR : reading " << dataname 
                          << " in " << input->GetName() << endl ;
        return 3 ;
    }
    
    TString dataClass =  input->Get(dataname)->ClassName() ;
    
    TGraphErrors * differential = new TGraphErrors() ;
    differential->SetName(  "differential" ) ;
    differential->SetTitle( "differential" ) ;
    TGraphErrors * weightedDifferential = new TGraphErrors() ;
    weightedDifferential->SetName(  "weightedDifferential" ) ;
    weightedDifferential->SetTitle( "weightedDifferential" ) ;
    TGraphErrors * integral = new TGraphErrors() ;
    integral->SetName(  "integral" ) ;
    integral->SetTitle( "integral" ) ;
    TGraphErrors * weightedIntegral = new TGraphErrors() ;
    weightedIntegral->SetName(  "weightedIntegral" ) ;
    weightedIntegral->SetTitle( "weightedIntegral" ) ;
    
    if( dataClass.Contains("Graph") ){
        
        TGraphErrors * graph = (TGraphErrors*)input->Get(dataname) ;
        unsigned int nPoints = graph->GetN() ;
        double value[2] , position[2] ;
        vector<double> xvalues ;
        
        bool ascending = false ;
        bool descending = false ;
        
        bool unusableData = false ;
        bool toBeSorted = false ;
        
        for(unsigned int p=0; p<nPoints; p++){
            graph->GetPoint( p , position[0] , value[0] ) ;
            xvalues.push_back( position[0] ) ;
            if( p > 0 ){
                if( position[0] == position[1] ){
                    unusableData = true ;
                    break ;
                }
                if     ( position[0] > position[1] ) ascending  = true ;
                else if( position[0] < position[1] ) descending = true ;
                if( ascending && descending ) toBeSorted = true ;
            }
            position[1] = position[0] ;
        }
        
        if( unusableData ){
            cout << " ERROR : " 
                 << " data can not be processed due overlapping " << endl ;
            return 4 ;
        }
        
        vector<unsigned int> order ;
        if( toBeSorted )
            order = getSortedIndices( xvalues ) ;
        
        unsigned int index ;
        double accumulated = 0. , weightedAccumulated = 0. ;
        double difference , distance , center ;
        
        for(unsigned int p=0; p<nPoints; p++){
            
            if( toBeSorted ) index = order.at( p ) ;
            else{
                if( descending ) index = nPoints - p - 1 ;
                else index = p ;
            }
            
            graph->GetPoint( index , position[0] , value[0] ) ;
            
            accumulated += value[0] ;
            
            integral->SetPoint( 
                                integral->GetN() ,  
                                position[0] ,
                                accumulated
                              ) ;
            
            if( p > 0 ){
                
                center     = 0.5 * ( position[0] + position[1] ) ;
                difference = value[0]    - value[1] ;
                distance   = position[0] - position[1] ;
                
                differential->SetPoint(
                                        differential->GetN() ,
                                        center ,
                                        difference 
                                      ) ;
                
                if( distance > 0. )
                    weightedDifferential->SetPoint(
                                            weightedDifferential->GetN() ,
                                            center ,
                                            difference / distance
                                        ) ;
                
            }
            
            position[1] = position[0] ;
            value[1]    = value[0] ;
            
        }
        
    }
    else if( dataClass.Contains("TH1") ){
        
        TH1D * hist = (TH1D*)input->Get(dataname) ;
        unsigned int nbins = hist->GetNbinsX() ; 
        double value[2] , lowEdge[2] , width[2] ;
        double accumulated = 0. , weightedAccumulated = 0. ;
        double difference , position , distance ;
        
        for(unsigned int b=1; b<=nbins; b++){
            
            value[0]   = hist->GetBinContent( b ) ;
            lowEdge[0] = hist->GetBinLowEdge( b ) ;
            width[0]   = hist->GetBinWidth(   b ) ;
            
            accumulated         +=   value[0] ;
            weightedAccumulated += ( value[0] * width[0] ) ;
            
            integral->SetPoint( 
                                integral->GetN() ,  
                                lowEdge[0] + width[0] ,
                                accumulated
                              ) ;
                              
            weightedIntegral->SetPoint( 
                                weightedIntegral->GetN() ,  
                                lowEdge[0] + width[0] ,
                                weightedAccumulated
                              ) ;
            
            if( b > 1 ){
                
                difference = value[0] - value[1] ;
                position = ( 
                                ( lowEdge[1] + 0.5 * width[1] ) * width[1] 
                                +  
                                ( lowEdge[0] + 0.5 * width[0] ) * width[0] 
                           ) 
                           / 
                           ( width[1] + width[0] ) ;
                           
                distance = 
                            lowEdge[0] + 0.5 * width[0] 
                            - 
                            ( lowEdge[1] + 0.5 * width[1] ) ;
                           
                differential->SetPoint(
                                        differential->GetN() ,
                                        position ,
                                        difference
                                      ) ;
                
                if( distance > 0. )
                    weightedDifferential->SetPoint(
                                        weightedDifferential->GetN() ,
                                        position ,
                                        difference / distance
                                    ) ;
                
            }
            
            value[1]   = value[0] ;
            lowEdge[1] = lowEdge[0] ;
            width[1]   = width[0] ;
            
        }
        
    }
    else{
        cout << " ERROR : no suitable data found " << endl ;
        return 6 ;
    }    
    
    TString name = filename ;
    if( name.Contains(".") ) 
        name = name( 0 , name.Last('.') ) ;
    if( name.Contains("/") ) 
        name = name( name.Last('/')+1 , name.Sizeof() ) ;
    name += "_" ;
    name += dataname ;
    name = replaceBadChars( name );
    name += "_differentials.root" ;
    
    TFile * outfile = new TFile( name , "RECREATE" ) ;
    
    outfile->cd() ;
    
    differential->Write() ;
    weightedDifferential->Write() ;
    integral->Write() ;
    if( dataClass.Contains("TH1") ) weightedIntegral->Write() ;
    else weightedIntegral->Delete() ;
    
    outfile->Close() ;

    return 0 ;
    
}