#include "bks.h"
const int BAD_ALLOCK = 1;
const int BAD_PROC_COUNT = 2;

const int NUMBER_TAG = 1;

#include <unistd.h> // TODO

size_t getFileSize( const char * filename ) {
    std::streampos fsize = 0;
    std::ifstream file( filename, std::ios::binary );
    if ( file.is_open() ) {
        fsize = file.tellg();
        file.seekg( 0, std::ios::end );
        fsize = file.tellg() - fsize;
        file.close();
    }
    return fsize;
}

void readBytes( unsigned char * array, size_t size, const char * filename ) {
    std::ifstream file( filename, std::ios::binary );
    if ( file.is_open() ) {
        for( size_t i = 0; i < size; i++ ) {
            if ( file.good() ) {
                array[ i ] = file.get();
            }
            else {
                break;
            }
        }
        file.close();
    }
}

void merge( unsigned char * arr1, unsigned char * arr2, size_t size, unsigned char * result ) {
    size_t i = 0, j = 0, k = 0;
    while ( i < size && j < size ) {
        if ( arr1[ i ] < arr2[ j ] ) {
            result[ k++ ] = arr1[ i++ ];
        }
        else {
            result[ k++ ] = arr2[ j++ ];
        }
    }
    
    while ( i < size ) {
        result[ k++ ] = arr1[ i++ ];
    }
    
    while ( j < size ) {
        result[ k++ ] = arr2[ j++ ];
    }
}    

void shellSort(unsigned char * array, size_t size) {
    size_t j;
    unsigned char tmp;
    for (size_t gap = ( size >> 1 ); gap > 0; gap >>= 1 ) {
        for (size_t i = gap; i < size; i++) {
            tmp = array[ i ];
            for ( j = i; j >= gap && array[ j - gap ] > tmp; j -= gap ) {
                array[ j ] = array[ j - gap ];
            }
            array[ j ] = tmp;
        }
    }
}

int expectedProcCount( size_t fileSize, int procCount ) {
    int logn = ( log2( fileSize ) + 0.5 );
    int nextPower = 1;
    if ( logn > 1 ) {
        nextPower++;
        fileSize--;
        while ( logn >>= 1 ) {
            nextPower <<= 1;
        }
    }
    return 2 * nextPower - 1;
}

int main( int argc, char **argv ) {
    int procCount, nameSize, procId, treeLevel, myLevel;;
    char procName[ MPI_MAX_PROCESSOR_NAME ];
    MPI::Init();
    MPI_Comm_size( MPI_COMM_WORLD, &procCount );
    MPI_Get_processor_name( procName, &nameSize );
    procId = MPI::COMM_WORLD.Get_rank();
    
    size_t listCount = ( procCount + 1 ) / 2;
    size_t fileSize;
    if ( procId == 0 ) {
        fileSize = getFileSize( argv[1] );
        int expected = expectedProcCount( fileSize, procCount );
        if ( expected != procCount ) {
            std::cerr << "Invalid number of processes, expected: " << expected << ", got " << procCount << std::endl; 
            MPI_Abort(MPI_COMM_WORLD, BAD_PROC_COUNT);
        }
    }
    MPI_Bcast(&fileSize, sizeof( fileSize ), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    if ( fileSize == 0 ) {
        MPI::Finalize();
        return 0;
    }
    
    size_t baseBucketSize = ( fileSize % listCount == 0 ) ? ( fileSize / listCount ) : ( ( fileSize / listCount ) + 1 );
    size_t bucketSize = baseBucketSize; 
    bool listProc = procId >= static_cast<int>( procCount - listCount );
    if ( !listProc ) {
        treeLevel  = log2( listCount ) + 1;
        myLevel    = log2( procId + 1 );
        bucketSize = pow( 2, treeLevel - myLevel - 1) * baseBucketSize;
    }
    
    unsigned char * bucketData = nullptr;
    try { 
        bucketData = new unsigned char[ bucketSize ];
    } catch ( std::bad_alloc & ba ) { 
        std::cerr << "Unable to allocate memory for bucket: " << ba.what(); 
        MPI_Abort(MPI_COMM_WORLD, BAD_ALLOCK);
    }

    if ( procId == 0 ) {
        readBytes( bucketData, fileSize, argv[1] );
        for ( size_t i = fileSize; i < bucketSize; i++ ) {
            bucketData[ i ] = 255;
        }
        for ( size_t i = 0; i < fileSize; i++ ) {
            std::cout << +bucketData[ i ] << ((i + 1 == fileSize) ? "" : " ");
        }
        
        std::cout << std::endl;
        
        double t1, t2; 
        t1 = MPI_Wtime();
        
        size_t dest = procCount - 1;
        size_t start = 0;
        for( size_t j = 0; j < listCount; j++ ) {
            MPI_Send( bucketData + start, baseBucketSize, MPI_UNSIGNED_CHAR, dest-- , NUMBER_TAG, MPI_COMM_WORLD);
            start += baseBucketSize;
        }
        
        size_t readSize = bucketSize >> 1;
        MPI_Recv( bucketData, readSize, MPI_UNSIGNED_CHAR, 1, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        MPI_Recv( bucketData + readSize, readSize, MPI_UNSIGNED_CHAR, 2, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        unsigned char * sorted = nullptr;
        try { 
            sorted = new unsigned char[ bucketSize ];
        } catch ( std::bad_alloc & ba ) { 
            std::cerr << "Unable to allocate memory for bucket: " << ba.what(); 
            MPI_Abort(MPI_COMM_WORLD, BAD_ALLOCK);
        }
        
        merge(bucketData, bucketData + readSize, readSize, sorted);
        
        t2 = MPI_Wtime();
        std::cerr << fileSize << ";" << t2 -t1 << std::endl;
        for ( size_t i = 0; i < fileSize; i++ ) {
            std::cout << +sorted[ i ] << std::endl;
        }
        delete [] sorted;
    }
    else {
        int merger = ( procId % 2 ) == 0 ? procId / 2 -1 : procId / 2;
        if ( listProc ) {
            MPI_Recv( bucketData, bucketSize, MPI_UNSIGNED_CHAR, 0, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            shellSort( bucketData, bucketSize );
            MPI_Send( bucketData, bucketSize, MPI_UNSIGNED_CHAR, merger, NUMBER_TAG, MPI_COMM_WORLD);
        }
        else {
            int recvFrom1  = procId * 2 + 1;
            int recvFrom2  = recvFrom1 + 1;
            size_t readSize = bucketSize >> 1;
            MPI_Recv( bucketData, readSize, MPI_UNSIGNED_CHAR, recvFrom1, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            MPI_Recv( bucketData + readSize, readSize, MPI_UNSIGNED_CHAR, recvFrom2, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            
            unsigned char * sorted = nullptr;
            try { 
                sorted = new unsigned char[ bucketSize ];
            } catch ( std::bad_alloc & ba ) { 
                std::cerr << "Unable to allocate memory for bucket: " << ba.what(); 
                MPI_Abort(MPI_COMM_WORLD, BAD_ALLOCK);
            }
            merge(bucketData, bucketData + readSize, readSize, sorted);
            MPI_Send( sorted, bucketSize, MPI_UNSIGNED_CHAR, merger, NUMBER_TAG, MPI_COMM_WORLD);
            delete [] sorted;
        }
    }
    delete [] bucketData;
    MPI::Finalize();
    return 0;
}