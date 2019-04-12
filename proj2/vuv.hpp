#include <iostream>
#include <mpi.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <map>

typedef std::map<char, bool> argSet;

enum retValues {
    BAD_ALLOCK = 1,
    BAD_PROC_COUNT,
    INVALID_ARGUMENTS
};

enum tags {
    WEIGHT_TAG = 1,
    SUCC_TAG,
    PREV_TAG,
    NODE_TAG,
};

inline int getDestNode( int procId, int len ) {
    return ( ++procId >= len ) ? ( procId - len + 2 ) >> 1 : 1 + procId;
}

inline int getSrcNode( int procId, int len ) {
    return ( ++procId >= len ) ? procId - len + 2 : ++procId >> 1;
}

inline int getSucc( int procId, int destNode, int len ) {
    if ( procId + 1 >= len ) {
        return procId % 2 ? procId - destNode - 1 : procId - len + 2;
    }
    else {
        return destNode << 1 > len ? procId + len - 1 : destNode + procId;
    }
}

inline int getPrev( int procId, int srcNode, int len ) {
    if ( procId + 1 >= len ) {
        int tmp = procId + 1 + srcNode;
        return tmp >= (len << 1) - 1 ? procId - len + 1 : tmp;
    }
    else {
        return procId % 2 ? procId + len - 2 : procId - srcNode;
    }
}

inline int getWeight( int procId, int len ) {
    return procId + 1 >= len ? 1 : -1;
}

template<typename T2, typename T3>
void DEBUG_LINE( int N, T2 y, T3 z) {
    std::cerr << N << y << z << std::endl;
}

template<typename T1, typename T2>
void DEBUG_LINE(T1 x, T2 y) {
    std::cerr << x << y << std::endl;
}