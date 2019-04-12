#include "vuv.hpp"

int nextPower( int n ) {
    int p = 1;
    if ( n && !( n & ( n - 1 ) ) ) {
        return n;
    }

    while ( p < n ) {
        p <<= 1;
    }

    return p;
}

int checkArguments( int argc, const char ** argv, unsigned procCount ) {
    argSet specified;
    if ( argc != 2 ) {
        return false;
    }
    else {
        int nodes  = ( procCount >> 1 ) + 1;
        int argLen = strlen( argv[1] );
        for( int i = 0; i < argLen; i++ ) {
            if ( specified.find( argv[1][i] ) == specified.end() ) {
                specified[ argv[1][i] ] = true;
            }
            else {
                return INVALID_ARGUMENTS;
            }
        }
        return nodes == nextPower( argLen + 1 ) - 1 ? 0 : BAD_PROC_COUNT;
    }
}

int main( int argc, const char ** argv ) {
    int procCount, procId;
    MPI::Init();

    // number of processes
    MPI_Comm_size( MPI_COMM_WORLD, &procCount );

    // id of current process
    procId = MPI::COMM_WORLD.Get_rank();
    int len = 0;
    int trueLen = 0;

    if ( procId == 0 ) {
        unsigned nodes  = ( procCount >> 1 ) + 1;
        DEBUG_LINE( "NumberOfProcesses: ", procCount );
        DEBUG_LINE( "Nodes: ", nodes );
        DEBUG_LINE( "Input: ", argv[1] );
        int argCheck = checkArguments( argc, argv, procCount );
        if ( argCheck == INVALID_ARGUMENTS) {
            std::cerr << "Invalid program arguments, please specify each node once." << std::endl;
            MPI_Abort( MPI_COMM_WORLD, INVALID_ARGUMENTS );
        }
        else if ( argCheck == BAD_PROC_COUNT ) {
            std::cerr << "Invalid nuber of processes." << std::endl;
            MPI_Abort( MPI_COMM_WORLD, BAD_PROC_COUNT );
        }

        if ( procCount == 1 ) {
            std::cout << argv[1][0] << ":0" << std::endl;
            MPI::Finalize();
            return 0;
        }

        trueLen = strlen( argv[1] );
        len     = nextPower( trueLen + 1 ) - 1;
        DEBUG_LINE(procId, ": trueLen = ", trueLen );
        DEBUG_LINE(procId, ": len = ", len );
        //MPI_Abort( MPI_COMM_WORLD, INVALID_ARGUMENTS );
    }



    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int weight = getWeight( procId, len );
    int destNode = getDestNode( procId, len );
    int srcNode  = getSrcNode( procId, len );
    int succ   = procId == len ? -1 : getSucc( procId, destNode, len );
    int prev   = procId ? getPrev( procId, srcNode, len ) : -1;
    int prevWeight = 0;

/*    usleep( procId * 3000 );
    DEBUG_LINE( procId, ": destNode=", destNode );
    DEBUG_LINE( procId, ": srcNode=", srcNode );
    DEBUG_LINE( procId, ": succ=", succ );
    DEBUG_LINE( procId, ": prev=", prev );
    DEBUG_LINE( procId, ": weight=", weight );
*/
    while ( succ >= 0 || prev >= 0 ) {
        if ( prev != -1 ) {
            MPI_Send( &weight, 1, MPI_INT, prev, WEIGHT_TAG, MPI_COMM_WORLD );
            MPI_Send( &succ, 1, MPI_INT, prev, SUCC_TAG, MPI_COMM_WORLD );
        }

        if ( succ != -1 ) {
            MPI_Send( &prev, 1, MPI_INT, succ, PREV_TAG, MPI_COMM_WORLD );

            //DEBUG_LINE( procId, ": weight=", weight );
            MPI_Recv( &prevWeight, 1, MPI_INT, succ, WEIGHT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            weight += prevWeight;
            //DEBUG_LINE( procId, ": weight=", weight );

            //DEBUG_LINE( procId, ": succ=", succ );
            MPI_Recv( &succ, 1, MPI_INT, succ, SUCC_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            //DEBUG_LINE( procId, ": succ=", succ );
        }

        if ( prev != -1 ) {
            //DEBUG_LINE( procId, ": prev=", prev );
            MPI_Recv( &prev, 1, MPI_INT, prev, PREV_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            //DEBUG_LINE( procId, ": prev=", prev );
        }
    }
    //  usleep( procId * 10000 );
    if ( procId == 0 ) {
        int * weights = new int[ len ];
        int idx = 1;
        while( idx < procCount ) {
            MPI_Recv( &srcNode, 1, MPI_INT, idx, NODE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            MPI_Recv( weights + srcNode - 1, 1, MPI_INT, idx, WEIGHT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            idx++;
        }
        for ( int i = 0; i < trueLen;) {
            std::cout << argv[1][i] << ':' << weights[i];
            if ( ++i == trueLen ) {
                std::cout << std::endl;
            }
            else {
                std::cout << ',';
            }
        }
    }
    else {
        MPI_Send( &srcNode, 1, MPI_INT, 0, NODE_TAG, MPI_COMM_WORLD );
        MPI_Send( &weight, 1, MPI_INT, 0, WEIGHT_TAG, MPI_COMM_WORLD );
    }

    MPI::Finalize();
    return 0;
}