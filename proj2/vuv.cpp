/**
 * Author: Jiri Matejka
 * Login: xmatej52
 * Modified: 13. 04. 2019
 */
#include "vuv.h"

int main( int argc, const char ** argv ) {
    int procCount, procId;
    MPI::Init();

    // number of processes
    MPI_Comm_size( MPI_COMM_WORLD, &procCount );

    // id of current process
    procId = MPI::COMM_WORLD.Get_rank();

    // len of balanced binary tree
    int len = 0;

    // len of binary tree from program arguments
    int trueLen = 0;

    if ( procId == 0 ) {
        // checks arguments
        int argCheck = checkArguments( argc, argv, procCount );
        if ( argCheck == INVALID_ARGUMENTS) {
            std::cerr << "Invalid program arguments, please specify each node once." << std::endl;
            MPI_Abort( MPI_COMM_WORLD, INVALID_ARGUMENTS );
        }
        else if ( argCheck == BAD_PROC_COUNT ) {
            std::cerr << "Invalid nuber of processes." << std::endl;
            MPI_Abort( MPI_COMM_WORLD, BAD_PROC_COUNT );
        }

        // tests if there are other processes
        if ( procCount == 1 ) {
            std::cout << argv[1][0] << ":0" << std::endl;
            MPI::Finalize();
            return 0;
        }

        // inits lengths of trees
        trueLen = strlen( argv[1] );
        len     = nextPower( trueLen + 1 ) - 1;
    }

    // root sends size of tree to all processes
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // inits weight with default value
    int weight     = getWeight( procId, len );

    // gets destination node
    int destNode   = getDestNode( procId, len );

    // gts source node
    int srcNode    = getSrcNode( procId, len );

    // gets successor process
    int succ       = procId == len ? -1 : getSucc( procId, destNode, len );

    // gets predecessor process
    int prev       = procId ? getPrev( procId, srcNode, len ) : -1;

    // here will be stored weights of successor processes in each cycle
    int prevWeight = 0;

   // cycle of prefixSum
    while ( succ >= 0 || prev >= 0 ) {
        // sends weight and successor to predecessor
        if ( prev != -1 ) {
            MPI_Send( &weight, 1, MPI_INT, prev, WEIGHT_TAG, MPI_COMM_WORLD );
            MPI_Send( &succ, 1, MPI_INT, prev, SUCC_TAG, MPI_COMM_WORLD );
        }

        if ( succ != -1 ) {
            // sends predecessor to successor
            MPI_Send( &prev, 1, MPI_INT, succ, PREV_TAG, MPI_COMM_WORLD );

            // sets weight of successor
            MPI_Recv( &prevWeight, 1, MPI_INT, succ, WEIGHT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            // modifies current weight
            weight += prevWeight;

            // sets new successor
            MPI_Recv( &succ, 1, MPI_INT, succ, SUCC_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        }

        // sets new predecessor
        if ( prev != -1 ) {
            MPI_Recv( &prev, 1, MPI_INT, prev, PREV_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        }
    }


    if ( procId == 0 ) {
        int * weights = new int[ len ];
        int idx = 1;

        // read results from other processes
        while( idx < procCount ) {
            MPI_Recv( &srcNode, 1, MPI_INT, idx, NODE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            MPI_Recv( weights + srcNode - 1, 1, MPI_INT, idx, WEIGHT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            idx++;
        }

        // prints result to stdout
        for ( int i = 0; i < trueLen;) {
            std::cout << argv[1][i] << ':' << weights[i];
            if ( ++i == trueLen ) {
                std::cout << std::endl;
            }
            else {
                std::cout << ',';
            }
        }

        delete weights;
    }
    else {
        // sends result to root process
        MPI_Send( &srcNode, 1, MPI_INT, 0, NODE_TAG, MPI_COMM_WORLD );
        MPI_Send( &weight, 1, MPI_INT, 0, WEIGHT_TAG, MPI_COMM_WORLD );
    }

    MPI::Finalize();
    return 0;
}

int nextPower( int len ) {
    int power = 1;

    if ( len < 2 ) {
        return 1;
    }

    // while power < len, we multiply power by 2
    while ( power < len ) {
        power <<= 1;
    }

    return power;
}

int checkArguments( int argc, const char ** argv, unsigned procCount ) {
    // checks number of arguments
    if ( argc != 2 ) {
        return false;
    }
    else {
        // expected number of nodes
        int nodes  = ( procCount >> 1 ) + 1;
        // len of tree
        int argLen = strlen( argv[1] );

        /*
        // map of already set argumetns
        argSet specified;

        // test if input sequence is unique
        for( int i = 0; i < argLen; i++ ) {
            if ( specified.find( argv[1][i] ) == specified.end() ) {
                specified[ argv[1][i] ] = true;
            }
            else {
                return INVALID_ARGUMENTS;
            }
        }
        */

        // test if number of processes is correct
        return nodes == nextPower( argLen + 1 ) - 1 ? 0 : BAD_PROC_COUNT;
    }
}