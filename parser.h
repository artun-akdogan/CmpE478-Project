#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_set>
#include <omp.h>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "csrmatrix.h"

using namespace std;
typedef unsigned int uint;

CSR_Matrix<double> *parse(const string &filename){
    // CSR matrix pointer
    CSR_Matrix<double> *csr;
    // Right to left unidirectional graph
    vector<vector<int>> link_to;
    // Left to right unidirectional graph
    vector<vector<int>> link_by;
    // Name to index dictionary
    unordered_map<string, int> name_dict;
    // Index to name dictionary
    vector<string> arr_dict;
    // Timing variables
    double tim_st, tim_end;

    unordered_set<string> unique_arr;
    vector<pair<string, string>>temp;
    ifstream in(filename);

    int i=0, p1, p2;

    tim_st = omp_get_wtime( );
    cout << "Reading file..." << endl;

    // Approximate size initialization (dropped time from 53 sec to 47 sec)
    temp.reserve(17000000);
    unique_arr.reserve(17000000);

    // Time passed: 53sec, but not parallelizable (Attempts made it worse)
    // Read file, and insert them into unordered_set to keep track of unique elements
    string t1, t2;
    while (!in.eof()){
        if(i%1000000==0){
            cout << i << endl;
        }
        in >> t1 >> t2;
        temp.push_back({t2, t1});
        unique_arr.insert(t1);
        unique_arr.insert(t2);
        i++;
    }
    in.close();
    
    tim_end = omp_get_wtime( );
    cout << "Time passed: " << tim_end-tim_st << endl;

    cout << "Total unique sites: " << unique_arr.size() << endl;


    tim_st = omp_get_wtime( );
    cout << "Creating numeration..." << endl;

    // Time passed: 3sec, but hard to parallelize (No need to parallelize either)
    // Enumerate each unique element
    unordered_set<string>::const_iterator uit = unique_arr.begin();
    for (i=0; uit != unique_arr.end(); ++uit, ++i) {
        name_dict[*uit] = i;
        arr_dict.push_back(*uit);
    }
    // Delete unused vectors
    unique_arr.clear();
    // Resize vectors for parallelisation and performance
    link_to.resize(arr_dict.size());
    link_by.resize(arr_dict.size());
    tim_end = omp_get_wtime( );
    cout << "Time passed: " << tim_end-tim_st << endl;

    tim_st = omp_get_wtime( );
    cout << "Creating nodes..." << endl;

    // Parallelization dropped time from 20 sec to 8 sec
    // Unrecorded in csv file as not requested
    // Node creation loop (from right to left)
    #pragma omp parallel for private(i, p1, p2) shared(link_to, link_by, name_dict) schedule(dynamic, 50000)
    for (i=0; i<temp.size(); i++) {
        p1 = name_dict.find(temp[i].first)->second;
        p2 = name_dict.find(temp[i].second)->second;

        // Don't push elements at the same time!
        #pragma omp critical
        {
            link_to[p1].push_back(p2);
            link_by[p2].push_back(p1);
        }
    }
    // Delete unused vectors
    temp.clear();

    tim_end = omp_get_wtime( );
    cout << "Time passed: " << tim_end-tim_st << endl;

    tim_st = omp_get_wtime( );
    cout << "Creating CSR Matrix..." << endl;
    // Create CSR matrix
    csr = new CSR_Matrix<double>(link_to, link_by, name_dict, arr_dict);
    tim_end = omp_get_wtime( );
    cout << "Time passed: " << tim_end-tim_st << endl;
    return csr;
};

#endif