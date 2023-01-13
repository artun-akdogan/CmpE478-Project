#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <limits>
#include <fstream>
#include <iostream>
#include <time.h>  //For clock_gettime

// Thrust
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/sort.h>
#include <thrust/copy.h>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "parser.h"
#include "csrmatrix.h"
#include "csv.h"

using namespace std;
#define uint unsigned int

void run_program(CSR_Matrix<double> *P){
    // Set initial values
    int iterations=0;
    double alpha = 0.2;
    double epsillon = 1e-6;
    vector<double> r_t, r_t1(P->get_size().second, 1);
    thrust::device_vector<double>d_x, d_x1(r_t1.begin(), r_t1.end())

    // Time measure
    struct timespec mt1, mt2;
    long int tt;
    
    cout << "Matrix in size: " << P->get_size().first << " " << P->get_size().second <<endl;
    clock_gettime (CLOCK_REALTIME, &mt1);
    
    // Begin operation. Keep going until vector diff is below epsilon
    // P->ops function is parallelised, this loop only performs minor operations.
    do{
        d_x = d_x1;
        P->ops(d_x1, d_x, alpha, 1-alpha);
        iterations++;
        cout << "Current Diff: "<<P->two_vec_diff<< endl;
    } while(P->two_vec_diff > epsillon);
    thrust::copy(d_x.begin(), d_x.end(), r_t.begin());

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

// Autonomously run different testcases, and parse CSR Matrix
int main(int argc, char** argv){
    ios::sync_with_stdio(false); // Comment if stdio has been used!!!
    CSR_Matrix<double> *P;

    // Time measure
    struct timespec mt1, mt2;
    long int tt;
    
    clock_gettime (CLOCK_REALTIME, &mt1);
    
    // Initialize CSR matrix. Either parse from file,
    // or load from dumped csv file if requested. (load filename)
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

    cout << "CSR Matrix Initialized" << endl;

    // Run program
    run_program(P);

    // Print runtime and exit.
    clock_gettime (CLOCK_REALTIME, &mt2);
    tt=1000000000*(mt2.tv_sec - mt1.tv_sec)+(mt2.tv_nsec - mt1.tv_nsec);
    cout << "Program finished in: " << tt/1000000 << "msecs" << endl;

    delete P;
    return 0;
}