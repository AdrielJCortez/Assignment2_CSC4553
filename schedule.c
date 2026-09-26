#include <stdio.h>
#include <stdlib.h>
// program should form a series of child processes, a child process for each process defined in an input file, adding them into a pqueue
// process with higher priority should run to completetion 
// proccesses with the same priority should circulate in a RR using time quantum intervals

int main(int argc, char *argv[]){
    int timeQuantum = atoi(argv[1]);
    char* fileName = argv[2];
    // check for content in the File

    // read line by line each line will have
    // Process, Priority, process binary file, optional params
    // EX:
    // 8    6   demo    5
    return 0;
}