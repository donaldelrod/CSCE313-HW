/* 
    File: dataserver.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/16

    Dataserver main program for MP3 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "reqchannel.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

struct PARAMS_DATA {
	
	RequestChannel* serv_recv;
	RequestChannel* serv_send;
	
	PARAMS_DATA(RequestChannel* sr, RequestChannel* ss) : serv_recv(sr), serv_send(ss) {}
	
};

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* VARIABLES */
pthread_mutex_t channel_mutex;
/*--------------------------------------------------------------------------*/

static int nthreads = 0;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void handle_process_loop(RequestChannel &recv_channel, RequestChannel &send_channel);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

	/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/


void * handle_data_requests(void * args) {

	PARAMS_DATA* pd = (PARAMS_DATA*)args;
	
	//RequestChannel * data_channel =  (RequestChannel*)args;

	// -- Handle client requests on this channel. 

	handle_process_loop(*pd->serv_recv, *pd->serv_send);
	
	//handle_process_loop(*data_channel);

	// -- Client has quit. We remove channel.

	delete pd;
	
	return nullptr;
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void process_hello(RequestChannel & _channel, const std::string & _request) {
	_channel.cwrite("hello to you too");
}

void process_data(RequestChannel & _channel, const std::string &  _request) {
	usleep(1000 + (rand() % 5000));
	//_channel.cwrite("here comes data about " + _request.substr(4) + ": " + std::to_string(random() % 100));
	_channel.cwrite(_request.substr(5) + " " + std::to_string(rand() % 100));
}


void process_newthread(RequestChannel & _channel, const std::string & _request) {
	pthread_mutex_lock(&channel_mutex);	
	int error;
	nthreads ++;
	
	RequestChannel* sr = new RequestChannel("/serv_recv", RequestChannel::SERVER_SIDE);
	RequestChannel* ss = new RequestChannel("/serv_send", RequestChannel::SERVER_SIDE);
	

	//_channel.cwrite(new_channel_name);
	pthread_mutex_unlock(&channel_mutex);
	// -- Construct new data channel (pointer to be passed to thread function)
	try {
		//RequestChannel * data_channel = new RequestChannel(new_channel_name, RequestChannel::SERVER_SIDE);

		// -- Create new thread to handle request channel
		
		PARAMS_DATA* pd = new PARAMS_DATA(sr, ss);

		pthread_t thread_id;
		//  std::cout << "starting new thread " << nthreads << endl;
		if ((errno = pthread_create(&thread_id, NULL, handle_data_requests, pd)) != 0) {
			perror(std::string("DATASERVER: " + _channel.name() + ": pthread_create failure").c_str());
			delete pd;
			exit(errno);
		}
	}
	catch (sync_lib_exception sle) {
		perror(std::string(sle.what()).c_str());
	}

}


/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(RequestChannel & _channel, const std::string & _request) {
	
	//std::cout << "request in process request: " << _request << std::endl;
	
	if (_request.compare(0, 5, "hello") == 0) {
		process_hello(_channel, _request);
	}
	else if (_request.compare(0, 4, "data") == 0) {
		process_data(_channel, _request);
	}
	else if (_request.compare(0, 9, "newthread") == 0) {
		process_newthread(_channel, _request);
	}
	else {
		_channel.cwrite("unknown request");
	}
}

void handle_process_loop(RequestChannel &recv_channel, RequestChannel &send_channel) {
	
	for(;;) {
		//std::cout << "Reading next request from channel (" << _channel.name() << ") ..." << std::flush;
		std::cout << std::flush;
		std::string request = recv_channel.cread();
		//std::cout << " done (" << _channel.name() << ")." << endl;
		//std::cout << "New request is " << request << endl;

		if (request.compare("quit") == 0) {
			//send_channel.cwrite("bye");
			usleep(10000);          // give the other end a bit of time.
			break;                  // break out of the loop;
		}
		
		//std::cout << "handle process loop request: \\" << request << "/" << std::endl;

		process_request(send_channel, request);
	}
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/


int main(int argc, char * argv[]) {
	//  std::cout << "Establishing control channel... " << std::flush;
	pthread_mutex_init (&channel_mutex, NULL);
	RequestChannel control("/control", RequestChannel::SERVER_SIDE);

	//RequestChannel control_channel("control", RequestChannel::SERVER_SIDE);
	//  std::cout << "done.\n" << std::flush;
	
	handle_process_loop(control, control);
	
	execl("./cleanup.sh", NULL, NULL);
	
	exit(0);
}

