//
//  bounded_buffer.hpp
//  
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#ifndef bounded_buffer_h
#define bounded_buffer_h

#include <stdio.h>
#include <iostream>
#include <queue>
#include <string>
#include <pthread.h>
#include "semaphore.h"

class bounded_buffer {
private:
	semaphore emptySlots;
	semaphore fullSlots;
	semaphore mutex;
	int bufSize;
	int csize;
	std::queue<std::string> line;
	
	/* Internal data here */
public:
    bounded_buffer(int _capacity);
	//~bounded_buffer();
    void push_back(std::string str);
    std::string retrieve_front();
    int size();
};

#endif /* bounded_buffer_h */
