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


using namespace std;

int main() {
	int cn = 1;
	int cb = 1;
	int cw = 1;
    int cbacklog = 1;
	
	int n_max = 10000;
	int b_max = 51;
	int w_max = 51;
    int backlog_max = 100;
    int port = 2000;
    string host = "127.0.0.1";
	
    char* nf = (char*)"-n"; //number of requests
    char* bf = (char*)"-b"; //buffer size
    char* wf = (char*)"-w"; //number of workers
    char* pf = (char*)"-p"; //port number of server
    char* hf = (char*)"-h"; //domain name of server
    char* qf = (char*)"-q"; //turn output on or off
    char* sf = (char*)"-s"; //backlog of server listening port (for logging)
    char* lf = (char*)"-l"; //whether or not to log the runtime stats

    char* ps = (char*)to_string(port).c_str();
    char* hs = (char*)host.c_str();
    char* qs = (char*)"0"; //quiet = true;
    char* ls = (char*)"1"; //logging = true
    char* command = (char*)"./client";
    char* serv_comm = (char*)"./dataserver";
	
	cout << "starting tests" << endl;

    for (int backlog = cbacklog; backlog < backlog_max; backlog += 10) {
        pid_t split = fork();

        char* ss = (char*)to_string(backlog).c_str(); //current backlog for client logging

        if (split != 0) { //if this should be the client
            usleep(1000);
            for (int n = cn; n <= n_max; n*=10) {
                for (int w = cw; w <= w_max; w+=5) {
                    for (int b = cb; b <= b_max; b+=5) {
                        if (w > n*3)
                            break;
                        pid_t pid = fork();
                        if (pid == 0) {
                            cout << "running test with n = " << n << ", b = " << b << ", w = " << w << endl;
                            char* ns = (char*)to_string(n).c_str();
                            char* bs = (char*)to_string(b).c_str();
                            char* ws = (char*)to_string(w).c_str();

                            char* flags[] = {command, nf, ns, bf, bs, wf, ws, pf, ps, hf, hs, qf, qs, sf, ss, lf, ls, '\0'};
                            execvp(command, flags);
                        }
                        else wait(NULL);
                        
                    }
                }
            }
            kill(split, 9);
            wait(NULL);
        }
        else { //if this should start the server
            cout << "starting server with backlog = " << backlog << endl;
            char* backlog_s = (char*)to_string(backlog).c_str();
            char* serv_flags[] = {serv_comm, pf, ps, bf, backlog_s, '\0'};
            execvp(serv_comm, serv_flags);
        }
    }
}