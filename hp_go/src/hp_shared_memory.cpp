#include "../include/hp_shared_memory.h"

hp_shared_memory::hp_shared_memory(string shared_memory_ident,hp_shared_data_t *data) : my_shm_ident(shared_memory_ident), my_shared_data(data)
{
	boost::interprocess::shared_memory_object::remove(shared_memory_ident.c_str());

	//shared_memory_object shm(create_only, shared_memory_ident.c_str(), read_write);
}

bool hp_shared_memory::create_shm_object()
{
	boost::interprocess::shared_memory_object shm(boost::interprocess::create_only, my_shm_ident.c_str(), boost::interprocess::read_write);
	shm.truncate(100000);
	return true;
}

bool hp_shared_memory::push_shm_data(string data)
{
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

void hp_shared_memory::operator()()
{
	bool finish = false;
	boost::interprocess::shared_memory_object shm(boost::interprocess::create_only, "hippoShare", boost::interprocess::read_write);
	shm.truncate(10000);
	while(1){
		while(my_shared_data->commands.size() >0){
			hp_shared_command_t tmp = my_shared_data->commands[0];
			if(tmp.command_type == SHARED_PUSH_ALL){
				process_push_all(tmp);
			} else if(tmp.command_type == SHARED_PUSH_SINGLE_VALUE){
				process_push_single(tmp);
			} else if(tmp.command_type == SHARED_PUSH_COMPLEX_VALUE){
				process_push_complex(tmp);
			} else if(tmp.command_type == SHARED_EXIT){
				finish =true;
				break;
			}
			my_shared_data->mtx.lock();
			my_shared_data->commands.erase(my_shared_data->commands.begin());
			my_shared_data->mtx.unlock();
		}
		if(finish){
			break;
		}
		usleep(SHARED_SLEEP);
	}
}

void hp_shared_memory::process_push_all(hp_shared_command_t cmd)
{
	boost::interprocess::shared_memory_object shm(boost::interprocess::open_only, my_shm_ident.c_str(), boost::interprocess::read_write);
	boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
//	std::memset(region.get_address(), '', region.get_size());

	char *mem = static_cast<char*>(region.get_address());
	for(std::size_t i = 0; i < cmd.memory_value.length(); ++i){
		*mem++ = cmd.memory_value.c_str()[i];
      	}
}
void hp_shared_memory::process_push_single(hp_shared_command_t cmd)
{
	cout <<"Shared push single, ident: " << cmd.ident << " value: " << cmd.memory_value << " service: " << cmd.service_type << endl;
	int counter = 0,sep_count = 0;
	string sep = "separator";
	boost::interprocess::shared_memory_object shm(boost::interprocess::open_only, my_shm_ident.c_str(), boost::interprocess::read_write);
	boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
	char *mem = static_cast<char*>(region.get_address());
	while(cmd.service_type--){
		while(1){
			counter++;
			if(counter > (int)region.get_size() ){
				break;
			}
			if(*mem++ == sep.c_str()[sep_count]){
				if(sep_count == (int)sep.length()){
					sep_count =0;
					break;
				}
			} else {
				sep_count =0;
			}
		}
		if(counter > (int)region.get_size() ){
			break;
		}
	}
	std::size_t start_i = counter;

	for(std::size_t i = start_i; i < region.get_size(); ++i){
		if(*mem++ == cmd.ident.c_str()[counter]){
			counter++;
			if(counter == (int)cmd.ident.length()){
				if(*mem++ == '_'){
					*mem = cmd.memory_value[0];
					break;
				}
			}
		} else {
			counter = 0;
		}
	}
}

void hp_shared_memory::process_push_complex(hp_shared_command_t cmd)
{
	cout <<"Shared push complex, ident: " << cmd.ident << " value: " << cmd.memory_value << " service: " << cmd.service_type << endl;
	int counter = 0,sep_count = 0;
	string sep = "separator";
	boost::interprocess::shared_memory_object shm(boost::interprocess::open_only, my_shm_ident.c_str(), boost::interprocess::read_write);
	boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
	char *mem = static_cast<char*>(region.get_address());
	char *start_complex = NULL;
	while(cmd.service_type--){
		CHECK();
		while(1){
			counter++;
			if(counter > (int)region.get_size() ){
				break;
			}
			if(*mem++ == sep.c_str()[sep_count++]){
				if(sep_count == (int)sep.length()){
					sep_count =0;
					break;
				}
			} else {
				sep_count =0;
			}
		}
		if(counter > (int)region.get_size() ){
			break;
		}
	}
	cout <<"Coutner: " << counter << endl;
	std::size_t i,start_i = counter, end_counter = 0;
	counter = 0;

	for(i=start_i; i < region.get_size(); ++i){
		if(*mem++ == cmd.ident.c_str()[counter]){
			counter++;
			if(counter == (int)cmd.ident.length()){
				if(*mem++ == '_'){
					CHECK();
					start_complex = mem;
					//*mem = cmd.memory_value[0];
					break;
				}
			}
		} else {
			counter = 0;
		}
		if(*mem == '_'){
		      end_counter++;
		      if(end_counter == 2){
			      break;
		      }
	        } else {
		      end_counter = 0;
		}
	
	}
	counter = 0;
	if(start_complex != NULL){
		char *bp_data = (char *)malloc(region.get_size()-i);
		bool copy_data = false;
		for(size_t j=i; j<region.get_size(); j++){
			if(copy_data){
				bp_data[counter++] = *mem++;
			} else {
				if(*mem++ == ';'){
					copy_data = true;
				}
			}
		}
		mem = start_complex;
		for(size_t j = 0; j<cmd.memory_value.length(); j++){
			*mem++ = cmd.memory_value.c_str()[j];
		}
		*mem++=';';
		for(size_t j = 0; j<(size_t)counter; j++){
			*mem++ = bp_data[j];
		}
		free(bp_data);
		bp_data = NULL;
	}
}
hp_shared_memory::~hp_shared_memory() 
{
	boost::interprocess::shared_memory_object::remove(my_shm_ident.c_str());
}
