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
	int cn = 10000;
	int cb = 1;
	int cw = 1;
	
	int n_max = 10000;
	int b_max = 75;
	int w_max = 40;
	
	if (fork() == 0) {
		char* rm = (char*)"rm";
		char* rm_flags[] = {(char*)"rm", (char*)"-rf", (char*)"*.png", NULL};
		int rmer = execvp(rm, rm_flags);
		perror("couldnt remove pictures");
		exit(rmer);
	}
	else wait(NULL);
	
	cout << "starting tests" << endl;
	
	for (int n = cn; n <= n_max; n++) {
		for (int w = cw; w <= w_max; w++) {
			for (int b = cb; b <= b_max; b+=2) {
				pid_t pid = fork();
				if (pid == 0) {
					cout << "running test with n = " << n << ", b = " << b << ", w = " << w << endl;
					char* command = (char*)"./client";
					string ns = to_string(n);
					string bs = to_string(b);
					string ws = to_string(w);
					char* flags[] = {(char*)"./client", (char*)"-n", (char*)ns.c_str(), (char*)"-b", (char*)bs.c_str(), (char*)"-w", (char*)ws.c_str(), NULL};
					execvp(command, flags);
				}
				else wait(NULL);
				
			}
		}
	}
	
	char* matlab = (char*)"matlab";
	char* matlab_flags[] = {(char*)"matlab", (char*)"-r", (char*)"client_analyzer", NULL};
	int er = execvp(matlab, matlab_flags);
	perror("couldnt run matlab script");
	exit(er);
}