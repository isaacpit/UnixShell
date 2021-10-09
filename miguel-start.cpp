#include <stdio.h>
#include<iostream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

// personally implemented libraries and variables
// the library and vector initializer below exist for background process removal
#include <signal.h>
vector<int> bg_process;

void remove_spaces(string &str){
	// remove spaces from the front
	while (str[0] == ' ') {
		str = str.substr(1, str.length() - 1);
	}

	// remove spaces from the back
	while (str.back() == ' ') {
		str = str.substr(0, str.length() - 1);
	}
}

// function to split a given string by a given delimiter
// TODO: include single quotes to this list
// TODO: bug with only one quotation still letting echo work
vector<string> split(string str, char del){
	//cout << "Running split function(delimiter '" << del << "'): ";
	vector<string> string_vec;
	string temp;
	bool ignore_del = false;
	
	// split the string by its given delimiter
	for (int i = 0; i < str.size(); i++){
		// check for quotations in the text for ignoring the delimiter or not
		if((str[i] == '"') && !(ignore_del)){
			ignore_del = true;
		} else if ((str[i] == '"') && (ignore_del)){
			ignore_del = false;
		} 
		
		if((str[i] != del) || (ignore_del)){
			temp += (1, str[i]);
		} else{
			// removing any existing spaces on the front and back of commands
			remove_spaces(temp);
			
			// prevent adding an empty string to the vector of strings
			if (temp != "") {
				//cout << temp << " ";
				string_vec.push_back(temp);
				temp = "";
			}
		}
    }
	// add the last section of the string to the string vector
	//cout << temp << ";" << endl;
	string_vec.push_back(temp);
	
	return string_vec;
}
   
// function to find I/O redirection characters in a given command, and return -1 if not found
int find_IO(vector<string> split_cmd, string IO){
	for (int i = 0; i < split_cmd.size(); i++) {
		if (split_cmd[i] == IO) {
			return i;
		}
	}
	return -1;
}
   
// function to execute any cd command
void cd_cmd(vector<string> split_cmd, string &prev_dir, string &curr_dir){
	int cd_error = -1;
	// execute the cd command
	if (split_cmd[1] == "-") {
		cd_error = chdir(prev_dir.c_str());
	} else{
		cd_error = chdir(split_cmd[1].c_str());
	}
	
	// output any existing errors, otherwise the current working directory
	if (cd_error < 0) {
		cout << "Error: Requested directory not found" << endl;
	} else{
		prev_dir = curr_dir;
		curr_dir = string(getcwd(NULL, 100));
		//cout << "Previous directory: " << prev_dir << endl;
		cout << "Current working directory: " << curr_dir << endl;
	}
}
   
// function to execute a given command
void execute_cmd(vector<string> split_cmd){
	// creating args array from split_cmd and NULL
	int arr_size = split_cmd.size() + 1;
	char* args [arr_size];
	for (int i = 0; i < arr_size - 1; i++){
		// removes quotations from beginning and end of string (for nicer echo command)
		if (split_cmd[i][0] == '"') {
			split_cmd[i] = split_cmd[i].substr(1, split_cmd[i].length() - 2);
		}
		args[i] = (char *) split_cmd[i].c_str();
	}
	args[arr_size - 1] = NULL;
	
	// preparing the input command for execution 
	int exe_error = execvp(args [0], args);
	if (exe_error < 0){
		cout << "Error: Command could not be run" << endl;
	}
}
   
// function to process a given command
void process_command(string command){
	cout << "Running Command: " << command.c_str() << endl;
	
	// split the command by spaces
	vector<string> split_cmd = split(command, ' ');
	
	int pid = fork ();
    if (pid == 0){ //child process
        // creating args array from split_cmd and NULL
		int arr_size = split_cmd.size() + 1;
		char* args [arr_size];
		for (int i = 0; i < arr_size - 1; i++){
			// removes quotations from beginning and end of string (for nicer echo command)
			if (split_cmd[i][0] == '"') {
				split_cmd[i] = split_cmd[i].substr(1, split_cmd[i].length() - 2);
			}
			args[i] = (char *) split_cmd[i].c_str();
		}
		args[arr_size - 1] = NULL;
		
		// preparing the input command for execution 
        execvp (args [0], args);
    } else{
		// wait for the child process 
        waitpid (pid, 0, 0); 
    }
}

// function to kill zombie child processes
void kill_zombies(int) {
	int pid;
	// iterate through zombie processes and erase them
	while ((pid = waitpid(0, 0, WNOHANG)) > 0){
		//cout << "PID of process removed: " << pid << endl;
		// remove the pid from the vector containing pid of all background processes
		for (int i = 0; i < bg_process.size(); i++){
			if (bg_process[i] == pid) {
				bg_process.erase(bg_process.begin() + i);
			}
		}
	}
}

