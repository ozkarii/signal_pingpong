#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define HELP_STR "Usage: pingpong [<options>]\n\n    --copies, -c        Amount of child process copies to be created\n"

int num_children = 0;
int sigusr1_count = 0;
int running_flag = 1;
pid_t* child_processes = NULL;

// Send SIGUSR2 to all children, call after all SIGUSR1s have been received from kids
void kill_children() {
    for (int i = 0; i < num_children; i++) {
        if (kill(child_processes[i], SIGUSR2) == 0) {
            printf("Sent SIGUSR2 to PID %d\n", child_processes[i]);
        }
        else {
            printf("Error sending SIGUSR2 to PID %d", child_processes[i]);
        }
    }
    running_flag = 0;
}

// Parent process receives SIGUSR1 from children
void sigusr1_handler(int signum) {
    if (signum != SIGUSR1) {
        printf("wtf\n");
        return;
    }
    printf("SIGUSR1 received by PID=%d\n", getpid());
    sigusr1_count++;

    if (sigusr1_count == num_children) {
        kill_children();
    }
}

// Child receives SIGUSR2 and exits
void sigusr2_handler(int signum) {
    if (signum != SIGUSR2) {
        printf("wtf\n");
        return;
    }
    printf("SIGUSR2 received by PID=%d\n", getpid());

    free(child_processes);
    exit(0);
}


int main(int argc, char** argv) {
    
    pid_t parent_pid = getpid();
    pid_t pid = 0; // pid to be used for getting child pids

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

    // Set signal handler for SIGUSR1 which comes from the children
    signal(SIGUSR1, sigusr1_handler);

    for (int i = 0; i < num_children; i++) {
        pid = fork();
        if (pid == 0) {
            break;
        }
        else {
            child_processes[i] = pid;
        }
    }
    
    if (pid != 0) {
        signal(SIGUSR2, sigusr2_handler);
    }
    
    // Send SIGUSR1 to parent if this process is a child
    if (pid == 0) {
        kill(parent_pid, SIGUSR1);
    }

    while (running_flag);

    free(child_processes);

    return 0;
}
