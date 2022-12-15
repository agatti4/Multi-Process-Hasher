/* main.c
 * Anthony Gatti - 11/7/22
 * Comp 398 - Systems and Multiprogramming
 * Description: The main file that runs the hasher
 * program the number of processes wanted based 
 * on forking and creates a pipe for each process.
 * Inputs:
 * Input file: Input file with what you want to hash
 * Output file: Output file that contains the
 * input file contents, a 6 digit random number,
 * and a 6 digit value for the count of hashes tried
 * Number of zeros: The number of zeros wanted in
 * the prefix of the binary hash
 * Number of processes: The number of processes wanted
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

void getFromFile(FILE* fp, char* fileName, char* buf);

void makePipes(char* numProcS, int* pipeArray[]);

void createForksAndExec(char* numProcS, char* argv[], 
pid_t* childrenID, int* pipeArray[]);

void reapAndOutput(char* numProcS, pid_t* childrenID, char* argv[], 
char* buf, int* pipeArray[]);

int main(int argc, char* argv[]) {
    FILE* fp; // Initalize file for input file
    char* fileName = argv[1]; // Store input file name
    char* numProcS = argv[4]; // Number of processes
    char *buf;
    buf = (char*) malloc(1000013); // Initialize buffer
    pid_t* childrenID = malloc(atoi(numProcS) * sizeof(pid_t)); // child ids
    int i, j; // Iterators for for loops
    int* pipeArray[atoi(numProcS)]; // Pipe numbers array

    // Makes sure there is enough inputs
    if (argc != 5) { // Takes 4 inputs
        printf("Incorrect command line inputs. Enter 3 stuff.\n");
        return 1;
    }

    // Check if file
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Improper filename\n");
        return 2; // Returns 2 if no file exists
    }

    // Allocate memory for the pipe array
    for (i = 0; i < atoi(numProcS); i++) {
        pipeArray[i] = (int*)malloc(2 * sizeof(int));
    }

    getFromFile(fp, fileName, buf); // Stores file contents in buf

    makePipes(numProcS, pipeArray); // Creates pipes

    createForksAndExec(numProcS, argv, childrenID, pipeArray); // Fork + execvp

    reapAndOutput(numProcS, childrenID, argv, buf, pipeArray); // Reap + output

    // Close reading end of each pipe and deallocate space for pipeArray
    for (j = 0; j < atoi(numProcS); j++) {
        close(pipeArray[j][0]);
        free(pipeArray[j]);
    }
    
    fclose(fp); // Close the input file
    free(buf); // Dealloacte space for buf
    free(childrenID); // Deallocate space for childrenID
    return 0;
}

/**
* Pull each character from file and put in buffer
* @param fp is an integer telling how many rows are in the board.
* @param fileName is an integer telling how many columns are in the board.
* @param buf is a character array that represents the board.
* @return void
* @throws no errors
*/
void getFromFile(FILE* fp, char* fileName, char* buf) {
    char c; // Input char for reading from file
    int i = 0; // Iterators used for for loops

    //Pull each character from file and put in buffer
    while (fscanf(fp, "%c", &c) == 1)
    {
        buf[i] = c;
        i++;
    }
    buf[i] = '\0';
}

/**
* Create the pipes for each process
* @param numProcs is a char array storing the number of processes
* @param pipeArray is an integer array storing the pipe numbers
* @return void
* @throws no errors
*/
void makePipes(char* numProcS, int* pipeArray[]) {
    int k; // Iterator for for loops
    int retP; // return value from pipe

    for (k = 0; k < atoi(numProcS); k++) {
        retP = pipe(pipeArray[k]);
        if(retP == -1){
            printf("%s", "Pipe Failed\n");
            exit(1);
        }
    }
}

/**
* Create forks for each process and execvp
* @param numProcs is a char array storing the number of processes
* @param argv is a char array of the command line inputs
* @param childrenID is an integer array storing the children process ids
* @param pipeArray is an integer array storing the pipe numbers
* @return void
* @throws no errors
*/
void createForksAndExec(char* numProcS, char* argv[], 
pid_t* childrenID, int* pipeArray[]) {
    int i; // Iterator for for loops
    pid_t ret; // return value from fork
    char *hashFile = (char []){"./hasher"}; // Program to run
    char** argHasher = malloc(5 * sizeof(char*)); // Array of inputs
    argHasher[0] = hashFile; // Program to run
    argHasher[1] = argv[1]; // Input File
    argHasher[2] = "PipeNum"; // Pipe Number convert to int
    char* numZerosS = argv[3]; 
    argHasher[3] = numZerosS; // Num zeros in prefix of the hash in binary
    argHasher[4] = NULL; // Null for execvp to run
    char placeH[10000] = ""; // Placeholder for pipe number

    // Creates forks based on number of processes inputted
    for (i = 0; i < atoi(numProcS); i++) {
        ret = fork(); // Create fork and store return value
        childrenID[i] = ret; // Store child id
        // printf("%d", ret);
        if (ret == 0) { // If fork was created create new process
            sprintf(placeH, "%d", pipeArray[i][1]); // Create outfile name
            argHasher[2] = placeH;
            execvp(hashFile, argHasher); // Run process
            printf("%s", "Bruh Moment");
        } else {
            close(pipeArray[i][1]);
        }
    }

    free(argHasher); // Deallocate space for argHasher
}

/**
* Reap the children and output to file
* @param numProcs is a char array storing the number of processes
* @param childrenID is an integer array storing the children process ids
* @param argv is a char array of the command line inputs
* @param buf a string to store buf plus 6 digit count
* @param pipeArray is an integer array storing the pipe numbers
* @return void
* @throws no errors
*/
void reapAndOutput(char* numProcS, pid_t* childrenID, char* argv[], 
char* buf, int* pipeArray[]) {
    int j;
    pid_t waitStore; // Process id of the process that finished first
    int hasherOutput[2]; // Value returned from hasher
    FILE* outputF; // Initalize file for output file
    int status; // Status if process finished

    waitStore = wait(&status); // Gets the process id of first finished process
    // Kills each child that isn't the process that finished first
    for (j = 0; j < atoi(numProcS); j++) {
        if (childrenID[j] != waitStore) { // Kill child if not first process
            kill(childrenID[j], SIGINT);
            wait(&status);
        } else { // If first process output what file it stored result in
            read(pipeArray[j][0], &hasherOutput, sizeof(int)*2);
            outputF = fopen(argv[2], "w+");
            sprintf(buf, "%s%06d%06d", buf, hasherOutput[0], hasherOutput[1]);
            fprintf(outputF, "%s", buf); // Put in file
        }
    }

    fclose(outputF); // Close the output file
}