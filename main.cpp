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

using namespace std;

// Convert a vector to vector of strings
template<typename T>
inline vector<string> vector_to_string(const vector<T> &vec){
    vector<string> ret;
    transform(vec.begin(), vec.end(), back_inserter(ret),
                   [&](T d) { return to_string(d); } );
    return ret;
}

const string join(const vector<string> &lst, const string &delim){
    // Basic string vector join, similar to Python's
    string ret;
    for(int i=0; i<lst.size()-1; i++) {
        ret += lst[i];
        ret += delim;
    }
    ret += lst.back();
    return ret;
}

// CSV file writer. Seperator is ',', newline is '\n'.
// If colname vector is empty, no colname will be written.
void write_csv(const string &filename, const vector<string> &colname, const vector<vector<string>> &vals){
    // Buffer data first.
    string buffer;
    if(!colname.empty()){
        buffer += join(colname, ",");
        buffer += "\n";
        assert(colname.size()==vals[0].size());
    }
    for(int i=0; i<vals.size(); i++){
        assert(vals[0].size()==vals[i].size()); // Every row should be equal in size with colnames
        buffer += join(vals[i], ",");
        buffer += "\n";
    }
    // Write buffer to file
    ofstream csv(filename);
    csv << buffer;
    csv.close();
}

template<typename T>
class CSR_Matrix{
    private:
    vector<int> row_begin;
    vector<T> values;
    vector<int> col_indices;

    public:
    // Write matrix to file
    void write(const string &filename){
        string buffer;
        buffer += join(vector_to_string(row_begin), ",");
        buffer += "\n";
        buffer += join(vector_to_string(values), ",");
        buffer += "\n";
        buffer += join(vector_to_string(col_indices), ",");
        buffer += "\n";
        // Write buffer to file
        ofstream csv(filename);
        csv << buffer;
        csv.close();
    }

    // Initialize a matrix from vector
    CSR_Matrix(vector<vector<T>> matrix){
        row_begin.resize(matrix.size()+1, -1);
        for(int i=0; i<matrix.size(); i++){
            assert(matrix[0].size()==matrix[i].size()); // Assert fixed matrix in column number
            for(int l=0; l<matrix[0].size(); l++){
                if(matrix[i][l]!=0){
                    if(row_begin[i]==-1){
                        row_begin[i] = values.size();
                    }
                    values.push_back(matrix[i][l]);
                    col_indices.push_back(l);
                }
            }
        }
        row_begin.back() = values.size();
    }
};

// Currently, main function is only for test and runtime measurement
int main(){
    ios::sync_with_stdio(false); // Comment if stdio has been used!!!

    // Test for csr write
    /*
    vector<vector<int>> vect
    {
        {11, 0, 13, 14, 0},
        {0, 0, 23, 24, 0},
        {31, 32, 33, 34, 0},
        {0, 42, 0, 44, 0},
        {51, 52, 0, 0, 55}
    };
    CSR_Matrix<int> csr(vect);
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