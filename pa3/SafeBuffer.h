#ifndef SafeBuffer_h
#define SafeBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <pthread.h>

class SafeBuffer {

	pthread_mutex_t mut;
	std::queue<std::string> q;
	int csize;

public:
    SafeBuffer();
	~SafeBuffer();
	int size();
    void push_back(std::string str);
    std::string retrieve_front();
};

#endif /* SafeBuffer_ */