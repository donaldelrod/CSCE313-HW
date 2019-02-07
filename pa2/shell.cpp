#include <stdio.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <bits/stdc++.h>
#include <stdexcept>



using namespace std;

class shell {
public:
	string command;
	vector<string> flags;

	shell() {//sets the first command to an empty string, because thats how execvp works
		flags.push_back(" ");
	}

	void setCommand(string comm) {
		command = comm;
	}

	void addFlag(string newFlag) {
		flags.push_back(newFlag);
	}

	char* getCommand() {
		int len = command.length();
		char* comm = new char[len];
		comm = strcpy(comm, command.c_str());
		return comm;
	}

	char** getFlags() {
		char** f = new char*[flags.size()];
		for (int i = 0; i < flags.size(); i++) {
			int len = flags.at(i).length();
			char* f1 = new char[len+1];
			f1 = strcpy(f1, flags.at(i).c_str());
			f[i] = f1;
		}
		return f;
	}

	void printCommand() {
		cout << "Command: " << command << "\t Flags: ";
		for (int i = 0; i < flags.size(); i++)
			cout << flags.at(i) << " ";
		cout << endl;
	}
};

bool checkForOutRedirect(string in) {
	int pos = in.find(" > ");
	if (pos == -1)
		return false;
	else return true;
}

bool checkForInRedirect(string in) {
	int pos = in.find(" < ");
	if (pos == -1)
		return false;
	else return true;
}

//separates the command name from the flags
shell parseCommand(string s) { 

	shell c_comm = shell();

	int pos = s.find(" ");
	int loop = 0;
	int c = 0;

	bool parsing = true;
	int eol = 0;

	while (parsing) {
		if (loop++ == 0)
			c_comm.setCommand(s.substr(0, pos));
		else
			c_comm.addFlag(s.substr(0, pos));
		s = s.erase(0, pos + 1);
		pos = s.find(" ");
		if (eol > 0)
			parsing = false;
		if (pos == -1)
			eol++;
	}

	return c_comm;
}

//splits the input into multiple commands
vector<shell> parseInput(string in) { 
	vector<shell> in_comm = vector<shell>();
	signed int pos = in.find(" | ");
	int c = 0;
	bool parsing = true;
	int eol = 0;

	bool redir_out = checkForOutRedirect(in);
	bool redir_in = checkForInRedirect(in);

	while (parsing) {//pos > 0 && c++ < 10) { //runs while there are still additional commands being piped to each other
		
		if (eol > 0)
			break;

		in_comm.push_back(parseCommand(in.substr(0, pos)));
		in_comm.at(c++).printCommand();
		
		
		if (pos == -1)
			eol++;

		in = in.erase(0, pos + 3);
		pos = in.find(" | ");
	}

	return in_comm;
}

//executes the commands in the vector comm_list
signed int executeProcess(vector<shell> comm_list, vector<int*> fds) { 

	shell current_comm = comm_list.at(comm_list.size()-1);

	for (int i = comm_list.size() - 1; i > 0; i--) {
		int c_status = 0;

		
		int pid = fork();

		if (pid == 0) { //child process
			current_comm = comm_list.at(i-1);
			if (i-1 == 0) { //if this is the first command entered
				//this should execute this as the first command, and it will be the last and most grand child of all processes
				//it will pass its output to fds[1]
				cout << "currently executing command " << current_comm.command << " " << current_comm.getFlags()[1] << endl;
				int *tfds = fds.at(i - 1);
				close(tfds[0]); //closes input of base input pipe

				cout << "child fds writing to pipe " << tfds[1] << endl;

				dup2(tfds[1], 1); //pipes output to tfds[1]
				
				//close(tfds[1]); //closes output pipe
				cerr << "process " << getpid() << " is finishing" << endl;
				execvp(current_comm.getCommand(), current_comm.getFlags());
				//at this point the process is replaced by the execvp shell call, and the program will terminate
				//allowing it's parents to run their shell commands
			}
			else { 
				//if there are commands that need to run before this one, this will continue the loop
				//and then fork it again, making this child a parent in its own right
				//cout << "continuing for some reason, even though i-1 = " << i - 1 << endl;
				continue;
			}
		}
		else { //parent process
			
			cout << "parent process " << current_comm.command << " " << current_comm.getFlags()[1] << " is waiting for child process ";
			cout << comm_list.at(i - 1).command << " " << comm_list.at(i-1).getFlags()[1] << ", which is process " << pid << endl;

			int *tfds1 = fds.at(i - 1);
			close(tfds1[1]); //closes the write pipe of child
			dup2(tfds1[0], 0); //replace stdin with child pipe out

			wait(&c_status);
			//waitpid(pid, &c_status, 1);


			cout << "done waiting... now executing command: " << current_comm.command << " " << current_comm.getFlags()[1] << endl;
			cout << "parent fds reading from pipe " << tfds1[0] << endl;

			
			if (i != comm_list.size() - 1) {
				int *tfds2 = fds.at(i);
				dup2(tfds2[1], 1); //replaces stdout with tfds2[1] which will be used by parent to read in
				close(tfds2[0]); //closes the read pipe of this process
				cout << "parent fds writing to pipe " << tfds2[1] << endl;
			}


			
			
			
			

			//string b;
			//while (getline(cin, b))
			//	cout << b << endl;

			//close(tfds1[0]);
				

			//close(tfds2[1]);

			
			cerr << "running parent exec" << endl;
			cerr << "process " << getpid() << " is finishing" << endl;
			execvp(current_comm.getCommand(), current_comm.getFlags());
			cerr << "theres a mother fuckin error you bitch" << endl;
			
			break;
		}
	}

	exit(0);
}


int main(int argv, char** args) {
	bool running = true;

	cout << endl << "Shell program by Donald Elrod" << endl << "Start typing commands below" << endl << endl << endl;

	while (running) {
		string raw;
		cout << ">> ";
		getline(cin, raw);
		//printf("input: %s\n", raw.c_str());
		vector<shell> command_list = parseInput(raw);


		int num_pipes = command_list.size() - 1;
		vector<int*> fds = vector<int*>();
		for (int i = 0; i < num_pipes; i++) {
			int* tempfds = new int[2];
			pipe(tempfds);
			//cout << tempfds[0] << "\t" << tempfds[1] << endl;
			fds.push_back(tempfds);
		}
		for (int i = 0; i < fds.size(); i++)
			cout << fds.at(i)[0] << "\t" << fds.at(i)[1] << endl;

		int child_status = 0;
		int pid = fork();
		if (pid == 0) { //this is the child process
			int status = executeProcess(command_list, fds);
		}
		else { //this is the parent process
			int *tfds = fds.at(num_pipes - 1);
			
			cout << "this is shell parent, waiting on process " << pid << endl;

			//sleep(2);
			//kill(pid, SIGTERM);
			//waitpid(pid, &child_status, 1);
			wait(&child_status);

			cout << "done" << endl;
			

			//string b;
			//while (getline(cin, b))
			//	cout << b << endl;

			continue;
		}

	}





}