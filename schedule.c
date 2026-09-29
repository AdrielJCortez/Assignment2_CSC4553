#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// program should form a series of child processes, a child process for each process defined in an input file, adding them into a pqueue
// process with higher priority should run to completetion 
// proccesses with the same priority should circulate in a RR using time quantum intervals
// .\schedule.exe 500 processes.tsv
// gcc -Wall -Wextra -o schedule schedule.c
int main(int argc, char *argv[]){

    if (argc != 2){
        printf("Incorrect amount of arguments")
        return 1;
    }

    int timeQuantum = atoi(argv[1]);
    char* fileName = argv[2];
    FILE *fp;
    
    // buffer to store each line of the file
    char line[256];

    fp = fopen(fileName, "r");

    if (fp == NULL){
        perror("Error");
        return 1;
    }

    else {
        while (fgets(line, sizeof(line), fp)) {
            // see if I get each line correct
            // printf("%s", line);

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

            }

        }

        fclose(fp);
    }

    return 0;
    // read line by line each line will have
    // Process, Priority, process binary file, optional params
    // EX:
    // 8    6   demo    5
    return 0;
}