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
    vector<unordered_set<int>> link_to;
    vector<vector<int>> link_by;
    unordered_map<string, int> name_dict;
    vector<string> arr_dict;

public:
    CSR_Matrix<double> *csr;

    Parser(const string &filename){
        unordered_set<string> unique_arr;
        vector<pair<string, string>>temp;
        ifstream in(filename);
        
        int i=0;

        auto st = chrono::high_resolution_clock::now();
        cout << "Reading file..." << endl;
        // Time passed: 50sec, but not parallelizable
        string t1, t2;
        while (!in.eof()){
            if(i%1000000==0){
                cout << i << endl;
            }
            in >> t1 >> t2;
            temp.push_back({t1, t2});
            //link_to[t1].insert(t2);
            //link_by[t2].push_back(t1);
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
        unordered_set<string>::const_iterator uit;
        // Time passed: 6040, but not parallelizable (No need to parallelize either)
        for (uit = unique_arr.begin(), i=0; uit != unique_arr.end(); ++uit, ++i) {
            name_dict[*uit] = i;
            arr_dict.push_back(*uit);
        }
        unique_arr.clear();
        vector<pair<string, string>>::const_iterator it;
        link_to.resize(arr_dict.size());
        link_by.resize(arr_dict.size());
        // Time passed: 40sec
        for (it = temp.begin(), i=0; it != temp.end(); ++it, ++i) {
            if(i%1000000==0){
                cout << i << endl;
            }
            int p1 = name_dict[it->first], p2=name_dict[it->second];
            link_to[p1].insert(p2);
            link_by[p2].push_back(p1);
        }
        temp.clear();
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