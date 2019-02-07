#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "reqchannel.h"

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

const bool VERBOSE = false;
const int MAX_MESSAGE = 255;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

RequestChannel::RequestChannel(const std::string _name, const Side _side, int seed) :
 my_name(_name), my_side(_side), side_name((_side == RequestChannel::SERVER_SIDE) ? "SERVER" : "CLIENT")
{	
	server_read = KSemaphore(1, seed); 
	client_read = KSemaphore(1, seed+200);

	key_t key = ftok("/home/ugrads/d/delrod19/csce313/pa6/shared_mem/shm.txt", seed);

	std::cout << my_name << ":" << side_name << " seed: " << seed << std::endl;
	//std::cout << my_name << ":" << side_name << " key: " << key << std::cout;
	shmid = shmget(key, 20, 0666 | IPC_CREAT | IPC_EXCL);
	if (shmid == -1)
		shmid = shmget(key, 20, 0666 | IPC_CREAT);
	std::cout << "shmid: " << shmid << std::endl;

	sigset_t sigpipe_set;
	sigemptyset(&sigpipe_set);
	if(sigaddset(&sigpipe_set, SIGPIPE) < 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on sigaddset(&sigpipe_set, SIGPIPE)");
	}
	
	if((errno = pthread_sigmask(SIG_SETMASK, &sigpipe_set, NULL)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_sigmas(SIG_SETMASK, &sigpipe_set, NULL)");
	}
	
	if((errno = pthread_mutexattr_init(&srl_attr)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_mutexattr_init");
	}
	if((errno = pthread_mutexattr_setrobust(&srl_attr, PTHREAD_MUTEX_ROBUST)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_mutexattr_setrobust");
	}
	if((errno = pthread_mutexattr_setpshared(&srl_attr, PTHREAD_PROCESS_SHARED)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_mutexattr_setpshared");
	}
	if((errno = pthread_mutex_init(&send_request_lock, &srl_attr)) != 0) {
		throw sync_lib_exception(my_name + ":" + side_name + ": failed on pthread_mutex_init");
	}
}

RequestChannel::~RequestChannel() {
	if(VERBOSE) std::cout << my_name << ":" << side_name << ": closing..." << std::endl;
	pthread_mutexattr_destroy(&srl_attr);
	pthread_mutex_destroy(&read_lock);
	pthread_mutex_destroy(&write_lock);
	pthread_mutex_destroy(&send_request_lock);
	
	shmctl(shmid, IPC_RMID, NULL);
}

/*--------------------------------------------------------------------------*/
/* READ/WRITE FROM/TO REQUEST CHANNELS  */
/*--------------------------------------------------------------------------*/

void RequestChannel::clear_mem() {
	char* data = (char*)shmat(shmid, 0, 0);
	memset(data, '\0', 20);
}

std::string RequestChannel::send_request(std::string _request) {
	//std::cout << "send_request request: " << _request << std::endl;

	if (my_side == CLIENT_SIDE) {
		
		int ret = cwrite(_request);	//writes to file
		client_read.P();
		server_read.V();
		client_read.P();
		std::string s = cread();	//reads from server
		client_read.V();
		clear_mem();
		return s;
	}
	// if(cwrite(_request) < 0) 
	// 	return "ERROR";
	
}

std::string RequestChannel::cread() {

	if(VERBOSE) 
		std::cout << my_name << ":" << side_name << ": reading..." << std::endl;
	
	if (my_side == SERVER_SIDE) {
		//std::cout << my_name << ":" << side_name << ":    server is trying to read yo" << std::endl;

		//server_read.P();//waits for server to be able to read

		char* buf = (char*)shmat(shmid, 0, 0);		//server reads request
		char request[10];
		strncpy(request, buf, 10);					//copies buffer to request
		std::cout << my_name << ":" << side_name << " read: " << request << std::endl;
		//server_read.V();
		return request;
	}
	else {
		char* buf = (char*)shmat(shmid, 0, 0);			//server reads request
		buf+=10;
		char response[10];
		strncpy(response, buf, 10);						//copies buffer to request
		//std::cout << my_name << ":" << side_name << "  read: " << response << std::endl;
		return response;
	}
}

void RequestChannel::setServLock() {
	server_read.P();
}

int RequestChannel::cwrite(std::string _msg) {
	
	//std::cout << _msg << std::endl;

	// if (_msg.length() >= MAX_MESSAGE) {
	// 	if(VERBOSE) std::cerr << my_name << ":" << side_name << "Message too long for Channel!" << std::endl;
	// 	return -1;
	// }

	char* data;

	if (my_side == SERVER_SIDE) {
		//std::cout << my_name << ":" << side_name << " writes: " << _msg << std::endl;
		data = (char*)shmat(shmid, 0, 0);
		data+=10;
		memset(data, '-', 10);
		strncpy(data, _msg.c_str(), 10);
		client_read.V();
	}
	else { //client
		//std::cout << my_name << ":" << side_name << " writes: " << _msg << std::endl;
		data = (char*)shmat(shmid, 0, 0);	//writes data
		memset(data, '-', 10);
		strncpy(data, _msg.c_str(), 10);
		server_read.V();
	}

	return 0;
}

std::string RequestChannel::name() {
	return my_name;
}