#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "reqchannel.h"
#include "SafeBuffer.h"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

struct PARAMS_REQUEST {	
	SafeBuffer* mb;
	int id;
	
	int n;
	
	PARAMS_REQUEST(int nn, int nid, SafeBuffer* nmb) {
		n = nn;
		id = nid;
		mb = nmb;
	}
	
};

struct PARAMS_WORKER {
	int n;
	int b;
	int w;
	
	SafeBuffer* mb;
	vector<SafeBuffer*>* ind_buf;
	
	RequestChannel* chan;
	
	
	PARAMS_WORKER(int nn, int nb, int nw, RequestChannel* nchan, SafeBuffer* nmb, vector<SafeBuffer*>* nind_buf) {
		n = nn;
		b = nb;
		w = nw;
		chan = nchan;
		mb = nmb;
		ind_buf = nind_buf;
	}
};

struct PARAMS_STAT {
	int id;
	SafeBuffer* ib;
	vector<int>* h;
	
	int n;
	int b;
	int w;
	
	PARAMS_STAT(int nn, int nb, int nw, int nid, SafeBuffer* nib, vector<int>* nh) {
		n = nn;
		b = nb;
		w = nw;
		id = nid;
		ib = nib;
		h = nh;
	}	
};


class atomic_standard_output {
    /*
         Note that this class provides an example
         of the usage of mutexes, which are a crucial
         synchronization type. You will probably not
         be able to complete this assignment without
		using mutexes.
     */
    pthread_mutex_t console_lock;
public:
		atomic_standard_output() {
			pthread_mutex_init(&console_lock, NULL);
		}
		~atomic_standard_output() {
			pthread_mutex_destroy(&console_lock);
		}
		void println(std::string s) {
			pthread_mutex_lock(&console_lock);
			std::cout << s << std::endl;
			pthread_mutex_unlock(&console_lock);
		}
		void perror(std::string s) {
			pthread_mutex_lock(&console_lock);
			std::cerr << s << ": " << strerror(errno) << std::endl;
			pthread_mutex_unlock(&console_lock);
		}
};

atomic_standard_output threadsafe_console_output;

std::string make_histogram_table(std::string name1, std::string name2,
        std::string name3, std::vector<int> *data1, std::vector<int> *data2,
        std::vector<int> *data3) {
	std::stringstream tablebuilder;
	tablebuilder << std::setw(25) << std::right << name1;
	tablebuilder << std::setw(15) << std::right << name2;
	tablebuilder << std::setw(15) << std::right << name3 << std::endl;
	for (int i = 0; i < data1->size(); ++i) {
		tablebuilder << std::setw(10) << std::left
		        << std::string(
		                std::to_string(i * 10) + "-"
		                        + std::to_string((i * 10) + 9));
		tablebuilder << std::setw(15) << std::right
		        << std::to_string(data1->at(i));
		tablebuilder << std::setw(15) << std::right
		        << std::to_string(data2->at(i));
		tablebuilder << std::setw(15) << std::right
		        << std::to_string(data3->at(i)) << std::endl;
	}
	tablebuilder << std::setw(10) << std::left << "Total";
	tablebuilder << std::setw(15) << std::right
	        << accumulate(data1->begin(), data1->end(), 0);
	tablebuilder << std::setw(15) << std::right
	        << accumulate(data2->begin(), data2->end(), 0);
	tablebuilder << std::setw(15) << std::right
	        << accumulate(data3->begin(), data3->end(), 0) << std::endl;

	return tablebuilder.str();
}


void* request_thread_function(void* arg) {
	const int JOHN_ID = 0;
	const int JANE_ID = 1;
	const int JOE_ID = 2;

	PARAMS_REQUEST* pr = (PARAMS_REQUEST*)arg;

	string push = "data ";

	switch (pr->id) {
		case JOHN_ID:
			push += "john";
			break;
		case JANE_ID:
			push += "jane";
			break;
		case JOE_ID:
			push += "joe ";
			break;
		default:
			push = "done";
			break;
	}

	for(int i = 0; i < pr->n; i++)
		pr->mb->push_back(push);
}

void* worker_thread_function(void* arg) {
	const int JOHN_ID = 0;
	const int JANE_ID = 1;
	const int JOE_ID = 2;

	PARAMS_WORKER* pw = (PARAMS_WORKER*)arg;

	vector<SafeBuffer*>* individual_buffers = pw->ind_buf;

	string response = "";

    while(true) {
		response = pw->mb->retrieve_front();
		
		if (response.compare("done") == 0)
			break;
		
		pw->chan->cwrite(response);
		string r = pw->chan->cread();
		string who = response.substr(5);
				
		if (who.compare("joe ") == 0)
			individual_buffers->at(JOE_ID)->push_back(r);
		else if (who.compare("john") == 0)
			individual_buffers->at(JOHN_ID)->push_back(r);
		else if (who.compare("jane") == 0)
			individual_buffers->at(JANE_ID)->push_back(r);
		else cout << "oh darn theres an error" << endl;
    }
	pw->chan->send_request("quit");
	delete pw->chan;
}

