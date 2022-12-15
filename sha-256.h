#ifndef __SHA256_H
#define __SHA256_H
#include <stdint.h>

void calc_sha_256(uint8_t hash[32], const void *input, size_t len);

/*****************************************************************************************
 * Computes the sha-256 hash of the bytes contained in the array input
 * @params string: an array of characters in which the hash is written in hexadecimal digits
 *                           the array is null-terminated.
 * @params input: an array of bytes in which the input of the hash is contained
 * @params len: an integer containing the number of bytes of input to hash 
 ******************************************************************************************
 */
void sha_256_string(char string[65], const void *input, size_t len);

#endif
