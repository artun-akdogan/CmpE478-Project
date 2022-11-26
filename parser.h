#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_set>

// Only for runtime measurement.
#include <chrono>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "csrmatrix.h"

using namespace std;
typedef unsigned int uint;

class Parser{
private:
    map<string, vector<string>> link_to;
    map<string, vector<string>> link_by;
    map<string, int> name_dict;
    vector<string> arr_dict;

public:
    CSR_Matrix<double> *csr;

    Parser(const string &filename){
        unordered_set<string> unique_arr;
        ifstream in(filename);
        
        int i=0;

        auto st = chrono::high_resolution_clock::now();
        cout << "Reading file..." << endl;
        // Time passed: 115973, but not parallelizable
        while (!in.eof()){
            if(i%1000000==0){
                cout << i << endl;
            }
            string t1, t2;
            in >> t1 >> t2;
            link_to[t1].push_back(t2);
            link_by[t2].push_back(t1);
            unique_arr.insert(t1);
            unique_arr.insert(t2);
            i++;
        }
        in.close();
        
        auto end = chrono::high_resolution_clock::now();
        cout << "Time passed: " << chrono::duration_cast<chrono::milliseconds>(end-st).count() << endl;

        cout << "Total unique sites: " << unique_arr.size() << endl;

        st = chrono::high_resolution_clock::now();
        cout << "Creating dictionaries..." << endl;
        unordered_set<string>::iterator it;
        arr_dict.reserve(unique_arr.size());
        // Time passed: 6040, but not parallelizable (No need to parallelize either)
        for (it = unique_arr.begin(), i=0; it != unique_arr.end(); ++it, ++i) {
            name_dict[*it] = i;
            arr_dict.push_back(*it);
        }
        unique_arr.clear();
        end = chrono::high_resolution_clock::now();
        cout << "Time passed: " << chrono::duration_cast<chrono::milliseconds>(end-st).count() << endl;

        st = chrono::high_resolution_clock::now();
        cout << "Creating CSR Matrix..." << endl;
        csr = new CSR_Matrix<double>(link_to, link_by, name_dict, arr_dict);
        end = chrono::high_resolution_clock::now();
        cout << "Time passed: " << chrono::duration_cast<chrono::milliseconds>(end-st).count() << endl;
    }
    ~Parser(){
        delete csr;
    }
};

#endif