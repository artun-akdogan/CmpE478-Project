#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <limits>
#include <fstream>
#include <iostream>
#include <time.h>  //For clock_gettime
#include <mpi.h>

#define row_num 1850065

uint par_rows[4] = {0, 616688, 1233376, 1850065};
//uint par_rows[2] = {0, 1850065};

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "parser.h"
#include "csrmatrix.h"
#include "csv.h"

using namespace std;
#define uint unsigned int

int mypid, numprocs;

void run_program(CSR_Matrix<double> *P){
    // Set initial values
    int iterations=0;
    double alpha = 0.2;
    double epsillon = 1e-6;
    double difference = 0; // Keep it for MPI difference calculation
    vector<double> r_t(P->get_size().second, 1), r_t1, r_recv[numprocs-1];

    // Time measure
    struct timespec mt1, mt2;
    long int tt;
    
    cout << "Matrix in size: " << P->get_size().first << " " << P->get_size().second <<endl;
    clock_gettime (CLOCK_REALTIME, &mt1);
    
    if(mypid==0){
        // Begin operation. Keep going until vector diff is below epsilon
        // P->ops function is parallelised, this loop only performs minor operations.
        do{
            // Send vector
            MPI_Barrier(MPI_COMM_WORLD);
            for(uint i=1; i<numprocs; i++){
                MPI_Send(&r_t[0], P->get_size().second, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
            }
            // Wait and receive threads from other threads
            MPI_Barrier(MPI_COMM_WORLD);
            for(uint i=1; i<numprocs; i++){
                r_recv[i-1].resize(par_rows[i]-par_rows[i-1]);
                MPI_Recv(&r_recv[i-1][0], par_rows[i]-par_rows[i-1], MPI_DOUBLE, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // Reunite vector from other threads
            r_t.clear();
            for(uint i=1; i<numprocs; i++){
                r_t.insert(r_t.end(), r_recv[i-1].begin(), r_recv[i-1].end());
            }
            
            MPI_Barrier(MPI_COMM_WORLD);
            // Two vec diff is already 0 for main as it never run ops function
            MPI_Allreduce(&P->two_vec_diff, &difference, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            iterations++;
            cout << "Current Diff: "<<difference<< endl;
        } while(difference > epsillon);
    }else{
        do{
            // Receive vector
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Recv(&r_t[0], r_t.size(), MPI_DOUBLE, 0, mypid, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Calculate partial vector
            r_t1 = P->ops(r_t, alpha, 1-alpha, par_rows[mypid-1]);
            // Send it back to main thread for it to unite
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Send(&r_t1[0], par_rows[mypid]-par_rows[mypid-1], MPI_DOUBLE, 0, mypid, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
            // Calculate partial differences via reduce operation
            MPI_Allreduce(&P->two_vec_diff, &difference, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        } while(difference > epsillon);
    }

    if(mypid==0){

        // Print passed time. Also, this value will be used on schedule_program function.
        clock_gettime (CLOCK_REALTIME, &mt2);
        tt=1000000000*(mt2.tv_sec - mt1.tv_sec)+(mt2.tv_nsec - mt1.tv_nsec);
        cout << "Completed in "<< iterations << " iterations..."<<endl;
        cout << "Time passed: " << tt/1000000 << "msecs" << endl;

        vector<vector<string>>high;
        double maxi, last = numeric_limits<double>::max();

        cout << "First 5 elements:\n";
        // Find max 5 elements with simple n*5 iteration
        for(int i=0; i<5; i++){
            int ind=0;
            maxi = numeric_limits<double>::min();
            for (int l=0; l<r_t.size(); l++){
                if(maxi<r_t[l] && r_t[l]<last){
                    maxi = r_t[l];
                    ind = l;
                }
            }
            cout << i+1 << ": " <<P->arr_dict[ind] << " : "<< maxi << endl;
            last = maxi;
            // Push elements in order.
            high.push_back(vector<string>({to_string(high.size()+1), P->arr_dict[ind], to_string(maxi)}));
        }
        // Write result.csv
        write_csv("result.csv", vector<string>({"No.", "Nodes", "Scores"}), high);
    }
}

// Autonomously run different testcases, and parse CSR Matrix
int main(int argc, char** argv){
    ios::sync_with_stdio(false); // Comment if stdio has been used!!!
    CSR_Matrix<double> *P;

    // Time measure
    struct timespec mt1, mt2;
    long int tt;
    
    // Initialize mpi
    MPI_Init (&argc, &argv);                        /* starts MPI */

    MPI_Comm_rank (MPI_COMM_WORLD, &mypid);        /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);     /* get number of processes */

    clock_gettime (CLOCK_REALTIME, &mt1);
    
    // Initialize CSR matrix. Either parse from file,
    // or load from dumped csv file if requested. (load filename)
    if(mypid==0){
        if(argc>=3 && strcmp(argv[1], "load")==0){
            P = new CSR_Matrix<double>(string(argv[2]));
        }
        else{
            P = parse("graph.txt");
            // If requested, dump file to csv file (save filename)
            if(argc>=3 && strcmp(argv[1], "save")==0){
                P->write(argv[2]);
            }
        }
        cout << "Starting matrix partitioning without METIS..." << endl;
        MPI_Barrier(MPI_COMM_WORLD);
        uint last=0;
        vector<uint>mod_row(P->row_begin.size());
        for(uint i=1; i<numprocs; i++){
            uint l=par_rows[i-1], beg;
            for(; l<par_rows[i]; l++){
                if(l==0 && last==0){
                    mod_row[l-par_rows[i-1]] = -1;
                } else if(P->row_begin[l-1]==P->row_begin[l]){
                    mod_row[l-par_rows[i-1]]=-1;
                } else{
                    break;
                }
            }
            beg = P->row_begin[l];
            for(; l<par_rows[i]; l++){
                mod_row[l-par_rows[i-1]] = P->row_begin[l]-beg;
            }
            mod_row[l-par_rows[i-1]] = P->row_begin[l]-beg;
            MPI_Send(&P->row_begin[par_rows[i-1]], par_rows[i]-par_rows[i-1]+1, MPI_UNSIGNED, i, i, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        
        for(uint i=1; i<numprocs; i++){
            uint siz = (P->row_begin[par_rows[i]]==-1 ? 0 : P->row_begin[par_rows[i]])-(P->row_begin[par_rows[i-1]]==-1 ? 0 : P->row_begin[par_rows[i-1]]);
            MPI_Send(&P->col_indices[P->row_begin[par_rows[i-1]]==-1 ? 0 : P->row_begin[par_rows[i-1]]], siz, MPI_UNSIGNED, i, i, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        for(uint i=1; i<numprocs; i++){
            uint siz = (P->row_begin[par_rows[i]]==-1 ? 0 : P->row_begin[par_rows[i]])-(P->row_begin[par_rows[i-1]]==-1 ? 0 : P->row_begin[par_rows[i-1]]);
            MPI_Send(&P->values[P->row_begin[par_rows[i-1]]==-1 ? 0 : P->row_begin[par_rows[i-1]]], siz, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
        }
        cout << "Sent all values to threads" << endl;
    }else{
        P = new CSR_Matrix<double>(par_rows[mypid]-par_rows[mypid-1], 1850065);
        cout << "Matrix row size for thread #" << mypid <<": " << par_rows[mypid]-par_rows[mypid-1] << endl;
        // Get row_begin
        P->row_begin.resize(P->row+1);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Recv(&P->row_begin[0], P->row+1, MPI_UNSIGNED, 0, mypid, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        uint siz = (P->row_begin.back()==-1 ? 0 : P->row_begin.back());
        cout << "Actual size for thread #" << mypid <<": " <<siz <<  endl;

        // Get col_indices
        P->col_indices.resize(siz);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Recv(&P->col_indices[0], P->col_indices.size(), MPI_UNSIGNED, 0, mypid, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Get values
        P->values.resize(siz);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Recv(&P->values[0], P->values.size(), MPI_DOUBLE, 0, mypid, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "CSR Matrix Initialized for thread #" << mypid<< endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Run program
    run_program(P);


    if(mypid==0){
        // Print runtime and exit.
        clock_gettime (CLOCK_REALTIME, &mt2);
        tt=1000000000*(mt2.tv_sec - mt1.tv_sec)+(mt2.tv_nsec - mt1.tv_nsec);
        cout << "Program finished in: " << tt/1000000 << "msecs" << endl;
    }

    delete P;
    MPI_Finalize();
    return 0;
}