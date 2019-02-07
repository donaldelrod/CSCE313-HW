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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "reqchannel.h"

const bool VERBOSE = false;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   RequestChannel  */
/*--------------------------------------------------------------------------*/

//client side
RequestChannel::RequestChannel(std::string hostname, short portnum) : server_name(hostname), port(portnum) {
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	inet_pton(AF_INET, hostname.c_str(), (void*)&addr.sin_addr);
	memset(&(addr.sin_zero), '\0', 8);
	socklen_t addr_size = sizeof(sockaddr_in);
	int succ_conn = connect(s, (sockaddr*)&addr, addr_size);
	if (succ_conn == -1) {
		std::cout << "failed to connect the socket" << std::endl;
		perror("shit");
	}
}

//server side
RequestChannel::RequestChannel(short port, int b, void* (*connection_handler) (void *)) : port(port) {
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	//inet_pton(AF_INET, INADDR_ANY, (void*)&addr.sin_addr);
	addr.sin_addr.s_addr = INADDR_ANY;
	socklen_t addr_size = sizeof(sockaddr_in);
	int succ_bind = bind(s, (sockaddr*)&addr, addr_size);
	//std::cout << "bind return: " << succ_bind << std::endl;
	int e = listen(s, b);
	memset(&(addr.sin_zero), '\0', 8);
	while(1) {
		int new_s = accept(s, (sockaddr*)&addr, &addr_size);
		RequestChannel* n = new RequestChannel(new_s);
		pthread_t tid;
		pthread_create(&tid, 0, (**connection_handler), n);
	}
}

RequestChannel::RequestChannel(int s) : s(s) {}

RequestChannel::~RequestChannel() {
	close(s);
}

/*--------------------------------------------------------------------------*/
/* READ/WRITE FROM/TO REQUEST CHANNELS  */
/*--------------------------------------------------------------------------*/

const int MAX_MESSAGE = 255;

std::string RequestChannel::send_request(std::string _request) {
	if(cwrite(_request) < 0) {
		perror("error in cwrite");
	}
	std::string s = cread();
	return s;
}

std::string RequestChannel::cread() {
	char buf[MAX_MESSAGE];
	memset(buf, '\0', MAX_MESSAGE);
	
	int read_ret_val;
	if ((read_ret_val = recv(s, buf, MAX_MESSAGE, 0)) <= 0) {
		perror("error in cread");
	}

	return buf;
}

int RequestChannel::cwrite(std::string _msg) {
	if (_msg.length() >= MAX_MESSAGE) {
		if(VERBOSE) std::cerr << my_name << ":" << side_name << "Message too long for Channel!" << std::endl;
		return -1;
	}
	
	const char * st = _msg.c_str();

	int write_return_value;
	if ((write_return_value = send(s, st, strlen(st)+1, 0)) < 0) {
		perror("error writing to socket");
	}
	
	return write_return_value;
}

/*--------------------------------------------------------------------------*/
/* ACCESS THE NAME OF REQUEST CHANNEL  */
/*--------------------------------------------------------------------------*/

std::string RequestChannel::name() {
	return my_name;
}

/*--------------------------------------------------------------------------*/
/* ACCESS FILE DESCRIPTORS OF REQUEST CHANNEL  */
/*--------------------------------------------------------------------------*/

int RequestChannel::read_fd() {
	return s;
}