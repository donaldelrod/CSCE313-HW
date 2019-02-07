
#include <stdio.h>
#include <string.h>

/* List of Commands to Implement:
	ps
	head
	tail
	dd
	pwd
	mv
	cd
	rm
	jobs
	sleep
	awk
	cat
*/

struct shell_command {
	char* comm_name;
	char** flags;
	int num_flags;
	bool background;
	shell_command* pipeline_from;
	shell_command* pipeline_to;
	shell_command* redirect_from;
	shell_command* redirect_to;
	
	shell_command init_shell_command() {
		shell command new_shell = malloc(shell_command);
		new_shell.num_flags = 1;
		new_shell.flags[0] = "doesn't matter";
		new_shell.pipeline_from = NULL;
		new_shell.pipeline_to = NULL;
		new_shell.redirect_from = NULL;
		new_shell.redirect_to = NULL;
		return new_shell;
	}
	
	void print_command() {
		
		
	}
}


int main(int argc, char** argv) {
	
	bool running = true;
	
	while (running) {
		cout << ">> ";
		char* command;
		cin >> command;
		shell_command* comm_list = parse_command(command);
		signed int proc_code;
		if (fork() == 0)
			signed int proc_code = start_process(comm_list);
		if (proc_code == -1)
			cout << endl << "Incorrect usage of command: try again" << endl;
	}
}

signed int start_process(shell_command* comm_list) {//, int fd) {
	/*
	int len = sizeof(comm_list) / sizeof(comm_list[0]);
	int child_status;
	int fds[2];
	pipe(fds);
	
	int process_id = fork();
	
	if (process_id == 0) {//child process
		for (int i = len-1; i >= 0; i++) {
			
			int pid
			
		
		
		}
	}
	*/
	
	
	int len = sizeof(comm_list) / sizeof(comm_list[0]);
	int child_status;
	int fds[2];
	pipe(fds);//0 is std in, 1 in std out
	
	int num_instr = length(comm_list);
	
	
	while (num_instr > 0) {
		
		num_instr--;
		
		shell_command thiscommand = comm_list[num_instr];
		
		int pid = fork();
		
		if (pid == 0) {
			
			if (num_instr != 0) {//if there are more instructions, this child will become the parent of the next child
				continue;
			}
			else {
				//dup2(fd[0], 0);
				dup2(fd[1], 1);
				execv(thiscommand.name, thiscommand.flags);
				return 0;
			}			
		}
		else {
			wait(&child_status);
			
			dup2(fd[0], 0);
			dup2(fd[1], 1);
			
			execv(thiscommand.name, thiscommand.flags);
			
			return 0;
		}
		
	}
	
	/*
	int process_id = fork();
	
	if (process_id == 0) {//child process
		//dup2(fd, 0);
		dup2(fds[1], 1);
		//if (level == len-1) { //if this is the base command
		execv(comm_list[level].comm_name, comm_list[level].flags);
		//}
	}
	else if (process_id < 0)
		return process_id;
	else { //parent process
		wait(&child_status);
		start_process(comm_list, fd[0]);
		//dup2(fd[0], 0);
		
		//if (!background) {
		//	wait(&child_status);
		//	execv(comm_list[level].comm_name, comm_list[level].flags);
		//}
		//else return 0;
	}
	*/
	return 0;
	
}

//parses the input into separated commands
shell_command* parse_command(char* command) {
	
	char* comm_parts;
	comm_parts = strtok(command, " "); //starts parsing command, with spaces as separators
	shell_command comm_list*;		//list of shell_command objects
	unsigned int comm_index = 0;	//number of commands that exist
	if (comm_parts != NULL)
		comm_list[comm_index].comm_name = comm_parts;
	else return -1;
	
	bool background = false;
	int num_commands = 1;
	
	while ((comm_parts = strtok(NULL, " ")) != NULL) {
		if (strcmp(comm_parts, "|")) {
			shell_command next = init_shell_command();//malloc(sizeof(shell_command));
			
			comm_list[comm_index].pipeline_to = next;
			next.pipeline_from = comm_list[comm_index];
			
			comm_parts = strtok(NULL, " ")
			if (comm_parts != NULL)
				next.comm_name = comm_parts;
			else return -1;
			
			comm_index++;
			comm_list[comm_index] = next;
			num_commands++;
			continue;
		}
		else if (strcmp(comm_parts, ">")) { //if redirecting from or to a file
			num_commands++;
			continue;
		}
		else if (strcmp(comm_parts, "&")) {
			background = true;
		}
		else { //adds flag to the shell_command
			int numf = comm_list[comm_index];
			comm_list[comm_index].flags[numf+1] = comm_parts;
			comm_list[comm_index].flags++;
		}
	}
	if (background) {
		for (int i = 0; i < num_commands; i++) {
			comm_list[i].background = true;
		}
	}
	return comm_list;
}