int main (){
	// get the current working directory of the shell script
	string curr_dir = string(getcwd(NULL, 100));
	// variable to store previous working directory
	string prev_dir = string(getcwd(NULL, 100));
	
    // Shell script
	while (true){
		// get the current time
		// TODO: retrieve timezone from script
		time_t now = time(0);
		char* dt = strtok(ctime(&now), "\n");
		cout << "[" << dt << "] ";

		string inputline;
		
		// finding the tail folder of the current directory
		string dir(curr_dir);
		string dir_tail = split(dir, '/').back();
		
		// shell standard input prompt
		cout << "neone@comp:~/" << dir_tail << "$ ";
		string input_line;
        getline (cin, input_line);   // get a line from standard input
        
		// exiting the shell upon taking command "exit"
		if (input_line == string("exit")){
            cout << "Exiting the shell" << endl;
            break;
        }
		
		//TODO: pipes and $
		// split the standard input by | symbol
		vector<string> split_line = split(input_line, '|');
		// the below vector is initialized globally now
		//vector<int> bg_process;
		// saving stdin and stdout in order to restore it after redirection
		int save_stdin = dup(0);
		int save_stdout = dup(1);
		int pid_outer = fork();
		if (pid_outer == 0) {
			// outer child 
			for (int i = 0; i < split_line.size(); i++){
				cout << "Running Command: " << split_line[i].c_str() << endl;
				// split the command by spaces
				vector<string> split_cmd = split(split_line[i], ' ');
				
				// setting up boolean variable for background processes "&"
				bool bg = false;
				if (split_cmd.back() == "&"){
					bg = true;
					split_cmd.pop_back();
				}
				
				// setup the pipe
				int fd [2];
				pipe(fd);
				
				int pid_inner = fork();
				if (!pid_inner) { // child process
					cout << "inner child: " << pid_inner << endl;
					if (i < split_line.size() - 1) {
						cerr << "duping input \n\t(" << i <<  "," << split_line.size() << ")" << endl;
						dup2(fd[1], 1);
						// if (i >= 2) {
						// 	exit(0);
						// }
					} 
					close(fd[0]);
					close(fd[1]);
					
					int index_output = find_IO(split_cmd, "<");
					// support output redirection
					if (index_output > 0){
						cout << "Performing the executable command on file: " << split_cmd[index_output + 1] << endl;
						
						// creating new file to store execvp outputs onto
						int fd_output = open(split_cmd[index_output + 1].c_str(), O_RDONLY);
						dup2(fd_output, 0);
						close(fd_output);
						
						// remove < and the following filename from the command
						split_cmd.erase(split_cmd.begin() + index_output + 1);
						split_cmd.erase(split_cmd.begin() + index_output);
					}
					
					int index_input = find_IO(split_cmd, ">");
					// support input redirection
					if (index_input > 0){
						cout << "Saving the output of the exectuable command into file: " << split_cmd[index_input + 1] << endl;
						
						// creating new file to store execvp outputs onto
						int fd_input = open(split_cmd[index_input + 1].c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
						dup2(fd_input, 1);
						close(fd_input);
						
						// remove > and the following filename from the command
						split_cmd.erase(split_cmd.begin() + index_input + 1);
						split_cmd.erase(split_cmd.begin() + index_input);
					}
					
					if (split_cmd[0] == "cd"){
						// execute the cd command
						cd_cmd(split_cmd, prev_dir, curr_dir);
					} else {
						// execute the current command
						execute_cmd(split_cmd);
					}
					
					// resetting the input to stdin after < redirection
					if (index_output > 0){
						dup2(save_stdin, 0);
						close(save_stdin);
					}
					// resetting the output to stdout after > redirection
					if (index_input > 0){
						dup2(save_stdout, 1);
						close(save_stdout);
					}
					
					//if (i < split_line.size() - 1) {
					//	dup2(0, 1);
					//	close(fd[0]);
					//}
				} else{ // child parent process
					// calling waitpid on the last process in the standard input
					if (!bg){ // for normal processes ((i == split_line.size() - 1) && (!bg))
						cout << "inner parent: " << pid_inner << endl;
						if (i < split_line.size() - 1) {
							// duping input
							dup2(fd[0], 0);
						}
						
						close(fd[0]);
						close(fd[1]);
						waitpid(pid_inner, 0, 0);

						if (i >= split_line.size() - 1) {
							cout << "EXITING: " << i << "," << split_line.size() << endl;
							exit(0);
						}

					} else if ((i == split_line.size() - 1) && (bg)){ // for background processes ((i == split_line.size() - 1) && (bg))
						//waitpid(pid, 0, WNOHANG);
						bg_process.push_back(pid_inner);
						bg = false;
					}
					
					//dup2(fd[0], 0);
					//close(fd[1]);
				}
			}
			
			// check for zombie processes and kill them
			if (bg_process.size()){
				//cout << "Number of zombie child processes: " << bg_process.size() << endl;
				signal(SIGCHLD, kill_zombies);
			}

		} else {
			// outer parent

			cout << "outer_parent: " << pid_outer << endl;
        	waitpid (pid_outer, 0, 0); // wait for the child process 

		}
		
	}
}