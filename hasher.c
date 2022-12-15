/* hasher.c
 * Anthony Gatti - 11/7/22
 * Comp 398 - Systems and Multiprogramming
 * Description: A hasher that takes 3 inputs
 * Input file: Input file with what you want to hash
 * Writing End Pipe: An integer that describes the
 * writing end of the pipe being passed by main
 * Number of zeros: The number of zeros wanted in
 * the prefix of the binary hash
 * This program concatenates the input file contents,
 * a 6 digit random number, and a 6 digit count for
 * number of hashes tried. This becomes the buffer 
 * that is pushed through sha_256 which results in 
 * a hash value in hexidecimal. This value is then 
 * converted to binary to try and find enough
 * zeroes in the prefix of this binary hash.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "sha-256.h"

void hexToBin(char* hexHash, char* binHash);

void createRandomNum(struct timeval randTime,
int randNum, char* randNumS, char* buf, char* randCount, int passToPipe[]);

int startHashing(int i, char* buf, char* bufbuf, char* randCount, 
char* randNumS, char* hash, char* binHash, int numZeros, int passToPipe[]);

void getFromFile(FILE* fp, char* fileName, char* buf);

int pipeNum; // Global var

void handler(int sig) { // Signal Handler
  printf("This program terminated by SIGINT\n");
  fflush(stdout);
  close(pipeNum);
  exit(1);
}

int main(int argc, char* argv[]) {
    FILE* fp; // Initalize file for input file
    char *buf;
    buf = (char*) malloc(1000013); // Initialize buffer
    char *bufbuf;
    bufbuf = (char*) malloc(1000013); // Initialize buffer for the buffer
    char *randCount;
    randCount = (char*) malloc(13); // Output string with random num and count
    int randNum; // Initialize int used to store a random number
    char randNumS[7]; // Hold the random number as a string
    int passToPipe[2]; // Holds the random number and count
    char hash[65]; // Initilize the hash
    char binHash[1000000] = ""; // Initialize the hash in binary
    int numZeros; // The number of zeroes wanted in the prefix of binHash
    int countZeros = 0; // A count of how many zeroes in the prefix of binHash
    struct timeval randTime; // Initialize time to use to seed the random number
    numZeros = atoi(argv[3]); // Store the number of zeros wanted
    pipeNum = atoi(argv[2]); // Store the pipe number for this process
    char* fileName = argv[1]; // Filename of input file
    int i = 0; // Iterators used for for loops

    signal(SIGINT, handler); // Start the signal handler

    // Check if file
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Improper filename\n");
        return 2; // Returns 2 if no file exists
    }

    getFromFile(fp, fileName, buf); // Stores file contents in buf

    //Infinite Loop
    while (1 == 1) {
        createRandomNum(randTime, randNum, randNumS, buf, randCount, 
        passToPipe);
        //Count from 0 to 999999 inclusive looking for a hash
        for (i = 0; i < 1000000; i++) {
            countZeros = startHashing(i, buf, bufbuf, randCount, randNumS, 
            hash, binHash, numZeros, passToPipe); // num zeros in prefix binhash
            if (countZeros == numZeros) { // Check if the hash is valid
                write(pipeNum, &passToPipe, sizeof(int)*2);
                close(pipeNum);
                free(buf); // Deallocate buf
                free(bufbuf); // Deallocate bufbuf
                free(randCount); // Deallocate randCount
                fclose(fp); // Close input file
                printf("Solution Found\n"); // Print that a solution was found
                fflush(stdout);
                return 0;
            }
        }
    }
}

/**
* Converts the hexadecimal hash to binary
* @param hexHash a string of the hash in hexadecimal
* @param binHash a string of the hash in binary
* @return void
* @throws no errors
*/
void hexToBin(char* hexHash, char* binHash) {
    int i; // Iterator for for loops

    // Converts each hex value to binary
    for(i = 0; i < strlen(hexHash); i++) {
        switch(hexHash[i]) {
            case '0':
                strcat(binHash, "0000");
                break;
            case '1':
                strcat(binHash, "0001");
                break;
            case '2':
                strcat(binHash, "0010");
                break;
            case '3':
                strcat(binHash, "0011");
                break;
            case '4':
                strcat(binHash, "0100");
                break;
            case '5':
                strcat(binHash, "0101");
                break;
            case '6':
                strcat(binHash, "0110");
                break;
            case '7':
                strcat(binHash, "0111");
                break;
            case '8':
                strcat(binHash, "1000");
                break;
            case '9':
                strcat(binHash, "1001");
                break;
            case 'a':
            case 'A':
                strcat(binHash, "1010");
                break;
            case 'b':
            case 'B':
                strcat(binHash, "1011");
                break;
            case 'c':
            case 'C':
                strcat(binHash, "1100");
                break;
            case 'd':
            case 'D':
                strcat(binHash, "1101");
                break;
            case 'e':
            case 'E':
                strcat(binHash, "1110");
                break;
            case 'f':
            case 'F':
                strcat(binHash, "1111");
                break;
            default:
                printf("Invalid hexadecimal input.");
        }
    }
}

