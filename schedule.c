#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
// program should form a series of child processes, a child process for each process defined in an input file, adding them into a pqueue
// process with higher priority should run to completetion
// proccesses with the same priority should circulate in a RR using time quantum intervals
// .\schedule.exe 500 processes.tsv
// gcc -Wall -Wextra -o schedule schedule.c

struct Process {
    int id;
    int priority;
    pid_t pid;
    char **args;
    int numArgs;
    int done;
};

volatile sig_atomic_t timeUp = 0;
volatile pid_t currentPid = 0;

void alarmHandler(int sig) {
    (void)sig;
    timeUp = 1;
    if (currentPid > 0) {
        kill(currentPid, SIGSTOP);
    }
}

int main(int argc, char *argv[]){

    if (argc != 3){
        printf("Incorrect amount of arguments\n");
        return 1;
    }

    int timeQuantum = atoi(argv[1]);
    char* fileName = argv[2];
    FILE *fp;

    struct Process *processes = NULL;
    int numProcesses = 0;

    // buffer to store each line of the file
    char line[1024];

    fp = fopen(fileName, "r");

    if (fp == NULL){
        perror("Error");
        return 1;
    }

    else {
        while (fgets(line, sizeof(line), fp)) {
            // see if I get each line correct
            // printf("%s", line);

            char *token = strtok(line, "\t\r\n");
            if (token == NULL) {
                continue;
            }

            struct Process newProcess;
            newProcess.id = atoi(token);
            newProcess.priority = atoi(strtok(NULL, "\t\r\n"));

            newProcess.args = malloc(sizeof(char *));
            newProcess.numArgs = 0;
            newProcess.done = 0;

            token = strtok(NULL, "\t\r\n");
            while (token != NULL) {
                int len = strlen(token);
                if (len >= 2 && token[0] == '"' && token[len - 1] == '"') {
                    token[len - 1] = '\0';
                    token++;
                }
                newProcess.args = realloc(newProcess.args, (newProcess.numArgs + 2) * sizeof(char *));
                newProcess.args[newProcess.numArgs] = strdup(token);
                newProcess.numArgs++;
                token = strtok(NULL, "\t\r\n");
            }
            newProcess.args[newProcess.numArgs] = NULL;

            fflush(stdout);

            // fork for each line
            pid_t pid = fork();

            // error
            if (pid < 0) {
                perror("fork failed");
                exit(1);
            }
            // child
            else if (pid == 0) {
                // 1. Stop
                raise(SIGSTOP);
                execv(newProcess.args[0], newProcess.args);
                perror("exec failed");
                exit(1);
            }

            int status;
            waitpid(pid, &status, WUNTRACED);
            newProcess.pid = pid;

            processes = realloc(processes, (numProcesses + 1) * sizeof(struct Process));
            int i = numProcesses;
            while (i > 0 && processes[i - 1].priority > newProcess.priority) {
                processes[i] = processes[i - 1];
                i--;
            }
            processes[i] = newProcess;
            numProcesses++;
        }

        fclose(fp);
    }

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = alarmHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGALRM, &action, NULL);

    struct itimerval timer;
    struct itimerval offTimer;
    memset(&timer, 0, sizeof(timer));
    memset(&offTimer, 0, sizeof(offTimer));
    timer.it_value.tv_sec = timeQuantum / 1000;
    timer.it_value.tv_usec = (timeQuantum % 1000) * 1000;

    int groupStart = 0;
    while (groupStart < numProcesses) {
        int groupEnd = groupStart;
        while (groupEnd < numProcesses && processes[groupEnd].priority == processes[groupStart].priority) {
            groupEnd++;
        }

        int running = groupEnd - groupStart;
        while (running > 0) {
            for (int i = groupStart; i < groupEnd; i++) {
                if (processes[i].done) {
                    continue;
                }

                int status;
                timeUp = 0;
                currentPid = processes[i].pid;
                kill(currentPid, SIGCONT);
                setitimer(ITIMER_REAL, &timer, NULL);

                waitpid(currentPid, &status, WUNTRACED);

                setitimer(ITIMER_REAL, &offTimer, NULL);
                currentPid = 0;

                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    processes[i].done = 1;
                    running--;
                }
            }
        }

        groupStart = groupEnd;
    }

    for (int i = 0; i < numProcesses; i++) {
        for (int j = 0; j < processes[i].numArgs; j++) {
            free(processes[i].args[j]);
        }
        free(processes[i].args);
    }
    free(processes);

    return 0;
    // read line by line each line will have
    // Process, Priority, process binary file, optional params
    // EX:
    // 8    6   demo    5
    return 0;
}
