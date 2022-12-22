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

pair<double, int> run_program(CSR_Matrix<double> *P, int thread_num, int block_size, omp_sched_t _type){
    // Set initial values
    int iterations=0;
    double alpha = 0.2;
    double epsillon = 1e-6;
    double tim_st, tim_end;
    double last_tim;
    vector<double> r_t, r_t1(P->get_size().second, 1);

    // Set runtime scheduling method
	omp_set_num_threads(thread_num);
    omp_set_schedule(_type, block_size);
    
    cout << "Matrix in size: " << P->get_size().first << " " << P->get_size().second <<endl;
    tim_st = omp_get_wtime( );
    
    // Begin operation. Keep going until vector diff is below epsilon
    // P->ops function is parallelised, this loop only performs minor operations.
    do{
        r_t = r_t1;
        r_t1 = P->ops(r_t, alpha, 1-alpha);
        iterations++;
        cout << "Current Diff: "<<P->two_vec_diff<< endl;
    } while(P->two_vec_diff > epsillon);

    // Print passed time. Also, this value will be used on schedule_program function.
    tim_end = omp_get_wtime();
    last_tim = tim_end - tim_st;
    cout << "Completed in "<< iterations << " iterations..."<<endl;
    cout << "Time passed: " << last_tim << endl;

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
    // Return values for logging.
    return {last_tim, iterations};
}

// Another function to schedule testcases. Also prepares csv log file.
void schedule_program(CSR_Matrix<double> *P, vector<vector<string>> *logs, omp_sched_t _type, string schedule){
    static int csv_iter=1;
    // Block size iteration
    for(int block_size=1; block_size<=1000000; block_size*=100){
        vector<string> last_row;
        last_row.push_back(to_string(csv_iter));
        last_row.push_back(schedule); //Schedule method
        last_row.push_back(to_string(block_size)); //Chunk size
        last_row.push_back(""); // Total iterations (index: 3)
        pair<double, int>retval;
        // Thread number iteration
        for(int i=1; i<=8; i++){
            cout << "Running program with "<< schedule << " : " << block_size << " : " << i << endl;
            retval = run_program(P, i, block_size, _type);
            // First value returned is passed time
            last_row.push_back(to_string(retval.first));
        }
        // Second value returned is iteration number
        last_row[3]=to_string(retval.second);
        logs->push_back(last_row);
        csv_iter++;
    }
}

// Autonomously run different testcases, and parse CSR Matrix
int main(int argc, char** argv){
    ios::sync_with_stdio(false); // Comment if stdio has been used!!!
    CSR_Matrix<double> *P;
    double tim_st, tim_end;

    tim_st = omp_get_wtime( );
    
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

    vector<vector<string>> logs;

    // Run for 4 different schedules
    schedule_program(P, &logs, omp_sched_static, "static");
    schedule_program(P, &logs, omp_sched_dynamic, "dynamic");
    schedule_program(P, &logs, omp_sched_guided, "guided");
    schedule_program(P, &logs, omp_sched_auto, "auto");

    // Write log to CSV file.
    vector<string>col_names({"Test No.", "Scheduling Method", "Chunk Size", "No of Iterations", "1", "2", "3", "4", "5", "6", "7", "8"});
    cout << "Now write" << endl;
    write_csv("log.csv", col_names, logs);

    // Print runtime and exit.
    tim_end = omp_get_wtime();
    cout << "Program finished in: " << tim_end-tim_st << endl;

    delete P;
    return 0;
}