//
//  BoundedBuffer.hpp
//  
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <pthread.h>
#include "semaphore.h"
using namespace std;
class BoundedBuffer {
	/* Internal data here */
	semaphore* mutex;;
	semaphore* full;
	semaphore* empty;
	queue<string> q;
public:
    BoundedBuffer(int _capacity);
    ~BoundedBuffer();
    void push(string str);
    string pop();
};

#endif /* BoundedBuffer_h */
