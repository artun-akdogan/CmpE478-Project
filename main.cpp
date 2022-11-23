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
typedef unsigned int uint;

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
    uint row, col;
    // Value indices. Must be ordered
    vector<uint> row_begin;
    vector<uint> col_indices;
    //Non zero values
    vector<T> values;

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

    // Initialize an empty matrix
    CSR_Matrix(uint row, uint col){
        this.row = row;
        this.col = col;

    }

    // Initialize a matrix from vector
    CSR_Matrix(vector<vector<T>> matrix){
        this->row = matrix.size();
        this->col = matrix[0].size();
        row_begin.resize(matrix.size()+1, -1);
        for(uint i=0; i<matrix.size(); i++){
            assert(matrix[0].size()==matrix[i].size()); // Assert fixed matrix in column number
            for(uint l=0; l<matrix[0].size(); l++){
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

    // Non optimal value set. Try not to use
    void set(uint row, uint col, T val){
        auto st = next(col_indices.begin(), row_begin[row]);
        auto end = next(col_indices.begin(), row_begin[row+1]);
        int loc = distance(col_indices.begin(), lower_bound(st, end, col));
        if(col == col_indices[loc] && loc < row_begin[row+1]){
            values[loc] = val;
            return;
        }
        // Zero value sets are in linear time O(col*row)
        values.insert(values.begin()+loc, val);
        col_indices.insert(col_indices.begin()+loc, col);
        for(uint i=row+1; i<=this->row; i++){
            row_begin[i]++;
        }
    }

    // Return value in worst case O(log(col))
    T val(uint row, uint col){
        auto st = next(col_indices.begin(), row_begin[row]);
        auto end = next(col_indices.begin(), row_begin[row+1]);
        int loc = distance(col_indices.begin(), lower_bound(st, end, col));
        if(col == col_indices[loc] && loc < row_begin[row+1]){
            return values[loc];
        }
        return 0;
    }

    vector<T> mul(const vector<T> &vec, T sca){
        assert(vec.size()==this->col);
        vector<T> ret(this->row);
        for(uint i=0; i<this->row; i++){
            for(uint l=row_begin[i]; l<row_begin[i+1]; l++){
                ret[i] += values[l] * vec[col_indices[l]] * sca;
            }
        }
        return ret;
    }
};

// Currently, main function is only for test and runtime measurement
int main(){
    ios::sync_with_stdio(false); // Comment if stdio has been used!!!

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
    vector<int> mulled = csr.mul(vec, 3);
    for(uint i=0; i<vect.size(); i++){
        for(uint l=0; l<vect[0].size(); l++){
            cout << csr.val(i, l) << " ";
        }
        cout << "\n";
    }
    for(uint i=0; i<mulled.size(); i++){
        
        cout << mulled[i] <<"\n";
    }

    csr.write("test.txt");

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