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

void process_request(RequestChannel & _channel, const std::string & _request);

/*--------------------------------------------------------------------------*/
/* VARIABLES */
pthread_mutex_t channel_mutex;
/*--------------------------------------------------------------------------*/

static int nthreads = 0;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void handle_process_loop(RequestChannel & _channel);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/

void * handle_data_requests(void * args) {

	RequestChannel * data_channel =  (RequestChannel*)args;

	handle_process_loop(*data_channel);

	delete data_channel;
	
	return nullptr;
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void* connection_handler(void* vchan) {
	RequestChannel* chan = (RequestChannel*)vchan;
	while(1) {
		std::string request = chan->cread();
		if (request.compare("quit") == 0) {
			chan->cwrite("bye");
			usleep(10000);
			break;
		}
		process_request(*chan, request);
	}
	delete chan;
}

void process_data(RequestChannel & _channel, const std::string &  _request) {
	usleep(1000 + (rand() % 5000));
	std::string response = std::to_string(rand() % 100);
	_channel.cwrite(response);
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(RequestChannel & _channel, const std::string & _request) {
	if (_request.compare(0, 4, "data") == 0) {
		process_data(_channel, _request);
	}
	else {
		_channel.cwrite("unknown request");
	}
}

void handle_process_loop(RequestChannel & _channel) {
	
	for(;;) {
		std::string request = _channel.cread();

		if (request.compare("quit") == 0) {
			_channel.cwrite("bye");
			usleep(10000);          // give the other end a bit of time.
			break;                  // break out of the loop;
		}

		process_request(_channel, request);
	}
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/


int main(int argc, char * argv[]) {
	int opt = 0;
	int port = 2666;
	int b = 100;
	while ((opt = getopt(argc, argv, "p:b:")) != -1) {
        switch (opt) {
            case 'p':
                port = (short)atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
        }
    }
	
	pthread_mutex_init (&channel_mutex, NULL);
	RequestChannel control_channel(port, b, connection_handler);

	handle_process_loop(control_channel);
}

