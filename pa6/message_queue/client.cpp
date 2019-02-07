/*
    File: client.cpp

    Author: J. Higginbotham
    Department of Computer Science
    Texas A&M University
    Date  : 2016/05/21

    Based on original code by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
    /* -- This might be a good place to put the size of
        of the patient response buffers -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*
    No additional includes are required
    to complete the assignment, but you're welcome to use
    any that you think would help.
*/
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <assert.h>
#include <fstream>
#include <numeric>
#include <vector>
#include "reqchannel.h"
#include "bounded_buffer.h"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/


struct PARAMS_REQUEST {	
	bounded_buffer* mb;
	int id;
	
	int n;
	int w;
	int b;
	
	PARAMS_REQUEST(int nn, int nb, int nw, int nid, bounded_buffer* nmb) {
		n = nn;
		b = nb;
		w = nw;
		id = nid;
		mb = nmb;
	}
	
};

struct PARAMS_WORKER {
	int n;
	int b;
	int w;
	
	bounded_buffer* mb;
	vector<bounded_buffer*>* ind_buf;
	
	RequestChannel* serv_send;
	RequestChannel* serv_recv;
	
	
	PARAMS_WORKER(int nn, int nb, int nw, RequestChannel* ss, RequestChannel* sr, bounded_buffer* nmb, vector<bounded_buffer*>* nind_buf) {
		n = nn;
		b = nb;
		w = nw;
		serv_send = ss;
		serv_recv = sr;
		mb = nmb;
		ind_buf = nind_buf;
	}
};

struct PARAMS_STAT {
	int id;
	bounded_buffer* ib;
	vector<int>* h;
	
	int n;
	int b;
	int w;
	
	PARAMS_STAT(int nn, int nb, int nw, int nid, bounded_buffer* nib, vector<int>* nh) {
		n = nn;
		b = nb;
		w = nw;
		id = nid;
		ib = nib;
		h = nh;
	}	
};

/*
    This class can be used to write to standard output
    in a multithreaded environment. It's primary purpose
    is printing debug messages while multiple threads
    are in execution.
 */
class atomic_standard_output {
    pthread_mutex_t console_lock;
public:
    atomic_standard_output() { pthread_mutex_init(&console_lock, NULL); }
    ~atomic_standard_output() { pthread_mutex_destroy(&console_lock); }
    void print(std::string s){
        pthread_mutex_lock(&console_lock);
        cout << s << endl;
        pthread_mutex_unlock(&console_lock);
    }
};

atomic_standard_output threadsafe_standard_output;

/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS */
/*--------------------------------------------------------------------------*/

std::string make_histogram(std::string name, std::vector<int> *data) {
	int total = 0;
	for (int i = 0; i < data->size(); i++)
		total += data->at(i);
    std::string results = "Frequency count for " + name + " (total count: " + to_string(total) + "):\n";
    for(int i = 0; i < data->size(); ++i) {
        results += std::to_string(i * 10) + "-" + std::to_string((i * 10) + 9) + ": " + std::to_string(data->at(i)) + "\n";
    }
    return results;
}

void* request_thread_function(void* arg) {
	const unsigned int JOE_ID = 0;
	const unsigned int JOHN_ID = 1;
	const unsigned int JANE_ID = 2;
	
	
	PARAMS_REQUEST pr = *((PARAMS_REQUEST*)arg);
	int request_id = pr.id;
	string req_str = "data ";
	bounded_buffer* mb = pr.mb;
	
	switch (request_id) {
		case JOE_ID:
			req_str += "joe ";
			break;
		case JOHN_ID:
			req_str += "john";
			break;
		case JANE_ID:
			req_str += "jane";
			break;
		default:
			req_str = "done";
			break;
	}
	for (int i = 0; i < pr.n; i++)
		mb->push_back(req_str);
}

void* worker_thread_function(void* arg) {
	const unsigned int JOE_ID = 0;
	const unsigned int JOHN_ID = 1;
	const unsigned int JANE_ID = 2;
	
	
	PARAMS_WORKER pw = *((PARAMS_WORKER*)arg);
	
	//RequestChannel* w_chan = (RequestChannel*)arg;
	
	string response;
	
	vector<bounded_buffer*>* individual_buffers = pw.ind_buf;
	
	bool running = true;
	
	while (running) {
		response = pw.mb->retrieve_front();
		
		//cout << "response: " << response << endl;
		
		if (response.compare("done") == 0)
			break;
		
		
		//string r = pw.serv_recv->send_request(response);
		pw.serv_recv->cwrite(response);
		//cout << "wrote response" << endl;
		string r = pw.serv_send->cread();
		string who = r.substr(0,4);
		r = r.substr(5);
		
		
		//cout << "request response: \\" << r << "/" << endl;
		
		if (who.compare("joe ") == 0)
			individual_buffers->at(JOE_ID)->push_back(r);
		else if (who.compare("john") == 0)
			individual_buffers->at(JOHN_ID)->push_back(r);
		else if (who.compare("jane") == 0)
			individual_buffers->at(JANE_ID)->push_back(r);
		else cout << "oh shit theres an error" << endl;
	}
	
	//pw.serv_recv->~RequestChannel();
	//~(pw.serv_recv);
	//pw.wc->send_request("quit");
}

