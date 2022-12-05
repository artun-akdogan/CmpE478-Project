#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <omp.h>

// Only for runtime measurement.
#include <chrono>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "parser.h"
#include "csrmatrix.h"
#include "csv.h"

using namespace std;
#define uint unsigned int

// Currently, main function is only for test and runtime measurement
int main(int argc, char** argv){
    ios::sync_with_stdio(false); // Comment if stdio has been used!!!
	omp_set_num_threads(8);
    omp_sched_t _type;
    if(argc<3) return -1;
    if(strcmp(argv[1], "static")==0){
        _type=omp_sched_static;
    } else if(strcmp(argv[1], "dynamic")==0){
        _type=omp_sched_dynamic;
    } else if(strcmp(argv[1], "guided")==0){
        _type=omp_sched_guided;
    } else if(strcmp(argv[1], "auto")==0){
        _type=omp_sched_auto;
    } else{
        return -1;
    }
    omp_set_schedule(_type, atoi(argv[2]));
    CSR_Matrix<double> *P;

    if(argc==5 && strcmp(argv[3], "load")==0){
        P = new CSR_Matrix<double>(string(argv[4]));
    }
    else{
        P = parse("graph.txt");
        P->write("backup.csv");
    }

    cout << "CSR Matric Initialized" << endl;

    double alpha = 0.2;
    double epsillon = 1e-6;
    vector<double> r_t, r_t1(P->get_size().second, 1);
    int i=0;
    
    auto st = chrono::high_resolution_clock::now();
    cout << "Matrix in size: " << P->get_size().first << " " << P->get_size().second <<endl;
    // Better to parallelize CSR Matrix operations instead of this loop.
    do{
        r_t = r_t1;
        r_t1 = P->ops(r_t, alpha, 1-alpha);
        i++;
        cout << check_sub_ops(r_t1, r_t) << " " << epsillon<< endl;
    } while(abs(check_sub_ops(r_t1, r_t)) > epsillon);
    cout << "Completed in "<< i << " iterations..."<<endl;
    auto end = chrono::high_resolution_clock::now();
    cout << "Time passed: " << chrono::duration_cast<chrono::milliseconds>(end-st).count() << endl;

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
        high.push_back(vector<string>({P->arr_dict[ind], to_string(maxi)}));
    }

    write_csv("result.csv", vector<string>({"Nodes", "Scores"}), high);
/*
    for(uint i=0; i<5 && (i<r_t.size()); i++){
        cout << r_t[i] <<endl;
    }*//*
    for(uint i=0; i<8; i++){
        cout << P->tim_arr[i] <<endl;
    }*/

    vector<vector<string>> logs = read_csv("log.csv");
    cerr << "Read csv" << endl;
    vector<string> last_row;
    last_row.push_back(to_string(logs.size()+1));
    last_row.push_back(argv[1]); //Schedule method
    last_row.push_back(argv[2]); //Chunk size
    last_row.push_back(to_string(P->no_of_iterations));
    vector<string> iter = vector_to_string(P->tim_arr);
    last_row.insert(last_row.end(), iter.begin(), iter.end());
    logs.push_back(last_row);
    vector<string>col_names({"Test No.", "Scheduling Method", "Chunk Size", "No of Iterations", "1", "2", "3", "4", "5", "6", "7", "8"});
    cerr << "Now write" << endl;
    write_csv("log.csv", col_names, logs);

    delete P;
    return 0;

    /*
    For below matrix, google algorithm has given in 26 iterations:
    5.4697
    3.57576
    6.98485
    2.89394
    5.26136
    */
   /*
    vector<vector<double>> P_vec
    {
        {1, 0, 1, 1, 0},
        {0, 0, 1, 1, 0},
        {1, 1, 1, 1, 0},
        {0, 1, 0, 1, 0},
        {1, 1, 0, 0, 1}
    };
    CSR_Matrix P(P_vec);
    double alpha = 0.2;
    double epsillon = 1e-6;
    vector<double> r_t, r_t1(P.get_size().second, 1);
    int i=0;
    do{
        r_t = r_t1;
        r_t1 = P.ops(r_t, alpha, 1-alpha);
        i++;
        cout << check_sub_ops(r_t1, r_t) << " " << epsillon<< endl;
    } while(check_sub_ops(r_t1, r_t) > epsillon);

    cout << i<<endl;
    for(uint i=0; i<r_t1.size(); i++){
        cout << r_t1[i] << " " << r_t[i] <<"\n";
    }

*/
/*
    // Test for csr write
    vector<vector<int>> vect
    {
        {11, 0, 13, 14, 0},
        {0, 0, 23, 24, 0},
        {31, 32, 33, 34, 0},
        {0, 42, 0, 44, 0},
        {51, 52, 0, 0, 55}
    };
    vector<int> vec(5, 2);
    CSR_Matrix<int> csr(vect);
    csr.set(1, 0, 22);
    csr.set(3, 4, 100);
    csr.set(1, 2, 200);
    csr.insert_row(vector<int>{2, 3, 4, 5, 6});
    vector<int> mulled = csr.mul(vec, 3);
    uint row, col;
    row = csr.get_size().first;
    col = csr.get_size().second;
    for(uint i=0; i<row; i++){
        for(uint l=0; l<col; l++){
            cout << csr.val(i, l) << " ";
        }
        cout << "\n";
    }
    for(uint i=0; i<mulled.size(); i++){
        
        cout << mulled[i] <<"\n";
    }

    csr.write("test.txt");
*/
    

    //Test for csvwrite
    /*
    vector<string> colname;
    vector<vector<string>>data(100000);
    auto st = chrono::high_resolution_clock::now();
    colname.reserve(100);
    for(int i=0; i<100; i++)
        colname.push_back(string("test")+to_string(i+1));
    for(int i=0; i<100000; i++){
        data[i].reserve(100);
        for(int l=0; l<100; l++)
            data[i].push_back(string("test")+to_string(i+1)+"+"+to_string(l+1));
    }
    auto mid = chrono::high_resolution_clock::now();
    write_csv("test.csv", colname, data);
    auto tot = chrono::high_resolution_clock::now();
    cout << "Time passed: " << chrono::duration_cast<chrono::milliseconds>(mid-st).count()
            << " " <<  chrono::duration_cast<chrono::milliseconds>(tot-mid).count() << endl;
    */
    return 0;
}