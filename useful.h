#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <map>

std::map< std::string , unsigned int > secondsPER = {
    { ""  ,        1 } ,
    { "s" ,        1 } ,
    { "m" ,       60 } , 
    { "h" ,     3600 } , // 60 * 60
    { "d" ,    86400 } , // 24 * 60 * 60
    { "w" ,   604800 } , // 7 * 24 * 60 * 60
    { "y" , 31536000 }   // 365 * 24 * 60 * 60
} ;

std::vector< std::vector<std::string> > getInput( 
                                                    std::string filename ,
                                                    std::string delimiter = " "
                                                ){
    
    std::vector< std::vector<std::string> > input;
    
    if( filename.compare("") == 0 ){ 
        std::cout << " WARNING : no input to read from " << std::endl;
        return input;
    }
    
    std::ifstream ifile(filename.c_str());
    if( !( ifile ) ){ 
        std::cout << " WARNING : could not read input file " 
                  << filename << std::endl;
        return input;
    }
    
    bool swapDelimiter = false ;
    if( delimiter != " " ) swapDelimiter = true ;
    
    std::string line = "";
    std::string word = "";
    std::vector<std::string> dummy;
    std::size_t found ;
    
    while( getline( ifile, line) ){
        
        if(swapDelimiter){
            found = line.find( delimiter ) ;
            while( found != std::string::npos ){
                line.replace( found , delimiter.length() , " " ) ;
                found = line.find( delimiter ) ;
            }
        }
        
        std::stringstream sline(line);
        
        while( !( sline.eof() ) ){ 
            
            sline >> std::skipws >> word;
            if( word != "" ) dummy.push_back(word);
            word="";
            
        }
        
        if( dummy.size() > 0 ) input.push_back(dummy);
        dummy.clear();
    }
    
    ifile.close();
    
    return input;
    
}

bool toDiscard( double d ){
    if( (d==0.) && ((d+1)!=0.) ) return false ;
    else return ( !( std::isnormal( d ) ) ) ;
}

double getNumberWithUnit( std::string s , std::string &unit ){
    
    unit = "" ;

    if( s.length() < 1 ) return nan("") ;

    std::string worker = s ;
    char tester = s[ s.length() - 1 ] ;
    
    while(
        !( 
            isdigit( tester ) 
            ||
            tester == '.'
        )
    ){
        
        unit = tester + unit ;
        worker.resize( worker.length() - 1 ) ;

        if( worker.length() < 1 ) break ;

        tester = worker[ worker.length() - 1  ] ;
    
    }    

    if( worker.length() < 1 ) return nan("") ;

    return atof( worker.c_str() ) ;
    
}

double getNumberWithRange( std::string s , double &low , double &high ){
    
    unsigned int nChars = s.length() ;
    
    if( nChars < 1 ){ 
        low  = nan("") ;
        high = nan("") ;
        return nan("") ;
    }
    
    double value ;
    
    std::size_t bra = s.find("[") ;
    
    if( bra != std::string::npos && s[ nChars-1 ] == ']' ){
        std::size_t comma = s.find(",") ;
        if( bra == 0 ){
            if( comma != std::string::npos ){
                value = nan("") ;
                low   = atof( s.substr( 1       , comma-1        ).c_str() ) ;
                high  = atof( s.substr( comma+1 , nChars-comma-1 ).c_str() ) ;
            }
            else{
                value = atof( s.substr( 1       , nChars-1       ).c_str() ) ;
                low   = value ;
                high  = value ;
            }
        }
        else{
            value = atof( s.substr( 0 , bra ).c_str() ) ;
            if( comma != std::string::npos && comma > bra ){
                low   = atof( s.substr( bra+1   , comma-bra-1    ).c_str() ) ;
                high  = atof( s.substr( comma+1 , nChars-comma-1 ).c_str() ) ;
            }
            else if( nChars-1 == bra+1 ){
                low   = nan("") ;
                high  = nan("") ;
            }
            else{
                low   = atof( s.substr( bra+1   , nChars-1       ).c_str() ) ;
                high  = low ;
            }
        }
        if( low > high ) std::swap( low , high ) ;
    }
    else{
        value = atof( s.c_str() ) ;
        low   = nan("") ;
        high  = nan("") ;
    }
    
    return value ;
    
}

