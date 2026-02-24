#include "../include/hp_timing.h"

hp_timing::hp_timing(hp_timing_t *data)
{
	time_t rawtime;
	struct tm * timeinfo;
	rawtime=time(NULL);
	timeinfo = localtime ( &rawtime );
	my_data= data;
	my_data->new_sec = 0;
	my_data->stop_thread = false;

	my_last_sec = timeinfo->tm_sec;
}

void hp_timing::operator()()
{
	time_t rawtime;
	struct tm * timeinfo;
	while(1){
		rawtime=time(NULL);
		timeinfo = localtime ( &rawtime );
		if(timeinfo->tm_sec != my_last_sec){
			my_last_sec = timeinfo->tm_sec;
			my_data->mtx.lock();
			my_data->new_sec = 1;
			my_data->mtx.unlock();
		}
		if(my_data->stop_thread){
			break;
		}
		std::this_thread::sleep_for(chrono::microseconds(TIMING_SLEEP));
	}
}

hp_timing::~hp_timing() 
{
}
