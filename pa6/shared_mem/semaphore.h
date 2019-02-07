#ifndef _semaphore_H_                   // include file only once
#define _semaphore_H_


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <string>

/*--------------------------------------------------------------------------*/
/* CLASS   s e m a p h o r e  */
/*--------------------------------------------------------------------------*/



class semaphore {
	
private:

	int val;
	int max;
	pthread_mutex_t mut;
	pthread_cond_t cond;
	
public:
	
	semaphore() {}
	
    semaphore(int _val) : val(_val) {
		pthread_mutex_init(&mut, NULL);
		pthread_cond_init(&cond, NULL);
	}   

    ~semaphore(){
		pthread_mutex_destroy(&mut);
		pthread_cond_destroy(&cond);
    }
    
    void P() {
		pthread_mutex_lock(&mut);
		val--;
		
		if (val < 0) //while there are no more resources to consume
			pthread_cond_wait(&cond, &mut); //waits to be woken up by a producer
		
		pthread_mutex_unlock(&mut); 
    }

    void V() {
		pthread_mutex_lock(&mut);

		val++;
		pthread_cond_signal(&cond); //notifies consumer threads they can consume now
		
		pthread_mutex_unlock(&mut);
    }
};

#endif