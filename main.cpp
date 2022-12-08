#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <limits>
#include <fstream>
#include <iostream>
#include <omp.h>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "parser.h"
#include "csrmatrix.h"
#include "csv.h"

using namespace std;
#define uint unsigned int

int iterations=0;
double last_tim=0;

void run_program(CSR_Matrix<double> *P, int thread_num, int block_size, omp_sched_t _type){
	omp_set_num_threads(thread_num);
    omp_set_schedule(_type, block_size);

    double alpha = 0.2;
    double epsillon = 1e-6;
    double tim_st, tim_end;
    vector<double> r_t, r_t1(P->get_size().second, 1);
    iterations=0;
    
    cout << "Matrix in size: " << P->get_size().first << " " << P->get_size().second <<endl;
    tim_st = omp_get_wtime( );
    // Better to parallelize CSR Matrix operations instead of this loop.
    do{
        r_t = r_t1;
        r_t1 = P->ops(r_t, alpha, 1-alpha);
        iterations++;
        cout << "Current Diff: "<<P->two_vec_diff<< endl;
    } while(P->two_vec_diff > epsillon);
    tim_end = omp_get_wtime();
    last_tim=tim_end-tim_st;
    cout << "Completed in "<< iterations << " iterations..."<<endl;
    cout << "Time passed: " << last_tim << endl;

/*
    st = chrono::high_resolution_clock::now();

    sort(r_t.begin(), r_t.end(), greater<double>());
    cout << "Sort completed..."<<endl;
    end = chrono::high_resolution_clock::now();
    cout << "Time passed: " << chrono::duration_cast<chrono::milliseconds>(end-st).count() << endl;
*/
    vector<vector<string>>high;
    double maxi, last = numeric_limits<double>::max();
    // Find max 5 with simple n*5 iteration
    for(int i=0; i<5; i++){
        int ind=0;
        maxi = numeric_limits<double>::min();
        for (int l=0; l<r_t.size(); l++){
            if(maxi<r_t[l] && r_t[l]<last){
                maxi = r_t[l];
                ind = l;
            }
        }
        cerr << ind << " "<< last << " " << maxi << endl;
        last = maxi;
        high.push_back(vector<string>({to_string(high.size()+1), P->arr_dict[ind], to_string(maxi)}));
    }

    write_csv("result.csv", vector<string>({"No.", "Nodes", "Scores"}), high);
/*
    for(uint i=0; i<5 && (i<r_t.size()); i++){
        cout << r_t[i] <<endl;
    }*//*
    for(uint i=0; i<8; i++){
        cout << P->tim_arr[i] <<endl;
    }*/

    
}

void schedule_program(CSR_Matrix<double> *P, vector<vector<string>> *logs, omp_sched_t _type, string schedule){
    static int csv_iter=1;
    for(int block_size=100; block_size<=100000; block_size*=10){
        vector<string> last_row;
        last_row.push_back(to_string(csv_iter));
        last_row.push_back(schedule); //Schedule method
        last_row.push_back(to_string(block_size)); //Chunk size
        last_row.push_back("");
        for(int i=1; i<=8; i++){
            cout << "Running program with "<< schedule << " : " << block_size << " : " << i << endl;
            run_program(P, i, block_size, _type);
            last_row.push_back(to_string(last_tim));
        }
        last_row[3]=to_string(iterations);
        logs->push_back(last_row);
        csv_iter++;
    }
}

// Currently, main function is only for test and runtime measurement
int main(int argc, char** argv){
    ios::sync_with_stdio(false); // Comment if stdio has been used!!!

    CSR_Matrix<double> *P;
    
    if(argc>=3 && strcmp(argv[1], "load")==0){
        P = new CSR_Matrix<double>(string(argv[2]));
    }
    else{
        P = parse("graph.txt");
        if(argc>=3 && strcmp(argv[1], "save")==0){
            P->write(argv[2]);
        }
    }

    cout << "CSR Matric Initialized" << endl;

    vector<vector<string>> logs;

    schedule_program(P, &logs, omp_sched_static, "static");
    schedule_program(P, &logs, omp_sched_dynamic, "dynamic");
    schedule_program(P, &logs, omp_sched_guided, "guided");
    schedule_program(P, &logs, omp_sched_auto, "auto");

    vector<string>col_names({"Test No.", "Scheduling Method", "Chunk Size", "No of Iterations", "1", "2", "3", "4", "5", "6", "7", "8"});
    cerr << "Now write" << endl;
    write_csv("log.csv", col_names, logs);

    delete P;
    return 0;
}