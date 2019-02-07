#include "SafeBuffer.h"
#include <string>
#include <queue>
#include <unistd.h>

SafeBuffer::SafeBuffer() {
	pthread_mutex_init(&mut, NULL);
	csize = 0;
}

SafeBuffer::~SafeBuffer() {
	pthread_mutex_destroy(&mut);
}

int SafeBuffer::size() {
    return csize;
}

void SafeBuffer::push_back(std::string str) {
	pthread_mutex_lock(&mut);
	q.push(str);
	csize++;
	pthread_mutex_unlock(&mut);
}

std::string SafeBuffer::retrieve_front() {
	if (q.empty()) {
		usleep(10000);
		return retrieve_front();
	}
	pthread_mutex_lock(&mut);
	std::string fr;
	fr = q.front();
	q.pop();
	csize--;
	pthread_mutex_unlock(&mut);
	return fr;
}