#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define HELP_STR "Usage: pingpong [<options>]\n\n    --copies, -c        Amount of child process copies to be created\n"


int main(int argc, char** argv) {
    
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


    //pid_t pid = fork();

    return 0;
}