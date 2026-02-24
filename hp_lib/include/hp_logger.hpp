#ifndef HP_LOGGER_H
#define HP_LOGGER_H

#include <iostream>
#include <deque>
#include <memory>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

using namespace std;

class hp_logger {
	public:
		hp_logger(string location = ".", int debug=0) : my_location(location) {
			DIR *dp;
			string path = location + "/logs";
			if((dp = opendir(path.data())) == NULL){
				mkdir(path.data(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			} else {
				closedir(dp);
			}
			my_comm_log = path + "/data.txt";
			my_log_path = path;
			my_debug = debug;
		}
		int log_mess(string mess, string file_name = "", int verbose = 0) {
			if(verbose <= my_verbose){
				string filename = ""; 
				std::ofstream file;
				if(file_name != ""){
					filename = my_log_path+"/"+file_name;
				} else {
					filename =my_comm_log.c_str(); 
				}
				file.open(filename,std::ios::app);
				if (!file.is_open()) {
					std::cout<<"ERROR LOG CANNOT OPEN, file: " << filename<< "\n";
					return -1;
				}
				if(my_debug){
					cout << time_now()<<" "<<mess<<endl;
				}
				if(file_name != ""){
					file<<time(NULL)<<","<<mess<<endl;
				} else {
					file<<time_now()<<" "<<mess<<endl;
				}
				file.close();
			}
			return 0;
		}
	private:
		string my_location;
		string my_comm_log;
		string my_log_path;
		int my_verbose = 0;
		int my_debug;

		std::string time_now() {
			std::stringstream s;
			time_t rawtime;
			struct tm * timeinfo;
			rawtime=time(NULL);
			timeinfo = localtime ( &rawtime );	
			s<<"["<<timeinfo->tm_mday<<"-"<<timeinfo->tm_mon+1<<"-"<<timeinfo->tm_year+1900<<" "<<timeinfo->tm_hour<<":"<<timeinfo->tm_min<<":"<<timeinfo->tm_sec<<"]\t";
			return s.str();
		}
};

#endif
