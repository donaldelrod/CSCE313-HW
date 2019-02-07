#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <pwd.h>
#include <fcntl.h>
#include "KSemaphore.h"



KSemaphore::KSemaphore(){}

KSemaphore::KSemaphore ( int val, int seed ) {
    value = val;
    key_t key = ftok("/home/ugrads/d/delrod19/csce313/pa6/shared_mem/ksem.txt", seed);
    std::cout << "key from ftok: " << key << std::endl;
    //key = 1491;
    semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (semid == -1) {
        std::cout << "semaphore already exists" << std::endl;
    }
    // semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (semid == -1) {
        std::cout << "semaphore exists.." << std::endl;
        semid = semget(key, 1, 0666 | IPC_CREAT);
        if (semid == -1) {
            std::cout << "semaphore cannot be created" << std::endl;
            return;
        }
        std::cout << "semaphore created" << std::endl;
        return;
    }
    struct sembuf sb = {0, (short)val, (short)0};
    semop(semid, &sb, 1);
    printValue();
}

KSemaphore::~KSemaphore () {
    semctl(semid, 0, IPC_RMID);
}

void KSemaphore::P () {
    sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
    value--;
    //std::cout << "locking.." << std::endl;
}

void KSemaphore::V () {
    sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
    value++;
    //std::cout << "unlocking.." << std::endl;
}

void KSemaphore::printValue() {
    std::cout << "semaphore value is " << value << std::endl;
}

int KSemaphore::getSemid() {
    return semid;
}