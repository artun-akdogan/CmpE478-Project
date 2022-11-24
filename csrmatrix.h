#ifndef CSRMATRIX_H
#define CSRMATRIX_H

#include <fstream>
#include <vector>
#include <algorithm>
#include <limits.h>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "csv.h"

using namespace std;
typedef unsigned int uint;

template<typename T>
T check_sub_ops(const vector<T> &vector1, const vector<T> &vector2){
    assert(vector1.size()==vector2.size());
    T res=0;
    for(uint i=0; i<vector1.size(); i++){
        res += vector1[i] - vector2[i];
    }
    return res;
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

    void insert_row(vector<T> row){
        assert(row.size()==this->col);
        row_begin.back()=UINT_MAX;
        for(int i=0; i<row.size(); i++){
            if(row[i]!=0){
                if(row_begin.back()==UINT_MAX){
                    row_begin.back() = values.size();
                }
                values.push_back(row[i]);
                col_indices.push_back(i);
            }
        }
        row_begin.push_back(values.size());
        this->row++;
    }

    pair<uint, uint> get_size(){
        return {row, col};
    }

    // Initialize an empty matrix
    CSR_Matrix(uint row, uint col){
        this.row = row;
        this.col = col;
        row_begin.resize(row+1, UINT_MAX);
        row_begin.back() = 0;
    }

    // Initialize a matrix from vector
    CSR_Matrix(vector<vector<T>> matrix){
        this->row = matrix.size();
        this->col = matrix[0].size();
        row_begin.resize(matrix.size()+1, UINT_MAX);
        for(uint i=0; i<matrix.size(); i++){
            assert(matrix[0].size()==matrix[i].size()); // Assert fixed matrix in column number
            for(uint l=0; l<matrix[0].size(); l++){
                if(matrix[i][l]!=0){
                    if(row_begin[i]==UINT_MAX){
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

    vector<T> ops(const vector<T> &vec, T sca, T add){
        assert(vec.size()==this->col);
        vector<T> ret(this->row);
        for(uint i=0; i<this->row; i++){
            for(uint l=row_begin[i]; l<row_begin[i+1]; l++){
                ret[i] += (values[l] * vec[col_indices[l]] * sca) + add;
            }
        }
        return ret;
    }
};

#endif