bool containsNonCommonCharacters( std::string word ){
    bool specialFound = false ;
    for( auto c : word ){
        if( 
            !( isalpha( c ) )
            &&
            !( isdigit( c ) )
            &&
            c != '_'
        ){
            specialFound = true ;
            break ;
        }
    }
    return specialFound ;
}

class SpecifiedNumber{
    
    public :
        
        double number ;
        std::string specifier ;
        bool setting ;
        
        SpecifiedNumber(){
            number = 0. ;
            specifier = "" ;
            setting = false ;
        }
        
        SpecifiedNumber( double d ){
            number = d ;
            specifier = "" ;
            setting = true ;
        }
        
} ;

bool getStats(
    std::vector<double> * values ,
    double &mean ,
    double &stdv ,
    double &min ,
    double &max ,
    double &median 
){
    
    unsigned int nValues = values->size() ;
    
    if( nValues < 1 ){ 
        std::cout << " WARNING : vectorSize zero " << std::endl;
        return false ;
    }
                
    std::vector<double> vecCopy = *values ;
    
    mean = 0. ;
    stdv = 0. ;
    min = vecCopy.at(0) ;
    max = vecCopy.at(0) ;
    
    for( auto v : vecCopy ){
        mean += v ;
        stdv += ( v * v ) ;
        if( min > v ) min = v ;
        if( max < v ) max = v ;
    }
    
    mean /= (double)nValues ;
    if( nValues == 1 ) stdv = 0. ;
    else
        stdv = sqrt( 
                        ( stdv - mean * mean * (double)nValues ) 
                        / 
                        ( (double)nValues - 1. ) 
                ) ;
    
    nth_element( 
                    vecCopy.begin() , 
                    vecCopy.begin() + nValues/2 ,
                    vecCopy.end()
               ) ;
               
    if( nValues % 2 == 0 )
        median = 0.5 * ( vecCopy[nValues/2] + vecCopy[nValues/2 - 1] ) ;
    else
        median = vecCopy[nValues/2] ;
        
                
    return true ;
    
}

double interpolate(
    double x1 ,
    double y1 ,
    double x2 ,
    double y2 ,
    double v
){
    
    if( x1 == x2 ) return x1 ;
    
    double slope = ( y2 - y1 ) / ( x2 - x1 ) ;
    double intercept = ( y1 * x2 - y2 * x1 ) / ( x2 - x1 ) ;
    
    return slope * v + intercept ;
    
}

int getPower( double value ){
    if( value == 0. ) return 0 ;
    return (int)std::floor( std::log10( std::fabs( value ) ) ) ;
}

bool almostEqual( double a , double b ){
    return std::abs( ( a - b ) / 0.5 / ( a + b )  ) < 1e-8 ;
}

// double approach( double limit , double start , double step , bool higher ){
//     
//     step = std::abs( step ) ;
//     
//     double value = start ;
//     
//     if( higher ){
//         
//         if( almostEqual( start , limit ) )
//             value += step ;
//         else if( start > limit ){
//             while( 
//                 value - step > limit 
//                 &&
//                 ! almostEqual( value - step , limit )
//             ){
//                 value -= step ;
//             }
//         }
//         else{ 
//             while( 
//                 value + step < limit 
//                 ||
//                 almostEqual( value + step , limit )
//             ){
//                 value += step ;
//             }
//             value += step ;
//         }
//         
//     }
//     else{
//         
//         if( almostEqual( start , limit ) )
//             value -= step ;
//         else if( start > limit ){
//             while( 
//                 value - step > limit 
//                 ||
//                 almostEqual( value - step , limit )
//             ){
//                 value -= step ;
//             }
//             value -= step ;
//         }
//         else{ 
//             while( 
//                 value + step < limit 
//                 &&
//                 ! almostEqual( value + step , limit )
//             ){
//                 value += step ;
//             }
//         }
//         
//     }
//     
//     return value ;
//     
// }

