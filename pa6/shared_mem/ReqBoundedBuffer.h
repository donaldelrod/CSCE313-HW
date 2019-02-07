#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "KSemaphore.h"


class ReqBoundedBuffer {
	private:
		KSemaphore empty;
		KSemaphore full;
		char* address;
		int shmid;

	public:
		ReqBoundedBuffer() {}
		ReqBoundedBuffer(int seed) {
			empty = KSemaphore(1, seed); 
			full = KSemaphore(1, seed+200);

			key_t key = ftok("/home/ugrads/d/delrod19/csce313/pa6/shared_mem/shm.txt", seed);

			//std::cout << my_name << ":" << side_name << " seed: " << seed << std::endl;
			//std::cout << my_name << ":" << side_name << " key: " << key << std::cout;
			shmid = shmget(key, 11, 0666 | IPC_CREAT | IPC_EXCL);
			if (shmid == -1)
				shmid = shmget(key, 11, 0666 | IPC_CREAT);
			//std::cout << "shmid: " << shmid << std::endl;
			address = (char*)shmat(shmid, 0, 0);
		}

		void push(std::string data) {
			empty.P();
			strncpy(address, data.c_str(), 11);
			full.V();
		}

		std::string pop() {
			full.P();
			char response[11];
			strncpy(response, address, 11);
			memset(address, '\0', 11);
			empty.V();
		}
};