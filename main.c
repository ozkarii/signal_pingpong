#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/*
PARENT receives SIGUSR1
CHILD receives SIGUSR2
*/

#define HELP_STR "Usage: pingpong [<options>]\n    --copies, -c        Amount of child process copies to be created\n"
#define SLEEP_TIME_US 50000

pid_t parent_pid;
int num_children = 0;
int sigusr1_count = 0;
int parent_running_flag = 1;
pid_t* child_processes = NULL;


// Send SIGUSR2 to all children, call after all SIGUSR1s have been received from kids
void kill_children() 
{
    for (int i = 0; i < num_children; i++) {
        usleep(SLEEP_TIME_US);
        printf("Parent sending #%d SIGUSR2 to PID %d...\n", i, child_processes[i]);
        if (!kill(child_processes[i], SIGUSR2) == 0) {
            printf("Error in parent sending SIGUSR2 to PID %d", child_processes[i]);
        }
    }
    parent_running_flag = 0;
}

// Parent process receives SIGUSR1 from children
void sigusr1_handler(int signum) 
{
    // Should only be used by parent
    if (getpid() != parent_pid) {
        return;
    }

    sigusr1_count++;
    printf("SIGUSR1 #%d received by parent PID %d\n", sigusr1_count, getpid());

    if (sigusr1_count == num_children) {
        printf("Start killing children... ;)\n");
        kill_children();
    }
}

// Child receives SIGUSR2 and exits
void sigusr2_handler(int signum) 
{
    if (getpid() == parent_pid) {
        return;
    }
    printf("SIGUSR2 received by PID %d, exiting...\n", getpid());
    free(child_processes);
    exit(0);
}


int main(int argc, char** argv) 
{
    parent_pid = getpid();
    pid_t pid = 0; // PID to be used for getting child PIDs

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
    }
    else {
        printf("Invalid option\n");
        printf(HELP_STR);
    }

    num_children = atoi(argv[2]);
    child_processes = malloc(sizeof(pid_t) * num_children);

    // Set SIGUSR1 handler for parent
    struct sigaction sa1;
    sa1.sa_handler = &sigusr1_handler;
    sigaction(SIGUSR1, &sa1, NULL);

    for (int i = 0; i < num_children; i++) {
        usleep(SLEEP_TIME_US);
        pid = fork();
        // if child process, stop forking
        if (pid == 0) {
            struct sigaction sa2;
            sa2.sa_handler = &sigusr2_handler;
            sigaction(SIGUSR2, &sa2, NULL);
            printf("PID %d sending SIGUSR1 to parent PID %d...\n", getpid(), parent_pid);
            kill(parent_pid, SIGUSR1);
            pause();
            break;
        }
        else {
            printf("PID %d created\n", pid);
            child_processes[i] = pid;
        }
    }
    
    if (pid != 0) {
        printf("All children spawned, last PID: %d\n", pid);
    }

    while (parent_running_flag) {
        pause();
    }

    free(child_processes);

    return 0;
}
