#ifndef HP_DEFAULTS_H
#define HP_DEFAULTS_H

#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <memory>
#include <iterator>
#include <string>
#include "xmlParser.h"
#include <iomanip>
#include <stdio.h>
#include <mutex>

#define CHECK() fprintf(stderr, "%s:%d:check\n", __FILE__, __LINE__)
#define LOG_CH() patch::to_string(__FILE__)+":"+ patch::to_string(__LINE__) 
#define BASE_DEC 0
#define BASE_HEX 1

#define DEBUG 1

#ifndef HP_GUI_DATA_S
#define HP_GUI_DATA_S
typedef struct {
	std::deque<std::string> data;
	std::mutex mtx;
	bool finish = false;
} hp_gui_data_t;
#endif

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
	stm.setf( std::ios::fixed, std:: ios::floatfield );
        stm << std::setprecision(2) << n ;
        return stm.str() ;
    }
    template< typename T> std::string trim(const T& str) {
	    size_t first = str.find_first_not_of(' ');
	    if (first == std::string::npos)
	        return "";
	    size_t last = str.find_last_not_of(' ');
	    return str.substr(first, (last-first+1));
    }
    template < typename T> int string2int(const T& str,int base = BASE_DEC)
    {
	int res = -1;
	int valid = -1;
	if(base == BASE_DEC) {
		valid = sscanf(str.c_str(),"%d",&res);
	} else if (base == BASE_HEX){
		valid = sscanf(str.c_str(),"%x",&res);
	} else {
		valid = EOF;
	}
	if(valid == EOF){
		res = -1;
	}
	return res;
    }
	template < typename T> float string2float(const T& str) 
		{
		try {
			std::string::size_type sz;
			float f = std::stof (str,&sz);
			return f;
		} catch(...) {
			return -1;
		}
	}
    inline  std::vector<std::string> split (const std::string &s, char delim = '_') {
	    std::vector<std::string> result;
	    std::stringstream ss (s);
	    std::string item;
	
	    while (getline (ss, item, delim)) {
	        result.push_back (item);
	    }
	
	    return result;
    }

}

#endif
