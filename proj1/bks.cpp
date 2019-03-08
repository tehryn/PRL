/**
 * Author: Jiri Matejka
 */
#include "bks.h"

int main( int argc, char **argv ) {
    int procCount, procId, treeLevel, myLevel;;
    MPI::Init();
    
    // number of processes
    MPI_Comm_size( MPI_COMM_WORLD, &procCount );
    
    // id of current process
    procId = MPI::COMM_WORLD.Get_rank();
    
    // number of list processes
    size_t listCount = ( procCount + 1 ) / 2;
    
    // size of file
    unsigned long long fileSize;
    
    // Only root process will read from file
    if ( procId == 0 ) {
        if ( argc == 1 ) {
            fileSize = getFileSize( argv[1] );
        }
        else {
            fileSize = getFileSize( "numbers" );
        }
        int expected = expectedProcCount( fileSize, procCount );
        if ( expected != procCount ) {
            std::cerr << "Invalid number of processes, expected: " << expected << ", got " << procCount << std::endl; 
            MPI_Abort(MPI_COMM_WORLD, BAD_PROC_COUNT);
        }
    }
    
    // broadcast filesize to other processes
    MPI_Bcast(&fileSize, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    
    if ( fileSize == 0 ) {
        if ( procId == 0 ) {
            std::cerr << "Error while reading from file: Unable to open file or file is empty." << std::endl; 
        }
        MPI_Abort(MPI_COMM_WORLD, FILE_ERROR);
    }
    
    // test if current process is list process
    bool listProc = procId >= static_cast<int>( procCount - listCount );
    
    // bucket size for list processes
    size_t baseBucketSize = ( fileSize % listCount == 0 ) ? ( fileSize / listCount ) : ( ( fileSize / listCount ) + 1 );
    
    // bucket size for current process
    size_t bucketSize = baseBucketSize; 
    
    // list processes does not care about their level in tree
    // also their bucketSize was already set
    if ( !listProc ) {
        treeLevel  = log2( listCount ) + 1;
        myLevel    = log2( procId + 1 );
        bucketSize = pow( 2, treeLevel - myLevel - 1) * baseBucketSize;
    }
    
    // allocating data for bucket
    unsigned char * bucketData = nullptr;
    try { 
        bucketData = new unsigned char[ bucketSize ];
    } catch ( std::bad_alloc & ba ) { 
        std::cerr << "Unable to allocate memory for bucket: " << ba.what(); 
        MPI_Abort(MPI_COMM_WORLD, BAD_ALLOCK);
    }

    if ( procId == 0 ) {
        
        // loads input to bucket
        if ( !readBytes( bucketData, fileSize, argv[1] ) ) {
            std::cerr << "Error while reading from file: Was not able to read whole file content." << std::endl; 
            MPI_Abort(MPI_COMM_WORLD, FILE_ERROR);
        }
        
        // adds extra 255 so each bucket will have same size
        for ( size_t i = fileSize; i < bucketSize; i++ ) {
            bucketData[ i ] = 255;
        }
        
        // Printing read sequence to stdout
        for ( size_t i = 0; i < fileSize; i++ ) {
            std::cout << +bucketData[ i ] << ((i + 1 == fileSize) ? "" : " ");
        }
        std::cout << std::endl;
        
        /* TODO - delete */
        double t1, t2; 
        t1 = MPI_Wtime();
        
        // sending data to list processes
        size_t dest = procCount - 1;
        size_t start = 0;
        for( size_t j = 0; j < listCount; j++ ) {
            MPI_Send( bucketData + start, baseBucketSize, MPI_UNSIGNED_CHAR, dest-- , NUMBER_TAG, MPI_COMM_WORLD);
            start += baseBucketSize;
        }
        
        // reading data from node processes
        size_t readSize = bucketSize >> 1;
        MPI_Recv( bucketData, readSize, MPI_UNSIGNED_CHAR, 1, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        MPI_Recv( bucketData + readSize, readSize, MPI_UNSIGNED_CHAR, 2, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        
        // allocating memory for sorted array
        unsigned char * sorted = nullptr;
        try { 
            sorted = new unsigned char[ bucketSize ];
        } catch ( std::bad_alloc & ba ) { 
            std::cerr << "Unable to allocate memory for bucket: " << ba.what(); 
            MPI_Abort(MPI_COMM_WORLD, BAD_ALLOCK);
        }
        
        // merging sorted arrays
        merge(bucketData, bucketData + readSize, readSize, sorted);
        
        /* TODO - delete */
        t2 = MPI_Wtime();
        std::cerr << fileSize << ";" << t2 -t1 << std::endl;
        
        // printing result to stdout
        for ( size_t i = 0; i < fileSize; i++ ) {
            std::cout << +sorted[ i ] << std::endl;
        }
        delete [] sorted;
    }
    else {
        // destination where sorted data will be sent
        int merger = ( procId % 2 ) == 0 ? procId / 2 -1 : procId / 2;
        if ( listProc ) {
            MPI_Recv( bucketData, bucketSize, MPI_UNSIGNED_CHAR, 0, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            shellSort( bucketData, bucketSize );
            MPI_Send( bucketData, bucketSize, MPI_UNSIGNED_CHAR, merger, NUMBER_TAG, MPI_COMM_WORLD);
        }
        else {
            // from who I will read data
            int recvFrom1  = ( procId << 1 ) + 1;
            int recvFrom2  = recvFrom1 + 1;
            
            // size o array where input data will be merged
            size_t readSize = bucketSize >> 1;
            MPI_Recv( bucketData, readSize, MPI_UNSIGNED_CHAR, recvFrom1, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            MPI_Recv( bucketData + readSize, readSize, MPI_UNSIGNED_CHAR, recvFrom2, NUMBER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            
            // allocating memory for array
            unsigned char * sorted = nullptr;
            try { 
                sorted = new unsigned char[ bucketSize ];
            } catch ( std::bad_alloc & ba ) { 
                std::cerr << "Unable to allocate memory for bucket: " << ba.what(); 
                MPI_Abort(MPI_COMM_WORLD, BAD_ALLOCK);
            }
            
            // merging 2 sorted array to new allocated array
            merge(bucketData, bucketData + readSize, readSize, sorted);
            
            MPI_Send( sorted, bucketSize, MPI_UNSIGNED_CHAR, merger, NUMBER_TAG, MPI_COMM_WORLD);
            delete [] sorted;
        }
    }
    delete [] bucketData;
    MPI::Finalize();
    return 0;
}

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

bool readBytes( unsigned char * array, size_t size, const char * filename ) {
    size_t i = 0;
    std::ifstream file( filename, std::ios::binary );
    if ( file.is_open() ) {
        for(; i < size; i++ ) {
            if ( file.good() ) {
                array[ i ] = file.get();
            }
            else {
                break;
            }
        }
        file.close();
    }
    return i == size;
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