/**
 * Author: Jiri Matejka
 */
#include <iostream>
#include <fstream>
#include <mpi.h>
#include <sstream>
#include <algorithm>
#include <math.h>

const int BAD_ALLOCK = 1;
const int BAD_PROC_COUNT = 2;
const int FILE_ERROR = 3;

const int NUMBER_TAG = 1;

/**
 * Finds out size of a file.
 * @param  filename name of file
 * @return size of file. If size is 0 or its unable to open file, returns 0.
 */
size_t getFileSize( const char * filename );

/**
 * Reads file content into array.
 * @param array    array with allocated memory for content
 * @param size     size of bytes to be read
 * @param filename name of file
 * @return true, if number of bytes read is equal to size. Otherwise false.
 */
bool readBytes( unsigned char * array, size_t size, const char * filename );

/**
 * Merges 2 sorted arrays into one sorted array
 * @param arr1   1st array to be merged
 * @param arr2   2nd array to be merged
 * @param size   size of 1st and 2nd array
 * @param result array with allocated memory where result will be stored
 */
void merge( unsigned char * arr1, unsigned char * arr2, size_t size, unsigned char * result );

/**
 * Sequence sort algorithm.
 * @param array array that will be sorted
 * @param size  size of array
 */
void shellSort(unsigned char * array, size_t size);
int expectedProcCount( size_t fileSize, int procCount );

template<typename T1, typename T2>
void DEBUG_INLINE(T1 x, T2 y) {
    std::cerr << x << y;
}

template<typename T1, typename T2>
void DEBUG_LINE(T1 x, T2 y) {
    std::cerr << x << y << std::endl;
}

template<typename T1, typename T2, typename T3>
void DEBUG_LINE(T1 x, T2 y, T3 z) {
    std::cerr << x << y << z << std::endl;
}

template<typename T1, typename T2, typename T3>
void DEBUG_INLINE(T1 x, T2 y, T3 z) {
    std::cerr << x << y << z;
}