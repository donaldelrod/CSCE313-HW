//
//  BoundedBuffer.cpp
//  
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#include "BoundedBuffer.h"
#include <string>
#include <queue>

BoundedBuffer::BoundedBuffer(int _capacity) {
	mutex = new semaphore(1);
	empty = new semaphore(_capacity);
	full  = new semaphore(0);
}

BoundedBuffer::~BoundedBuffer(){
	delete mutex, empty, full;
}

void BoundedBuffer::push(string req) {
    empty->P();
	mutex->P();
	q.push(req);
	mutex->V();
	full->V();
}

string BoundedBuffer::pop() {
    full->P();
	mutex->P();
	string s = q.front();
	q.pop();
	mutex->V();
	empty->V();
	return s;
}