/**
* Creates a random number and appends to buf
* @param randTime the current time in computer time
* @param randNum an integer random 6 digit number
* @param randNumS a string version of randNum
* @param buf a string that stores input, 6 digit random
* @return void
* @throws no errors
*/
void createRandomNum(struct timeval randTime,
int randNum, char* randNumS, char* buf, char* randCount, int passToPipe[]) {
    gettimeofday(&randTime, 0); // Get current computer time
    srand((randTime.tv_sec * 1000000) + (randTime.tv_usec));
    randNum = rand() % 1000000;

    //Convert randNum to a string
    sprintf(randNumS, "%d", randNum);

    //Add zeros if needed
    if (strlen(randNumS) < 6) {
        sprintf(randNumS, "%0*d%d", (int)(6-strlen(randNumS)), 0, randNum);
    }

    //Append randNumS
    passToPipe[0] = randNum;
    sprintf(buf, "%s%s", buf, randNumS);
    sprintf(randCount, "%s%s", randCount, randNumS);
}

/**
* Starts the hashing process and gets the zeros in prefix of binHash
* @param i an integer for the current iteration of hashes
* @param buf a string that stores input, 6 digit random
* @param bufbuf a string to store buf plus 6 digit count
* @param hash a string that holds bufbuf once hashed to hexadecimal
* @param binHash a string that holds hash in binary
* @param numZeros an integer that tells how many zeros to look for
* @return integer number of zeros found
* @throws no errors
*/
int startHashing(int i, char* buf, char* bufbuf, char* randCount, 
char* randNumS, char* hash, char* binHash, int numZeros, int passToPipe[]) {
    int k = 0; // Iterator for for loops
    int countZeros = 0; // Number of zeros found
    char iToString[7]; // char array to hold int i as a string
    int trueFalse = 0; // Start as false

    iToString[0] = '\0'; // Just to make sure
    sprintf(iToString, "%d", i); // Convert i to string
    if (strlen(iToString) < 6) { // Append 0's to prefix if less than 6 digits
        sprintf(iToString, "%0*d%d", (int)(6-strlen(iToString)), 0, i);
    }
    sprintf(bufbuf, "%s%s", buf, iToString); // Append count to buf
    sprintf(randCount, "%s%s", randNumS, iToString); // Add count to randCount
    passToPipe[1] = i;
    sha_256_string(hash, bufbuf, strlen(bufbuf)); // Hash
    hexToBin(hash, binHash); // Convert to binary
    while (trueFalse != 1 && k < strlen(binHash) && countZeros < numZeros) {
        if (binHash[k] == '0') {
            countZeros++;
            k++;
        } else {
            strncpy(binHash, "", 1000000);
            trueFalse = 1;
        }
    }

    return countZeros;
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

    // Pull each character from file and put in buffer
    while (fscanf(fp, "%c", &c) == 1)
    {
        buf[i] = c;
        i++;
    }
    buf[i] = '\0'; // End with null terminator
}