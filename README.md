# Multi-Process Hasher

Here is a project I made for Systems and Multiprogramming. 
This is a hasher that takes an input file and appends a nonce, which is then hashed. It will keep incrementing the nonce until a desired prefix of 0's in the hash are found.

To compile: Use terminal command gcc -g -o main main.c hasher.c sha-256.c 
To run: Use terminal command ./main inputFile outputFile numberOfZeros numberOfProcesses