void* stat_thread_function(void* arg) {
	const unsigned int JOE_ID = 0;
	const unsigned int JOHN_ID = 1;
	const unsigned int JANE_ID = 2;
	
	
	PARAMS_STAT ps = *((PARAMS_STAT*)arg);
	
	string s;
	int num;
	
	int id = ps.id;
	vector<int>* hist = ps.h;
	bounded_buffer* ind_b = ps.ib;
	
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


/*
void* request_thread_function(void* id) {
	int request_id = *((int*)id);
	string req_str = "data ";
	
	switch (request_id) {
		case JOE_ID:
			req_str += "joe";
			break;
		case JOHN_ID:
			req_str += "john";
			break;
		case JANE_ID:
			req_str += "jane";
			break;
		default:
			req_str = "done";
			break;
	}	
	for (int i = 0; i < n; i++)
		main_buffer->push_back(req_str);
}

void* worker_thread_function(void* arg) {
	RequestChannel* w_chan = (RequestChannel*)arg;
	
	string response;
	
	bool running = true;
	
	while (running) {
		response = main_buffer->retrieve_front();
		
		if (response.compare("done") == 0)
			break;
		
		std::string r = w_chan->send_request(response);
		std::string t = response.substr(5);
		
		if (t.compare("joe") == 0)
			individual_buffers.at(JOE_ID)->push_back(r);
		else if (t.compare("john") == 0)
			individual_buffers.at(JOHN_ID)->push_back(r);
		else if (t.compare("jane") == 0)
			individual_buffers.at(JANE_ID)->push_back(r);
		else cout << "oh shit theres an error" << endl;
	}
	w_chan->send_request("quit");
}

void* stat_thread_function(void* arg) {
	int id = *((int*)arg);
	string s;
	int num;
	for (int i = 0; i < n; i++) {
		switch (id) {
			case JOE_ID:
				s = individual_buffers.at(JOE_ID)->retrieve_front();
				num = atoi(s.c_str());
				histograms.at(JOE_ID).at(num)++;
				break;
			case JOHN_ID:
				s = individual_buffers.at(JOHN_ID)->retrieve_front();
				num = atoi(s.c_str());
				histograms.at(JOHN_ID).at(num)++;
				break;
			case JANE_ID:
				s = individual_buffers.at(JANE_ID)->retrieve_front();
				num = atoi(s.c_str());
				histograms.at(JANE_ID).at(num)++;
				break;
			default:
				cout << "oh shit theres an error pt 2" << endl;
				break;
		}
	}
}
*/

//runs cleanup script
void cleanup() {
	pid_t cleanup_pid = fork();
	if (cleanup_pid == 0) {
		char* command = (char*)"sh";
		char* flags[] = {(char*)"sh", (char*)"cleanup.sh", NULL};
		execvp(command, flags);
	}
	else wait(NULL);
	
	mq_unlink("/serv_recv");	
	mq_unlink("/serv_send");
}

//makes cleanup script
void make_cleanup() {
	ofstream cleanofs;
	cleanofs.open("cleanup.sh");
	cleanofs << "rm -rf *.o fifo*" << endl;
	//cleanofs << "&killall dataserver" << endl;
	//cleanofs << "&killall client" << endl;
	cleanofs.close();
	
	int status;
	
	pid_t chmod_pid = fork();
	if (chmod_pid == 0) {
		char* chmod = (char*)"chmod";
		char* chmod_flags[] = {(char*)"chmod", (char*)"+x", (char*)"cleanup.sh", NULL};
		int er = execvp(chmod, chmod_flags);
		perror("couldnt chmod");
		exit(er);
	}
	else {
		int st = wait(&status);
		if (st == chmod_pid)
			cleanup();
		else cerr << "failed to create cleanup script" << endl;
		
	}
}

void log_stats(float time, int n, int b, int w) {
	ofstream stat_file;
	stat_file.open("client_stats.dat", ios::app);
	stat_file << n << "," << b << "," << w << "," << time << endl;
	stat_file.close();
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/
int main(int argc, char * argv[]) {
	
	make_cleanup();
	
    int n = 10; //default number of requests per "patient"
    int b = 50; //default size of request_buffer
    int w = 10; //default number of worker threads
    bool USE_ALTERNATE_FILE_OUTPUT = false;
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:b:w:m:h")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg);
                break;
            case 'm':
                if(atoi(optarg) == 2) USE_ALTERNATE_FILE_OUTPUT = true;
                break;
            case 'h':
            default:
                cout << "This program can be invoked with the following flags:" << endl;
                cout << "-n [int]: number of requests per patient" << endl;
                cout << "-b [int]: size of request buffer" << endl;
                cout << "-w [int]: number of worker threads" << endl;
                cout << "-m 2: use output2.txt instead of output.txt for all file output" << endl;
                cout << "-h: print this message and quit" << endl;
                cout << "Example: ./client_solution -n 10000 -b 50 -w 120 -m 2" << endl;
                cout << "If a given flag is not used, a default value will be given" << endl;
                cout << "to its corresponding variable. If an illegal option is detected," << endl;
                cout << "behavior is the same as using the -h flag." << endl;
                exit(0);
        }
    }

    int pid = fork();
	if (pid > 0) {
        struct timeval start_time;
        struct timeval finish_time;
        int64_t start_usecs;
        int64_t finish_usecs;
        ofstream ofs;
        if(USE_ALTERNATE_FILE_OUTPUT) ofs.open("output2.txt", std::ios::out | std::ios::app);
        else ofs.open("output.txt", std::ios::out | std::ios::app);
		
        cout << "n == " << n << endl;
        cout << "b == " << b << endl;
        cout << "w == " << w << endl;

        cout << "CLIENT STARTED:" << endl;
        //cout << "Establishing control channel... " << flush;
		
        RequestChannel *chan = new RequestChannel("/control", RequestChannel::CLIENT_SIDE);
        		
		//cout << "done." << endl;
		
		timeval begin, end;
		gettimeofday(&begin, NULL);
		
		
		int num_clients = 3; //to make it easier to add more clients in future
		
		//vectors to hold the request threads
		vector<pthread_t> request_threads(num_clients);
		vector<pthread_t> worker_threads(w);
		vector<pthread_t> stats_threads(num_clients);
		
		bounded_buffer* main_buffer = new bounded_buffer(b);
		
		vector<bounded_buffer*> individual_buffers;
		
		//paramater vectors
		vector<PARAMS_REQUEST*> request_params(num_clients);
		vector<PARAMS_STAT*> stat_params(num_clients);
		vector<PARAMS_WORKER*> worker_params(w);
	
		//creates the request threads for number of clients
		for (int i = 0; i < num_clients; i++) {
			request_params.at(i) = new PARAMS_REQUEST(n, b, w, i, main_buffer);
			pthread_create(&request_threads.at(i), NULL, request_thread_function, (void*)request_params.at(i));
		}
		
		
		//creats the individual buffers
		for (int i = 0; i < num_clients; i++)
			individual_buffers.push_back(new bounded_buffer(100));
		
		//creates request channels for each worker thread, and then creates the threads
		for (int i = 0; i < w; i++) {
			//string r = chan->send_request("newthread");
			RequestChannel* serv_recv = new RequestChannel("/serv_recv", RequestChannel::CLIENT_SIDE);
			RequestChannel* serv_send = new RequestChannel("/serv_send", RequestChannel::CLIENT_SIDE);
			vector<bounded_buffer*>* t_ind_buf = &individual_buffers;
			worker_params.at(i) = new PARAMS_WORKER(n, b, w, serv_send, serv_recv, main_buffer, t_ind_buf);
			chan->cwrite("newthread");
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
		
		//sends the terminate signals to all the worker threads
		for (int i = 0; i < w; i++)
			main_buffer->push_back("done");
		
		//waits for the worker threads to join
		for (int i = 0; i < w; i++)
			pthread_join(worker_threads.at(i), NULL);
		
		//waits for the stats threads to join
		for (int i = 0; i < num_clients; i++)
			pthread_join(stats_threads.at(i), NULL);
			
		std::cout << make_histogram("Joe", histograms.at(0));
		std::cout << make_histogram("John", histograms.at(1));
		std::cout << make_histogram("Jane", histograms.at(2));

		gettimeofday(&end,NULL);
	
		float timetaken=(float)(end.tv_sec-begin.tv_sec)+(float)((float)(end.tv_usec-begin.tv_usec)/1000000);
		
		cout << "time taken  (sec): " << timetaken << endl;
		
        ofs.close();
        
		
		chan->cwrite("quit");
		
		cout << "See ya" << endl;

		cleanup();
		
		log_stats(timetaken, n, b, w);
    }
	else if (pid == 0) {
		execl("./dataserver", NULL);		
	}
}