double approach( double limit , double start , double step , bool higher ){
    
    step = std::abs( step ) ;
    
    double value = start ;
    
    if( almostEqual( start , limit ) ){
        if( higher ) value += step ;
        else         value -= step ;
    }
    else{
        
        int sign = 1 ;
        if( start > limit ) sign = -1 ;
        
        double difference = std::abs( value - limit ) ;
        
        while( 
            difference > step 
            || 
            almostEqual( difference , step )
        ){
            value += ( sign * step ) ;
            difference = std::abs( value - limit ) ;
        }
        
        sign = -1 ;
        if( higher ) sign = 1 ;
        
        if( almostEqual( value , limit ) )
            value += ( sign * step ) ;
        
        if( sign * value < sign * limit ) 
            value += ( sign * step ) ;
        
    }
    
    return value ;
    
}

void getLimits( double min , double max , double &low , double &high ){
    
    double difference = max - min ;
    
    low = min ;
    high = max ;
    
    int secondDigit = getPower( difference ) - 1 ;
    double step = std::pow( 10. , secondDigit ) ;
    
    if( almostEqual( min , max ) ){
        secondDigit = getPower( min ) - 1 ;
        step = std::pow( 10. , secondDigit ) ;
        low  = min - step ;
        high = max + step ;
        return ;
    }
    
    std::stringstream numberStream ;
    unsigned int highPrecision = 2 ;
    
    if( min >= 0. ){
    
        if( min * 10. < difference ){ 
            low = 0. ;
            highPrecision = 1 ;
            secondDigit++ ;
            step = std::pow( 10. , secondDigit ) ;
        }
        else{
            numberStream << std::setprecision(2) << min ;
            numberStream >> low ;
            low -= step ;
            low  = approach( min , low  , step , false ) ;
        }
        
        numberStream.clear() ;
        numberStream.str(std::string()) ;
        numberStream << std::setprecision(highPrecision) << max ;
        numberStream >> high ;
        high += step ;
        high = approach( max , high , step , true ) ;
        
    }
    else if( max <= 0. ){
    
        if( abs( max ) * 10. < difference ){ 
            high = 0. ;
            highPrecision = 1 ;
            secondDigit++ ;
            step = std::pow( 10. , secondDigit ) ;
        }
        else{
            numberStream << std::setprecision(2) << max ;
            numberStream >> high ;
            high += step ;
            high  = approach( max , high  , step , true ) ;
        }
        
        numberStream.clear() ;
        numberStream.str(std::string()) ;
        numberStream << std::setprecision(highPrecision) << min ;
        numberStream >> low ;
        low -= step ;
        low  = approach( min , low , step , false ) ;
    }
    
}

std::vector<unsigned int> getSortedIndices(std::vector<double> order){
   
    std::vector<unsigned int> sorted;
    unsigned int nPoints = order.size();
    double lower = 0;
    double lowest = 0;
    unsigned int index = 0;
    
    if( nPoints < 1 ) return sorted;

    for(unsigned int l=0; l<nPoints; l++){

        if( sorted.size() < 1 ) lowest = order.at(0);
        else{ 
            lower = order.at( sorted.at(l-1) );
            unsigned int newone = 0;
            bool found = false;
            for(unsigned int p=0; p<nPoints; p++){
                bool inlist = false;
                for(unsigned int s=0; s<sorted.size(); s++){
                    if( sorted.at(s) == p ){ 
                        inlist = true;
                        break;
                    }
                }
                if( !inlist){ 
                    newone = p;
                    found = true;
                    break;
                }
            }
            if( !found ){
                std::cout << " WARNING : no index found " << std::endl;
                break;
            }
            else{ 
                lowest = order.at(newone);
                index = newone;
            }
        }

        for(unsigned int p=0; p<nPoints; p++){

            if( sorted.size() < 1 && order.at(p) < lowest ){
                lowest = order.at(p);
                index = p;
            }
            if( order.at(p) < lowest && order.at(p) > lower  ){ 
                index = p;
                lowest = order.at(p);
            }

        }

        sorted.push_back( index );

    }
    
    return sorted;
  
}
