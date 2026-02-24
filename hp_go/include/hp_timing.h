#ifndef HP_TIMING_H
#define HP_TIMING_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include <mutex>
#include <chrono>
#include <thread>

using namespace std;

#ifndef TIMING_DATA_S
#define TIMING_DATA_S
typedef struct {
	mutex mtx;
	int new_sec;
	bool stop_thread;
} hp_timing_t;

#endif

#define TIMING_SLEEP 50000  // us

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif
class hp_timing {
	public:
		hp_timing(hp_timing_t *data);
		void operator()();
		~hp_timing();
	private: 
		hp_timing_t *my_data;
		int my_last_sec;
};

#endif
