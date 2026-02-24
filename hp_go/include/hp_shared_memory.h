#ifndef HP_SHARED_MEMORY_H
#define HP_SHARED_MEMORY_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <mysql/mysql.h>
#include <mutex>
#include <cstring>
#include <cstdlib>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace std;
//using namespace boost::interprocess;

#ifndef SHARED_DATA_S
#define SHARED_DATA_S
typedef struct {
	string ident;
	string memory_value;
	int command_type;
	int service_type;
} hp_shared_command_t;
typedef struct {
	vector<hp_shared_command_t> commands;
	mutex mtx;
	bool finish;
} hp_shared_data_t;
#endif

#define SHARED_SLEEP 10000  // us
#define SHARED_PUSH_ALL 0
#define SHARED_PUSH_SINGLE_VALUE 1
#define SHARED_PUSH_COMPLEX_VALUE 2
#define SHARED_EXIT 9

#define SHARED_SERVICE_STATUS 	0
#define SHARED_SERVICE_HEATING	1	
#define SHARED_SERVICE_SECURITY	2
#define SHARED_SERVICE_WATERING	3

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif
class hp_shared_memory {
	public:
		hp_shared_memory(string shared_memory_ident,hp_shared_data_t *data = NULL);
		void operator()();
		bool push_shm_data(string data);
		bool create_shm_object();
		~hp_shared_memory();
	private: 
		string my_shm_ident;
		hp_shared_data_t *my_shared_data;
		void process_push_all(hp_shared_command_t cmd);
		void process_push_single(hp_shared_command_t cmd);
		void process_push_complex(hp_shared_command_t cmd);
};

#endif
