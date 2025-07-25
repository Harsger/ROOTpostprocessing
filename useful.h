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
                                            bool useNonSpaceDelimiter = false ,
                                            std::string delimiter = " "
                                        ){
    
    std::vector< std::vector<std::string> > input;
    
    if( filename.compare("") == 0 ){ 
        std::cout << " WARNING : no input to read from " << std::endl ;
        return input ;
    }
    
    std::ifstream ifile( filename.c_str() ) ;
    if( !( ifile ) ){ 
        std::cout << " WARNING : could not read input file " 
                  << filename << std::endl ;
        return input ;
    }
    
    bool multiCharDelimiter = false ;
    char delimiterChar ;
    unsigned int nDelimiterChars = delimiter.length() ;
    bool emptyDelimiter = false ;
    if( nDelimiterChars > 1 ) multiCharDelimiter = true ;
    else if( nDelimiterChars == 0 ) emptyDelimiter = true ;
    else delimiterChar = delimiter[0] ;
    
    std::string line = "" ;
    std::string word = "" ;
    std::vector<std::string> dummy ;
    std::size_t found ;
    
    while( getline( ifile , line ) ){
        
        std::stringstream sline( line ) ;

        if( useNonSpaceDelimiter ){
            if( multiCharDelimiter ){
                found = line.find( delimiter ) ;
                while( found != std::string::npos ){
                    word = line.substr( 0 , found ) ;
                    if( word != "" ) dummy.push_back( word ) ;
                    line.erase( 0 , found + nDelimiterChars ) ;
                    found = line.find( delimiter ) ;
                }
                if( line != "" ) dummy.push_back( line ) ;
            }
            else if( emptyDelimiter ){
                dummy.push_back( line ) ;
            }
            else{
                while( getline( sline , word , delimiterChar ) ){
                    if( word != "" ) dummy.push_back( word ) ;
                    word = "" ;
                }
            }
        }
        else{
            while( !( sline.eof() ) ){ 
                sline >> std::skipws >> word ;
                if( word != "" ) dummy.push_back( word ) ;
                word = "" ;
            }
        }
        
        if( dummy.size() > 0 ) input.push_back(dummy) ;
        dummy.clear() ;
        
    }
    
    ifile.close() ;
    
    return input ;
    
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
    std::size_t com = s.find(",") ;
    std::size_t ket = s.find("]") ;
    
    if(
        bra != std::string::npos
        &&
        ket != std::string::npos
    ){
        if(
            bra != s.rfind("[")
            ||
            com != s.rfind(",")
            ||
            ket != s.rfind("]")
            ||
            ket != nChars-1
        ){
            low  = nan("") ;
            high = nan("") ;
            return nan("") ;
        }
        if( com != std::string::npos ){
            if( com < bra ){
                low  = nan("") ;
                high = nan("") ;
                return nan("") ;
            }
            if( bra == 0 )
                value = nan("") ;
            else
                value = atof( s.substr( 0 , bra ).c_str() ) ;
            if( com-1 == bra )
                low = nan("") ;
            else
                low = atof( s.substr( bra+1 , com-(bra+1) ).c_str() ) ;
            if( com+1 == ket )
                high = nan("") ;
            else
                high = atof( s.substr( com+1 , ket-(com+1) ).c_str() ) ;
        }
        else{
            if( bra == 0 ){
                if( bra+1 == ket ){
                    low  = nan("") ;
                    high = nan("") ;
                    return nan("") ;
                }
                else{
                    value = atof( s.substr( bra+1 , ket-1 ).c_str() ) ;
                    low   = value ;
                    high  = value ;
                }
            }
            else{
                value = atof( s.substr( 0 , bra ).c_str() ) ;
                if( bra+1 == ket )
                    low = nan("") ;
                else
                    low = atof( s.substr( bra+1 , ket-1 ).c_str() ) ;
                high = low ;
            }
        }
//         if( low > high ) std::swap( low , high ) ;
    }
    else if(
        bra == std::string::npos
        &&
        com == std::string::npos
        &&
        ket == std::string::npos
    ){
        value = atof( s.c_str() ) ;
        low   = nan("") ;
        high  = nan("") ;
    }
    else{
        low  = nan("") ;
        high = nan("") ;
        return nan("") ;
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

class Extremum{
public :

    bool         high     ;
    bool         set      ;
    double       value    ;
    double       position ;
    unsigned int index    ;

    Extremum(){
        high = true ;
        set  = false ;
    }

    Extremum( bool h ){
        high = h ;
        set  = false ;
    }

    Extremum( std::string s ){
        if(      s == "max" || s == "high" ){ high = true  ; }
        else if( s == "min" || s == "low"  ){ high = false ; }
        else{                                 high = true  ; }
        set = false ;
    }

    Extremum( unsigned int h ){
        if(      h == 0 ){ high = false ; }
        else if( h == 1 ){ high = true  ; }
        else{              high = true  ; }
        set = false ;
    }

    Extremum( bool h , double v , double p , unsigned int i ){
        high     = h ;
        value    = v ;
        position = p ;
        index    = i ;
        set = true ;
    }

    void put( double v , double p , unsigned int i ){
        value    = v ;
        position = p ;
        index    = i ;
        set = true ;
    }

    bool overwrite( double v , double p , unsigned int i ){
        if( !( set ) ){
            this->put( v , p , i ) ;
            return true ;
        }
        if( high ){
            if( value < v ){
                value    = v ;
                position = p ;
                index    = i ;
                set = true ;
                return true ;
            }
        }
        else{
            if( value > v ){
                value    = v ;
                position = p ;
                index    = i ;
                set = true ;
                return true ;
            }
        }
        return false ;
    }

} ;

class Statistics{
public :

    double       mean   ;
    double       stdv   ;
    double       min    ;
    double       max    ;
    unsigned int number ;

    double       accumulator ;
    double       quadratic   ;

    bool         set   ;
    bool         ready ;

    Statistics(){
        mean        = 0. ;
        stdv        = 0. ;
        min         = 0. ;
        max         = 0. ;
        number      = 0  ;
        accumulator = 0. ;
        quadratic   = 0. ;
        set   = false ;
        ready = false ;
    }

    Statistics( double v ){
        mean        = 0. ;
        stdv        = 0. ;
        min         = v ;
        max         = v ;
        number      = 1 ;
        accumulator = v ;
        quadratic   = v * v ;
        set   = true  ;
        ready = false ;
    }

    Statistics( double m , double s , double l , double h , unsigned int n ){
        mean        = m  ;
        stdv        = s  ;
        min         = l  ;
        max         = h  ;
        number      = n  ;
        accumulator = 0. ;
        quadratic   = 0. ;
        set   = false ;
        ready = true  ;
    }

    double fill( double v ){
        if( !( this->set ) ){
            *this = Statistics( v ) ;
            return v ;
        }
        number++ ;
        accumulator +=   v       ;
        quadratic   += ( v * v ) ;
        double resetExtremum = 0. ;
        if( min > v ){
            resetExtremum = v - min ;
            min = v ;
        }
        if( max < v ){
            resetExtremum = v - max ;
            max = v ;
        }
        return resetExtremum ;
    }

    bool calculate(){
        if( number < 1 ) return false ;
        mean = accumulator / (double)number ;
        if( number < 2 ){
            stdv = 0. ;
            return true ;
        }
        stdv = sqrt(
                        ( quadratic - mean * mean * (double)number )
                        /
                        ( (double)number - 1. )
                    ) ;
        return true ;
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

//std::vector<unsigned int> getSortedIndices(std::vector<double> order){
//   
//    std::vector<unsigned int> sorted;
//    unsigned int nPoints = order.size();
//    double lower = 0;
//    double lowest = 0;
//    unsigned int index = 0;
//    
//    if( nPoints < 1 ) return sorted;
//
//    for(unsigned int l=0; l<nPoints; l++){
//
//        if( sorted.size() < 1 ) lowest = order.at(0);
//        else{ 
//            lower = order.at( sorted.at(l-1) );
//            unsigned int newone = 0;
//            bool found = false;
//            for(unsigned int p=0; p<nPoints; p++){
//                bool inlist = false;
//                for(unsigned int s=0; s<sorted.size(); s++){
//                    if( sorted.at(s) == p ){ 
//                        inlist = true;
//                        break;
//                    }
//                }
//                if( !inlist){ 
//                    newone = p;
//                    found = true;
//                    break;
//                }
//            }
//            if( !found ){
//                std::cout << " WARNING : no index found " << std::endl;
//                break;
//            }
//            else{ 
//                lowest = order.at(newone);
//                index = newone;
//            }
//        }
//
//        for(unsigned int p=0; p<nPoints; p++){
//
//            if( sorted.size() < 1 && order.at(p) < lowest ){
//                lowest = order.at(p);
//                index = p;
//            }
//            if( order.at(p) < lowest && order.at(p) > lower  ){ 
//                index = p;
//                lowest = order.at(p);
//            }
//
//        }
//
//        sorted.push_back( index );
//
//    }
//    
//    return sorted;
//  
//}

std::vector<unsigned int> getSortedIndices( std::vector<double> data ){

    std::vector<unsigned int> sorted ;
    unsigned int n = data.size() ; 

    if( n < 1 ) return sorted;

    std::vector<bool> assigned( n , false );
    double lowest = data.at(0) ;
    unsigned int index = 0 ;

    for(unsigned int l=0; l<n; l++){

        bool found = false;

        for(unsigned int p=0; p<n; p++){
            if( assigned.at(p) ) continue ;
            index = p ;
            lowest = data.at(p) ;
            found = true ;
            break ;
        }

        if( !found ){
            std::cout << " WARNING : no next index found " << std::endl;
            break;
        }

        for(unsigned int p=0; p<n; p++){
            if( assigned.at(p) ) continue ;
            if( data[p] < lowest ){
                index = p;
                lowest = data[p];
            }
        }

        sorted.push_back( index );
        assigned.at( index ) = true ;
    }

    return sorted;

}

void splitFilename(
                    std::string fullname ,
                    std::string &path ,
                    std::string &file
){
    size_t found = fullname.rfind( "/" ) ;
    if( found != std::string::npos ){
        path = fullname.substr( 0 , found ) ;
        file = fullname.substr( found + 1 ) ;
    }
    else{
        path = "" ;
        file = fullname ;
    }
}

void printUsage( char argv0[] ){
    std::string executable = argv0 ;
    std::string path , file ;
    splitFilename( executable , path , file ) ;
    std::string readmePosition = path ;
    if( file == executable ) readmePosition += "." ;
    readmePosition += "/README.txt" ;
    std::ifstream readmeFile( readmePosition.c_str() ) ;
    if( !( readmeFile ) ){
        std::cout << " ERROR : can not open README.txt " << std::endl ;
        return ;
    }
    bool checkFile[2] = { false , false } ;
    bool toPrint = false ;
    std::string line ;
    while( getline( readmeFile , line ) ){
        if(
            line.length() > 70
            &&
            line.find("///") == 0
            &&
            line.rfind("///") == line.length()-3
        ){
            if( toPrint ){
                break ;
            }
            else{
                checkFile[0] = true ;
                continue ;
            }
        }
        if( checkFile[0] ){
            if( checkFile[1] ){
                if( line.find( file ) == 0 ){
                    toPrint = true ;
                    std::cout << std::endl ;
                }
                else{
                    checkFile[0] = false ;
                    checkFile[1] = false ;
                    continue ;
                }
            }
            else{
                checkFile[1] = true ;
                continue ;
            }
        }
        if( toPrint ){
            std::cout << line << std::endl ;
        }
    }
    readmeFile.close() ;
    if( !( toPrint ) ){
        std::cout << " ERROR : no entry for executable found " << std::endl ;
    }
}
