#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>

// Only for runtime measurement.
#include <chrono>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "csrmatrix.h"
#include "csv.h"

using namespace std;
typedef unsigned int uint;

// Currently, main function is only for test and runtime measurement
int main(){
    ios::sync_with_stdio(false); // Comment if stdio has been used!!!
    vector<vector<double>> P_vec
    {
        {1, 0, 1, 1, 0},
        {0, 0, 1, 1, 0},
        {1, 1, 1, 1, 0},
        {0, 1, 0, 1, 0},
        {1, 1, 0, 0, 1}
    };
    /*
    For below matrix, google algorithm has given in 26 iterations:
    5.4697
    3.57576
    6.98485
    2.89394
    5.26136
    */
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