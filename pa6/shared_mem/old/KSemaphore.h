#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

class KSemaphore {

    private:
        int semid;

        
    public:
        KSemaphore();
        KSemaphore ( int val, int seed );
        ~KSemaphore ();
        void P ();
        void V ();
};