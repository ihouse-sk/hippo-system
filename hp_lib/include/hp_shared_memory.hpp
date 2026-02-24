#ifndef HP_SHARED_MEMORY_H
#define HP_SHARED_MEMORY_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <jsoncpp/json/json.h>

using namespace std;
//using namespace boost::interprocess;

class hp_shared_memory {
	public:
		hp_shared_memory(string shared_memory_ident, unsigned int mem_size = 100000) : my_shm_ident(shared_memory_ident), my_mem_size(mem_size) {
			try {
				boost::interprocess::shared_memory_object::remove(shared_memory_ident.c_str());
				boost::interprocess::shared_memory_object shm(boost::interprocess::create_only, my_shm_ident.c_str(), boost::interprocess::read_write);
				shm.truncate(my_mem_size);
			} catch (std::exception const&  ex) {
				cout << "fatal error : " << ex.what() << endl;
			}
		}
		bool push_shm_data(Json::Value val) {
			Json::FastWriter wr;
			string data = wr.write(val);
			boost::interprocess::shared_memory_object shm(boost::interprocess::open_only, my_shm_ident.c_str(), boost::interprocess::read_write);
			boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
			//std::memset(region.get_address(), '_', region.get_size());
		
			if(region.get_size() > data.length()){
				char *mem = static_cast<char*>(region.get_address());
				for(std::size_t i = 0; i < data.length(); ++i){
					*mem++ =data.c_str()[i];
			      	}
				for(std::size_t i = data.length(); i < region.get_size(); ++i){
					*mem++ = 0x00;
				}
			} else {
				return false;
			}
		
			return true;
		}
		~hp_shared_memory(){};
	private: 
		string my_shm_ident;
		unsigned int my_mem_size;
};

#endif
