/*
 * This file is part of Autopin+.
 * Copyright (C) 2015 Technische Universität München - LRR
 *
 * This file is licensed under the GNU General Public License Version 3
 */

#include <AutopinPlus/Monitor/PageMigrate/Main.h>

#include <QTime>
#include <stdlib.h>
extern "C" {
 #include "util/migration_control.h"
}


namespace AutopinPlus {
namespace Monitor {
namespace PageMigrate {

Main::Main(QString name, const Configuration &config, AutopinContext &context)
	: PerformanceMonitor(name, config, context) {
	type = "random";
	started=false;
	//parameters for the memory access sampling
	period=1000;
	min_weight=50;
	sensing_time=30;
}


	
void Main::init() {
	context.info("Initializing monitor \"" + name + "\" (pageMigrate)");

	//Must be left, this value is read by the strategy
	valtype = MAX;
	// Set standard values


	if (config.configOptionExists(name + ".sampling_period") == 1) period = config.getConfigOptionInt(name + ".sampling_period");
	if (config.configOptionExists(name + ".min_weight") == 1) min_weight = config.getConfigOptionInt(name + ".min_weight");
	if (config.configOptionExists(name + ".sensing_time") == 1) sensing_time = config.getConfigOptionInt(name + ".sensing_time");
	
	//context.info("Maximum random value " + QString::number(rand_max));
}

Configuration::configopts Main::getConfigOpts() {
	Configuration::configopts result;

	
	result.push_back(Configuration::configopt("valtype", QStringList("MAX")));
	return result;
}

void Main::start(int tid) { 
	std::stringstream temp_str,temp_str2,temp_str3,temp_str4;
	std::list<const char *> params_list;
	context.info("My pid " + QString::number(monitored_pid));
	
	//this represents the parameters sent to the SPM via command line
	params_list.push_back("./perf");
	params_list.push_back("--numa-migrate");
	params_list.push_back("-e");
	params_list.push_back("cpu/mem-loads/pp");
	params_list.push_back("-a");
	params_list.push_back("--numa-repdetail");
	params_list.push_back("3");
	params_list.push_back("--track-accesslvls");
	params_list.push_back("-c");
	temp_str<<(period);
	std::string str = temp_str.str();
	params_list.push_back(str.c_str());
	params_list.push_back("--weighmin");
	temp_str2<<(min_weight);
	std::string str2  = temp_str2.str();
	params_list.push_back(str2.c_str());
	params_list.push_back("--sensing-time");
	temp_str3<<(sensing_time);
	std::string str3 = temp_str3.str();
	params_list.push_back(str3.c_str());
	params_list.push_back("--npid");
	temp_str4<<(monitored_pid);
	std::string str4 = temp_str4.str();
	params_list.push_back(str4.c_str());
	
	int list_size=params_list.size();
	const char** params_array=(const char**)malloc(sizeof(const char *)*list_size);
	
	for (int i=0; i<list_size; i++){
		params_array[i]=params_list.front();
		params_list.pop_front();
	}
	//const char * p1= "./perf";
	//const char * p2= "--numa-migrate";
	//const char * p3= "-e";
	//const char * p4= "cpu/mem-loads/pp";
	//const char * p5= "--cpu=0-31";
	//const char * p7= "--numa-repdetail";
	//const char * p8= "3";
	//const char * p9= "--track-accesslvls";
	//const char * p10= "-c";
	//temp_str<<(period);
	//std::string str = temp_str.str();
	//const char * p11= str.c_str();
	//const char * p12= "--weighmin";
	//temp_str2<<(min_weight);
	//std::string str2  = temp_str2.str();
	//const char * p13= str2.c_str();;
	//const char * p14= "--sensing-time";
	//temp_str3<<(sensing_time);
	//std::string str3 = temp_str3.str();
	//const char * p15= str3.c_str();
	//const char * p19= "--npid";
	//char p20[6];
	//sprintf(p20,"%d",monitored_pid);
	////"--numa-migrate -e cpu/mem-loads/pp --cpu=0-31  --numa-repdetail 3 --track-accesslvls -c 500
	//// --weighmin 150 --sensing-time 20   --npid 84789";
	//const char * args[]={p1,p2,p3,p4,p5,p7,p8,p9,p10,p11,p12,p13,p14,p15,p19,p20};
	if(!started){
		init_numa_analysis(params_array, list_size);
		started=true;
	}
	tid++; //just to get rid of warning
	 }

double Main::value(int tid) {
	
	return tid;
}

double Main::stop(int tid) {
	double result = value(tid);
	if (context.isError()) {
		return 0;
	}

	rands.erase(tid);
	return result;
}

void Main::clear(int tid) {
	if (rands.find(tid) != rands.end()) rands.erase(tid);
}

ProcessTree::autopin_tid_list Main::getMonitoredTasks() {
	ProcessTree::autopin_tid_list result;

	for (auto &elem : rands) result.insert(elem.first);

	return result;
}


} // namespace Random
} // namespace Monitor
} // namespace AutopinPlus
