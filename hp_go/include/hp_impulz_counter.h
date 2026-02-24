#ifndef HP_IMPULZ_COUNTER_H
#define HP_IMPULZ_COUNTER_H

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <mysql/mysql.h>
#include <time.h>

#ifndef HP_XML_PARSER_H
#include "../../hp_lib/include/hp_xml_parser.h"
#endif

#ifndef HP_DB_HANDLER_H
#include "hp_db_handler.h"
#endif

#ifndef HP_VIRTUAL_PIN_H
#include "hp_virtual_pin.h"
#endif

#define RECORD_ACTUAL 	"0"
#define RECORD_HOUR	"1"
#define RECORD_DAY	"2"
#define RECORD_MONTH	"3"

typedef struct {
	string ident;
	int pos;
	int off_interval;
	time_t on_time;
	string active_value;
	int impulz_count;
	int impulz_len;
} impulz_items_t;

using namespace std;

class hp_impulz_counter 
{
	public:
		hp_impulz_counter(XMLNode node, hp_db_data_t *data = NULL);
		void setup_items_position(const vector<hp_virtual_pin *> pins);
		void process_db(MYSQL *conn);
		void process_pin(int pos, string value);
		void process_data();
		void check_off_interval();
		void print_var();
	//	vector<cond_struc_t> check_condition(int pos, string value);
		~hp_impulz_counter();
	private: 
		hp_xml_parser xml_parser;
		hp_db_data_t *my_db_data;
		impulz_items_t prepare_item(XMLNode);

		vector<impulz_items_t> my_items;

		void push_db_query(string query, int type=DB_STATUSES_QUERY, int log_level=0);
};

#endif
