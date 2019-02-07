//
//  bounded_buffer.cpp
//  
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#include "bounded_buffer.h"
#include <string>
#include <queue>

using namespace std;

bounded_buffer::bounded_buffer(int _capacity) {
	bufSize = _capacity;
	emptySlots = semaphore(bufSize);
	fullSlots = semaphore(0);
	mutex = semaphore(1);
	csize = 0;
}

/*bounded_buffer::~bounded_buffer() {
	emptySlots.~semaphore();
	fullSlots.~semaphore();
	mutex.~semaphore();
}*/

void bounded_buffer::push_back(std::string req) {
	emptySlots.P();
	mutex.P();
	line.push(req);
	csize++;
	mutex.V();
	fullSlots.V();
}

std::string bounded_buffer::retrieve_front() {
	fullSlots.P();
	mutex.P();
	std::string s = line.front();
	line.pop();
	csize--;
	mutex.V();
	emptySlots.V();
	return s;
}

int bounded_buffer::size() {
	return csize;
}