void* stat_thread_function(void* arg) {
	const int JOHN_ID = 0;
	const int JANE_ID = 1;
	const int JOE_ID = 2;
	
	
	PARAMS_STAT ps = *((PARAMS_STAT*)arg);
	
	string s;
	int num;
	
	int id = ps.id;
	vector<int>* hist = ps.h;
	SafeBuffer* ind_b = ps.ib;
	
	for (int i = 0; i < ps.n; i++) {
		switch (id) {
			case JOE_ID:
				s = ind_b->retrieve_front();
				num = atoi(s.c_str());
				hist->at(num/10)++;
				break;
			case JOHN_ID:
				s = ind_b->retrieve_front();
				num = atoi(s.c_str());
				hist->at(num/10)++;
				break;
			case JANE_ID:
				s = ind_b->retrieve_front();
				num = atoi(s.c_str());
				hist->at(num/10)++;
				break;
			default:
				cout << "oh shit theres an error pt 2" << endl;
				break;
		}
	}
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
	int b = 50;
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:h")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
			case 'h':
            default:
				std::cout << "This program can be invoked with the following flags:" << std::endl;
				std::cout << "-n [int]: number of requests per patient" << std::endl;
				std::cout << "-w [int]: number of worker threads" << std::endl;
				std::cout << "-h: print this message and quit" << std::endl;
				std::cout << "(Canonical) example: ./client_solution -n 10000 -w 128" << std::endl;
				std::cout << "If a given flag is not used, or given an invalid value," << std::endl;
				std::cout << "a default value will be given to the corresponding variable." << std::endl;
				std::cout << "If an illegal option is detected, behavior is the same as using the -h flag." << std::endl;
                exit(0);
        }
    }

    int pid = fork();
	if (pid > 0) {

        std::cout << "n == " << n << std::endl;
        std::cout << "w == " << w << std::endl;

        std::cout << "CLIENT STARTED:" << std::endl;
        std::cout << "Establishing control channel... " << std::flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        std::cout << "done." << std::endl;

        /*
            All worker threads will use the following structures,
            but the current implementation puts
            them on the stack in main. You can change the code
		 	to use the heap if you'd like, or even a
		 	container other than std::vector. However,
		 	in the end you MUST use SafeBuffer, or some other
			threadsafe, FIFO data structure, instead
		 	of std::list for the request_buffer.
         */

		SafeBuffer request_buffer;
		//std::list<std::string> request_buffer;
        //std::vector<int> john_frequency_count(10, 0);
        //std::vector<int> jane_frequency_count(10, 0);
        //std::vector<int> joe_frequency_count(10, 0);

/*--------------------------------------------------------------------------*/
/*  BEGIN CRITICAL SECTION  */
/*
		 You will modify the program so that client_MP6.cpp
		 populates the request buffer using 3 request threads
		 instead of sequentially in a loop, and likewise
		 processes requests using w worker threads instead of
		 sequentially in a loop.

		 Note that in the finished product, as in this initial code,
		 all the requests will be pushed to the request buffer
		 BEFORE the worker threads begin processing them. This means
		 that you will have to ensure that all 3 request threads
		 have terminated before kicking off any worker threads.
		 In the next machine problem, we will deal with the
		 synchronization problems that arise from allowing the
		 request threads and worker threads to run in parallel
		 with each other. In the meantime, see if you can figure
		 out the limitations of this machine problem's approach (*cough*stack overflow,
		 as in not the Q&A site*coughcough*).

		 Just to be clear: for this machine problem, request
		 threads will run concurrently with request threads, and worker
		 threads will run concurrently with worker threads, but request
		 threads will NOT run concurrently with worker threads.

		 While gathering data for your report, it is recommended that you comment
		 out all the output operations occurring between when you
		 start the timer and when you end the timer. Output operations
		 are very time-intensive (at least compared to the other operations 
		 we're doing) and will skew your results.
*/
/*--------------------------------------------------------------------------*/

        //std::cout << "Populating request buffer... ";

		int num_clients = 3; //to make it easier to add more clients in future
		 
		//vectors to hold the request threads
		vector<pthread_t> request_threads(num_clients);
		vector<pthread_t> worker_threads(w);
		vector<pthread_t> stats_threads(num_clients);

		vector<SafeBuffer*> individual_buffers;
		vector<PARAMS_REQUEST*> request_params(num_clients);
		vector<PARAMS_WORKER*> worker_params(w);
		vector<PARAMS_STAT*> stat_params(num_clients);

		struct timeval start, stop;
		long sec, musec;
		gettimeofday(&start, NULL);


		for (int i = 0; i < num_clients; i++)
			individual_buffers.push_back(new SafeBuffer());

		for (int i = 0; i < num_clients; i++) {
			request_params.at(i) = new PARAMS_REQUEST(n, i, &request_buffer);
			pthread_create(&request_threads.at(i), NULL, request_thread_function, (void*)request_params.at(i));
		}

		//creates request channels for each worker thread, and then creates the threads
		for (int i = 0; i < w; i++) {
			string r = chan->send_request("newthread");
			RequestChannel* nchan = new RequestChannel(r, RequestChannel::CLIENT_SIDE);
			//vector<SafeBuffer*>* t_ind_buf = &individual_buffers;
			worker_params.at(i) = new PARAMS_WORKER(n, b, w, nchan, &request_buffer, &individual_buffers);//t_ind_buf);
			pthread_create(&worker_threads.at(i), NULL, worker_thread_function, (void*)worker_params.at(i));
		}

		vector<vector<int>*> histograms;
		
		//creates the stats threads
		for (int i = 0; i < num_clients; i++) {
			//makes a new vector of ints to store the results of each client
			histograms.push_back(new vector<int>(10, 0));
			stat_params.at(i) = new PARAMS_STAT(n, b, w, i, individual_buffers.at(i), histograms.at(i));
			pthread_create(&stats_threads.at(i), NULL, stat_thread_function, (void*)stat_params.at(i));
		}


		//waits for the request threads to join
		for (int i = 0; i < num_clients; i++)
			pthread_join(request_threads.at(i), NULL);

        for(int i = 0; i < w; i++)
            request_buffer.push_back("done");

		for (int i = 0; i < w; i++)
			pthread_join(worker_threads.at(i), NULL);

		//waits for the stats threads to join
		for (int i = 0; i < num_clients; i++)
			pthread_join(stats_threads.at(i), NULL);

		/*-------------------------------------------*/
		/* START TIMER HERE */
		/*-------------------------------------------*/

        //std::string s = chan->send_request("newthread");
        //RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

        // while(true) {
        //     std::string request = request_buffer.front();
        //     request_buffer.pop_front();
        //     std::string response = workerChannel->send_request(request);

        //     if(request == "data John Smith") {
        //         john_frequency_count.at(stoi(response) / 10) += 1;
        //     }
        //     else if(request == "data Jane Smith") {
        //         jane_frequency_count.at(stoi(response) / 10) += 1;
        //     }
        //     else if(request == "data Joe Smith") {
        //         joe_frequency_count.at(stoi(response) / 10) += 1;
        //     }
        //     else if(request == "quit") {
        //         delete workerChannel;
        //         break;
        //     }
        // }

/*--------------------------------------------------------------------------*/
/*  END CRITICAL SECTION    */
/*--------------------------------------------------------------------------*/

        /*
            By the point at which you end the timer,
            all worker threads should have terminated.
            Note that the containers from earlier (namely
			request_buffery and the *frequency_count vectors)
            are still in scope, so threads can still use them
            if they have a pointer to them.
         */

        /*-------------------------------------------*/
        /* END TIMER HERE   */
        /*-------------------------------------------*/

        /*
            You may want to eventually add file output
            to this section of the code in order to make it easier
            to assemble the timing data from different iterations
            of the program. If you do, (and this is just a recommendation)
			try to use a format that will be easy for you to make a graph with later, 
			such as comma-separated values (LaTeX) or labeled rows and columns 
			(spreadsheet software like Excel and Google Sheets).
         */

        //std::cout << "Finished!" << std::endl;
		gettimeofday(&stop, NULL);

		std::string histogram_table = make_histogram_table("John Smith",
		        "Jane Smith", "Joe Smith", histograms.at(0),
		        histograms.at(1), histograms.at(2));

        std::cout << "Results for n == " << n << ", w == " << w << std::endl;

		/*
		 	This is a good place to output your timing data.
		 */

        std::cout << histogram_table << std::endl;

    	float timetaken=(float)(stop.tv_sec-start.tv_sec)+(float)((float)(stop.tv_usec-start.tv_usec)/1000000);
    	cout << "Amout of time taken: " << timetaken << endl;

        std::cout << "Sleeping..." << std::endl;
        usleep(10000);

        std::string finale = chan->send_request("quit");
        delete chan;
        std::cout << "Finale: " << finale << std::endl; //This line, however, is optional.
    }
	else if (pid == 0)
		execl("dataserver", (char*) NULL);
}
