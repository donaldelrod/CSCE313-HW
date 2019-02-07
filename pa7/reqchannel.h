/*
    File: reqchannel.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/11

*/

#ifndef _reqchannel_H_                   // include file only once
#define _reqchannel_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

class sync_lib_exception : public std::exception {
	std::string err = "failure in sync library";
	
public:
	sync_lib_exception() {}
	sync_lib_exception(std::string msg) : err(msg) {}
	virtual const char* what() const throw() {
		return err.c_str();
	}
};

/*--------------------------------------------------------------------------*/
/* FORWARDS */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CLASS   R e q u e s t C h a n n e l */
/*--------------------------------------------------------------------------*/

class RequestChannel {

public:

	typedef enum {SERVER_SIDE, CLIENT_SIDE} Side;

	typedef enum {READ_MODE, WRITE_MODE} Mode;

private:

	std::string   my_name = "";
	std::string side_name = "";
	Side     my_side;

	/*  The current implementation uses named pipes. */ 

	std::string server_name;
	short port;

	int s;
	
	/*	Locks used to keep the dataserver from dropping requests.	*/
	pthread_mutex_t read_lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutexattr_t srl_attr;
	pthread_mutex_t send_request_lock;

public:

	/* -- CONSTRUCTOR/DESTRUCTOR */

	RequestChannel(std::string hostname, short portnum);

	RequestChannel(short port, int b, void* (*connection_handler) (void *));

	RequestChannel(int s);

	~RequestChannel();
	/* Destructor of the local copy of the bus. By default, the Server Side deletes any IPC 
	 mechanisms associated with the channel. */

	std::string send_request(std::string _request);
	/* Send a string over the channel and wait for a reply. */

	std::string cread();
	/* Blocking read of data from the channel. Returns a string of characters
	 read from the channel. Returns NULL if read failed. */

	int cwrite(std::string _msg);
	/* Write the data to the channel. The function returns the number of characters written
	 to the channel. */

	std::string name();
	/* Returns the name of the request channel. */

	int read_fd();

};

#endif