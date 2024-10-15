#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/*
22 Signal pingpong

Authors: Oskari Heinonen & Rasmus Salmi

*/

#define HELP_STR "Usage: pingpong [<options>]\n    --copies, -c        Amount of child process copies to be created\n"
#define SLEEP_TIME_US 50000

pid_t parent_pid;              // process id of parent
int num_children = 0;          // number of children to be created
int sigusr1_count = 0;         // amount of signals parent has received from children
int parent_running_flag = 1;   // used for exiting from parent process, always 1 for children
pid_t* child_processes = NULL; // dynamically allocated array of child PIDs


// Sends SIGUSR2 to all children, call after all SIGUSR1s have been received from kids
void kill_children() 
{
    for (int i = 0; i < num_children; i++) {
        usleep(SLEEP_TIME_US);
        printf("Parent sending #%d SIGUSR2 to PID %d...\n", i + 1, child_processes[i]);
        // Kill syscall used to send signal to child processes
        if (!kill(child_processes[i], SIGUSR2) == 0) {
            printf("Error in parent sending SIGUSR2 to PID %d", child_processes[i]);
        }
    }
    parent_running_flag = 0;
}

void sigint_handler(int signum)
{
	if (getpid() == parent_pid) {
		printf("SIGINT RECEIVED BY PARENT\n");
        exit(1);
    }
    else {
    	printf("SIGINT RECEIVED BY CHILD\n");
    	kill(parent_pid, SIGINT);
    }
}

// Handler for receiving SIGUSR1 from children
void sigusr1_handler(int signum) 
{
    // Should only be used by parent
    if (getpid() != parent_pid) {
        return;
    }

    sigusr1_count++;
    printf("SIGUSR1 #%d received by parent PID %d\n", sigusr1_count, getpid());

    // if signal received from all children
    if (sigusr1_count == num_children) {
        printf("Start killing children... ;)\n");
        kill_children();
    }
}

// Handler for SIGUSR2 signal received by children. Exit point for children.
void sigusr2_handler(int signum) 
{   
    // Should only be used by children
    if (getpid() == parent_pid) {
        return;
    }
    printf("SIGUSR2 received by PID %d, exiting...\n", getpid());
    free(child_processes); // free memory from each child's PID array
    exit(0);
}


int main(int argc, char** argv) 
{
    parent_pid = getpid();

    if (argc < 2) {
        printf("No parameters given.\n");
        printf(HELP_STR);
        return 1;
    }
    else if (!strcmp(argv[1], "-c") || !strcmp(argv[1], "--copies")) {
        if (argc < 3) {
            printf("Give the amout of child processes to spawn\n");
            printf(HELP_STR);
            return 1;
        }
        num_children = atoi(argv[2]);
		if (num_children < 1) {
			printf("Invalid number of children to spawn\n");
			return 1;
		}
    }
    else {
        printf("Invalid option\n");
        printf(HELP_STR);
        return 1;
    }

	pid_t pid = 0; // PID to be used for getting child PIDs
    child_processes = malloc(sizeof(pid_t) * num_children);

    // Set SIGUSR1 handler for parent. 
    // Using POSIX function sigaction() instead of signal().
    struct sigaction sa1;
    sa1.sa_handler = &sigusr1_handler;
    sigaction(SIGUSR1, &sa1, NULL);

    struct sigaction sa_int;
    sa_int.sa_handler = &sigint_handler;
    sigaction(SIGINT, &sa_int, NULL);

    for (int i = 0; i < num_children; i++) {
        // Put parent process to sleep to give time for previous child to send signal
        // before creating another one.
        usleep(SLEEP_TIME_US);
        pid = fork(); // using fork to start a new process from here
        // if child process, stop forking
        if (pid == 0) {
            struct sigaction sa2;
            sa2.sa_handler = &sigusr2_handler;
            sigaction(SIGUSR2, &sa2, NULL);
            printf("PID %d sending SIGUSR1 to parent PID %d...\n", getpid(), parent_pid);
            kill(parent_pid, SIGUSR1); // send signal with kill syscall
            pause(); // pause child until a signal is received
            break;
        }
        else {
            printf("PID %d created\n", pid);
            child_processes[i] = pid; // store newly created child's PID to the array
        }
    }
    
    if (pid != 0) {
        printf("All children spawned, last PID: %d\n", pid);
    }

    // Parent pauses until kill_children() unsets the flag.
    // Also, if the pause of child is interrupted above by someone else
    // than parent, the child will pause here
    while (parent_running_flag) {
        pause();
    }

    free(child_processes);

    printf("All children killed, parent exiting...\n");

    return 0;
}
