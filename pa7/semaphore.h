/* 
    File: semaphore.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 08/02/11

*/

#ifndef _semaphore_H_                   // include file only once
#define _semaphore_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <pthread.h>

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* FORWARDS */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CLASS   s e m a p h o r e  */
/*--------------------------------------------------------------------------*/

class semaphore {
private:
    /* -- INTERNAL DATA STRUCTURES */
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int value;
public:

    /* -- CONSTRUCTOR/DESTRUCTOR */

    semaphore(int _v) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        value = _v;
	}

    ~semaphore(){
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    /* -- SEMAPHORE OPERATIONS */
    
    void P() {
        pthread_mutex_lock(&mutex);
        --value;
        if(value < 0){
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    void V() {
        pthread_mutex_lock(&mutex);
        ++value;
        if(value <= 0){ 
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }
};

#endif


