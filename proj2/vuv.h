/**
 * Author: Jiri Matejka
 * Login: xmatej52
 * Modified: 13. 04. 2019
 */
#include <iostream>
#include <mpi.h>
#include <string.h>
#include <unistd.h>
//#include <map>

/// datatype for checking arguments
// typedef std::map<char, bool> argSet;

/// Return values of app
enum retValues {
    BAD_ALLOCK = 1,
    BAD_PROC_COUNT,
    INVALID_ARGUMENTS
};

/// communication tags for MPI
enum tags {
    WEIGHT_TAG = 1,
    SUCC_TAG,
    PREV_TAG,
    NODE_TAG,
};

/**
 * Retrieve destination node as number of specific process
 * @param  procId Id of process
 * @param  len    Number of nodes in tree
 * @return        Index of node in tree (indexes starts with 1)
 */
inline int getDestNode( int procId, int len ) {
    return ( ++procId >= len ) ? ( procId - len + 2 ) >> 1 : 1 + procId;
}

/**
 * Retrieve source node as number of specific process
 * @param  procId Id of process
 * @param  len    Number of nodes in tree
 * @return        Index of node in tree (indexes starts with 1)
 */
inline int getSrcNode( int procId, int len ) {
    return ( ++procId >= len ) ? procId - len + 2 : ++procId >> 1;
}

/**
 * Get successor of specific process
 * @param  procId   Id of process
 * @param  destNode Index of destination node
 * @param  len      Number of nodes in tree
 * @return          Id of successor process
 */
inline int getSucc( int procId, int destNode, int len ) {
    if ( procId + 1 >= len ) {
        return procId % 2 ? procId - destNode - 1 : procId - len + 2;
    }
    else {
        return destNode << 1 > len ? procId + len - 1 : destNode + procId;
    }
}

/**
 * Get predecessor of specific process
 * @param  procId  Id of process
 * @param  srcNode Index of source node
 * @param  len     Number of nodes in tree
 * @return         Id of predecessor process
 */
inline int getPrev( int procId, int srcNode, int len ) {
    if ( procId + 1 >= len ) {
        int tmp = procId + 1 + srcNode;
        return tmp >= (len << 1) - 1 ? procId - len + 1 : tmp;
    }
    else {
        return procId % 2 ? procId + len - 2 : procId - srcNode;
    }
}

/**
 * Gets starting weight of specific process
 * @param  procId Id of process
 * @param  len    Number of nodes in tree
 * @return        Defaukt weight of process
 */
inline int getWeight( int procId, int len ) {
    return procId + 1 >= len ? 1 : -1;
}

/**
 * Gets nearest power of 2 greater then len
 * @param  len number
 * @return     nearest power of 2 greater then len
 */
int nextPower( int len );

/**
 * Checks program arguments
 * @param  argc      Number of program arguments
 * @param  argv      Array of arguments
 * @param  procCount Number of processes
 * @return           values from retValues enumerator
 */
int checkArguments( int argc, const char ** argv, unsigned procCount );

template<typename T2, typename T3>
void DEBUG_LINE( int N, T2 y, T3 z) {
    std::cerr << N << y << z << std::endl;
}

template<typename T1, typename T2>
void DEBUG_LINE(T1 x, T2 y) {
    std::cerr << x << y << std::endl;
}