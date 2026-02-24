#ifndef HP_WATERING_ZONE_H
#define HP_WATERING_ZONE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>


#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#define WATERING_DAY_COUNT 4

#define ZONE_SINGLE	0
#define ZONE_ALL	1

using namespace std;

class hp_watering_zone {
	public:
		hp_watering_zone(XMLNode);
		hp_watering_zone(string ident);
		string get_id() { return my_ident;}
		bool is_start_time(int now);
		bool is_running() { return my_running;}
		bool is_end_time( int now);
		int get_auto_enabled() { return my_auto_enabled;}
		int get_zone_type() { return my_zone_type; }
		vector<string> get_actors() { return my_actors; }
		int get_time_period() { return my_timer; }
		void set_running(int running) { my_running = running;}
		void set_auto_enabled(int auto_enabled) { my_auto_enabled = auto_enabled; }
		void set_timer(int timer) { my_timer = timer; }
		void set_start_time(int value, int pos);
		void set_time_enabled(int value, int pos);
		void set_week_day_enabled(int value, int pos);
		void setup_zone(int running, int auto_enabled, int timer, vector<int> start_time, vector<int> time_enabled, vector<int> week_day_enabled);
		const vector<int>get_start_times() { return my_start_time;}
		const vector<int>get_time_enabled(){ return my_time_enabled; }
		const vector<int>get_week_day_enabled() { return my_week_day_enabled;}
		~hp_watering_zone();
	private: 
		hp_xml_parser xml_parser;
		string my_ident;
		vector<string> my_actors;
		int my_running;
		int my_auto_enabled;
		int my_timer;
		vector<int> my_start_time;
		vector<int> my_time_enabled;
		vector<int> my_week_day_enabled;
		bool my_status;
		int my_zone_type;
};

#endif
