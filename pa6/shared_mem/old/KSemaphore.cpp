#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "KSemaphore.h"



KSemaphore::KSemaphore(){}

KSemaphore::KSemaphore ( int val, int seed ) {
    key_t key = ftok("/home/ugrads/d/delrod19/csce313/pa6/shared_mem/ksems.txt", seed);
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        semid = semget(key, 1, IPC_CREAT | 0666);
        return;
    }
    struct sembuf sb = {0, val, 0};
    semop(semid, &sb, 1);
}

KSemaphore::~KSemaphore () {
    semctl(semid, 0, IPC_RMID);
}

void KSemaphore::P () {
    sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
    std::cout << "locking.." << std::endl;
}

void KSemaphore::V () {
    sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
    std::cout << "unlocking.." << std::endl;
}
