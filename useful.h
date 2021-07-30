#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

std::map< std::string , unsigned int > secondsPER = {
    { ""  ,        1 } ,
    { "s" ,        1 } ,
    { "m" ,       60 } , 
    { "h" ,     3600 } , // 60 * 60
    { "d" ,    86400 } , // 24 * 60 * 60
    { "w" ,   604800 } , // 7 * 24 * 60 * 60
    { "y" , 31536000 }   // 365 * 24 * 60 * 60
} ;

std::vector< std::vector<std::string> > getInput( std::string filename ){
    
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
    
    std::string line = "";
    std::string word = "";
    std::vector<std::string> dummy;
    
    while( getline( ifile, line) ){
        
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