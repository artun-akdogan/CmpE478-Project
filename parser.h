#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_set>
// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "csrmatrix.h"

using namespace std;
typedef unsigned int uint;

class Parser{
private:
    map<string, vector<string>> node;
    map<string, int> name_dict;
    vector<string> arr_dict;
    CSR_Matrix<double> *csr;

public:
    Parser(const string &filename){
        unordered_set<string> unique_arr;
        ifstream in(filename);
        
        int i=0;

        cout << "Reading file..." << endl;
        while (!in.eof()){
            if(i%1000000==0){
                cout << i << endl;
            }
            string t1, t2;
            in >> t1 >> t2;
            node[t1].push_back(t2);
            unique_arr.insert(t1);
            unique_arr.insert(t2);
            i++;
        }
        in.close();

        uint col_size = unique_arr.size();

        cout << "Total unique sites: " << col_size << endl;

        csr = new CSR_Matrix<double>(0U, col_size);

        cout << "Creating dictionaries..." << endl;
        unordered_set<string>::iterator it;
        arr_dict.reserve(col_size);
        for (it = unique_arr.begin(), i=0; it != unique_arr.end(); ++it, ++i) {
            name_dict[*it] = i;
            arr_dict.push_back(*it);
        }
        unique_arr.clear();

        cout << "Creating CSR Matrix..." << endl;
        for(i=0; i<col_size; i++){
            if(i%1000==0){
                cout << i << " " << node[arr_dict[i]].size() << endl;
            }
            vector<double> row(col_size, 0);
            for(auto x: node[arr_dict[i]]){
                row[name_dict[x]]=1;
            }
            csr->insert_row(row);
        }

    }
};

#endif