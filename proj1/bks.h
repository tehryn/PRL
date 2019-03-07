#include <iostream>
#include <fstream>
#include <mpi.h>
#include <sstream>
#include <algorithm>
#include <math.h>

template<typename T1>
void DEBUG_INLINE(T1 x) {
    std::cerr << x;
}

template<typename T1, typename T2>
void DEBUG_INLINE(T1 x, T2 y) {
    std::cerr << x << y;
}

template<typename T1>
void DEBUG_LINE(T1 x) {
    std::cerr << x << std::endl;
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