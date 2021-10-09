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

#define P_READ 0
#define P_WRITE 1

#define STD_IN 0
#define STD_OUT 1

void run_two(char* args1[], char* args2[], int fd[]) {
    int pid_inner = fork();
    cout << "cmd = 1!" << endl;
    
    if (pid_inner == 0) {
        // inner child process
        // first process : ls -la / 
        cout << "inner child: " << pid_inner << endl;
        cout << "\targs: " << args1[0] << endl;
        dup2(fd[P_WRITE], STD_OUT);
        close(fd[P_READ]);
        close(fd[P_WRITE]);
        cout << "HERE IS MORE" << endl;
        
        execvp(args1[0], args1);
    } else {
        // inner "parent" process
        // second process : grep dev
        cout << "inner parent: " << pid_inner << endl;
        cout << "\targs: " << args2[0] << endl;
        waitpid(pid_inner, 0, 0);
        dup2(fd[P_READ], STD_IN);
        close(fd[P_READ]);
        close(fd[P_WRITE]);
        // string s;
        // while (getline(cin, s)) {
        //     cout << s << endl;
        // }
        // cout << "AFTER READING STDIN: "<< endl;
        execvp(args2[0], args2);
        // exit(0);
    }
}

void run_n(char* args1[], int fd[], int i, int stop) {
    cout << "\t(" << i << "/" << stop << ")" << endl;
    int pid_inner = fork();
    printf("FORKED\n");
    

    
    if (pid_inner == 0) {
        // inner child process
        // first process : ls -la / 
        // if (i == 1) {
        //     string s;
        //     while (getline(cin, s)) {
        //         cout << s << endl;
        //     }
        //     cout << "AFTER READING STDIN of CHILD: "<< endl;
        //     // exit(0);
        // }
        cout << "inner child: " << pid_inner << endl;
        cout << "\targs: " << args1[0] << endl;
        if (i < stop - 1) {
            dup2(fd[P_WRITE], STD_OUT);
        }
        close(fd[P_READ]);
        close(fd[P_WRITE]);
        cout << "HERE IS MORE" << endl;
        
        execvp(args1[0], args1);
    } else {
        // inner "parent" process
        // second process : grep dev
        cout << "inner parent: " << pid_inner << endl;
        // if (i == stop - 1) {
            waitpid(pid_inner, 0, 0);
        // }
        
        dup2(fd[P_READ], STD_IN);
        close(fd[P_READ]);
        close(fd[P_WRITE]);
        if (i == 1) {
            exit(0);
        }
        // exit(0);
        // string s;
        // while (getline(cin, s)) {
        //     cout << s << endl;
        // }
        // cout << "AFTER READING STDIN of PARENT: "<< endl;
        // exit(0);
    }
}



int main (){


    // int READ_END = 0;
    // int READ_END = 1;
    int fd[2];

    string s = "ps aux | awk '/init/{print $1}' | sort -r";

    char* args1[] = { "ls", "-la", "/", NULL };
    char* args2[] = { "grep", "dev", NULL };
    char* args3[] = { "awk", "'{print $1, $2, $3, \"YOU DID IT\"}'", NULL };
    char* echo_args1[] = {"echo", "'test1'", NULL};
    char* echo_args2[] = {"echo", "'test2'", NULL};


    while (true){
        // cout << "My Shell$ ";
        string inputline;
        // getline (cin, inputline);   // get a line from standard input
        // vector<int[]> arr_pipes;

        if (inputline == string("exit")){
            cout << "Bye!! End of shell" << endl;
            break;
        }
        // inputline = args;
        

        int pid = fork();
        if (pid == 0){ // child process
            // preparing the input command for execution
            
            cout << "outer child: " << pid << endl;
            
            string cmd;
            cout << "enter the number of commands you want" << endl;
            getline(cin, cmd);
            int STOP = atoi(cmd.c_str());
            cout << "RUNNING WITH STOP = " << STOP << endl;
            for (int i = 0; i < STOP; ++i) {
                
                pipe(fd);

                // if (i == 1) {
                //     string s;
                //     while (getline(cin, s)) {
                //         cout << s << endl;
                //     }
                //     cout << "AFTER READING STDIN of CHILD: "<< endl;
                // }
                cout << "fd: " << fd[0] << ", " << fd[1] << endl;
                cout << "IN LOOP" << endl;
                if (cmd == "1") {
                    run_two(args1, args2, fd);
                }
                else if (cmd == "2") {
                    if (i == 0) {
                        run_n(args1, fd, i, STOP);
                    }
                    else if (i == 1) {
                        run_n(args2, fd, i, STOP);
                    }
                    else if (i == 2) {
                        run_n(args3, fd, i, STOP);
                    }
                    
                } else {
                    cout << "doing nothing > :\) " << endl;
                }
            }
            
        
        }else{
            cout << "parent: " << pid << endl;
            waitpid (pid, 0, 0); // wait for the child process 
            
            // we will discuss why waitpid() is preferred over wait()
        }
        // exit(0);
        cout << "Input to continue" << endl;
        getline(cin, inputline);
        
    }
}