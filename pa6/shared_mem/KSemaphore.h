#ifndef _KSemaphore_H_                   // include file only once
#define _KSemaphore_H_

#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

class KSemaphore {

    private:
        int semid;
        int value;
        
    public:
        KSemaphore();
        KSemaphore ( int val, int seed );
        ~KSemaphore ();
        void P ();
        void V ();
        void printValue();
        int getSemid();
};

#endif