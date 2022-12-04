#ifndef CSRMATRIX_H
#define CSRMATRIX_H

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits.h>
#include <omp.h>

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
    vector<double> tim_arr;
    int no_of_iterations=0;
    vector<string> arr_dict;
    // Write matrix to file
    void write(const string &filename){
        string buffer;
        buffer += to_string(this->row);
        buffer += ",";
        buffer += to_string(this->col);
        buffer += "\n";
        buffer += join(vector_to_string(row_begin), ",");
        buffer += "\n";
        buffer += join(vector_to_string(values), ",");
        buffer += "\n";
        buffer += join(vector_to_string(col_indices), ",");
        buffer += "\n";
        buffer += join(arr_dict, ",");
        buffer += "\n";
        // Write buffer to file
        ofstream csv(filename);
        csv << buffer;
        csv.close();
    }

    pair<uint, uint> get_size(){
        return {row, col};
    }

    // Initialize matrix from file
    CSR_Matrix(const string &filename){
        tim_arr.resize(8, 0);
        ifstream csv(filename);
        string str_init, str_row, str_val, str_col;
        csv >> str_init >> str_row >> str_val >> str_col;
        cout << "Read file" << endl;
        vector<uint>temp = string_to_uivector(str_init, ",");
        this->row = temp[0];
        this->col = temp[1];
        row_begin = string_to_uivector(str_row, ",");
        values = string_to_dvector(str_val, ",");
        col_indices = string_to_uivector(str_col, ",");
        arr_dict = string_to_svector(str_col, ",");

    }

    // Special array initializator
    // Time passed: 156651, must be parallelized somehow
    CSR_Matrix( vector<unordered_set<int>> &link_to,
                vector<vector<int>> &link_by,
                unordered_map<string, int> &name_dict,
                vector<string> &arr_dict){
        this->arr_dict = arr_dict;
        tim_arr.resize(8, 0);
        this->col = arr_dict.size();
        this->row = arr_dict.size();

        // This loop is not directly parallelizable, nested for loops should be parallelized.
        for(int i=0; i<this->col; i++){
            if(i%100000==0){
                cout << i << " " << i << " " << link_by[i].size() << endl;
            }

            uint old_size = values.size();
            values.resize(values.size()+link_by[i].size());
            col_indices.resize(values.size());
            if(values.size()==old_size){
                row_begin.push_back(UINT_MAX);
            }else{
                row_begin.push_back(old_size);
            }

            // This loop seems parallelizable
            //for(auto x: node[arr_dict[i]]){
            
            for(int l=0; l<link_by[i].size(); l++){
                assert(link_to[link_by[i][l]].size()!=0);
                values[old_size+l]=1/(link_to[link_by[i][l]].size());
                col_indices[old_size+l]=link_by[i][l];
            }
            // Sorting is not required
            //sort(col_indices.begin()+old_size, col_indices.end());
        }
        row_begin.push_back(values.size());
        assert(values.size()==col_indices.size());
    }
    //Unused, keep for future potential use
    /*
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

    // Initialize an empty matrix
    CSR_Matrix(uint row, uint col){
        this->row = row;
        this->col = col;
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
    }*/

    // Parallelizable, but 5 iterations take 3879. Should be parallelized as project description requires it.
    vector<T> ops(const vector<T> &vec, T sca, T add){
        assert(vec.size()==this->col);
        vector<T> ret(this->row, add);
        uint i, end, l, beg;
        double tim_st, tim_end;
        // Parallelizable
        for(beg=0; row_begin[beg]==UINT_MAX; beg++);

        #pragma omp parallel for private(i, end, l, tim_st, tim_end) //schedule(static,3) // I am changing this with function.
        for(i=beg; i<this->row; i++){
            tim_st = omp_get_wtime( );
            for(end=i+1; row_begin[end]==UINT_MAX; end++);
            for(l=row_begin[i]; l<row_begin[end]; l++){
                ret[i] += (values[l] * vec[col_indices[l]] * sca);
            }
            tim_end = omp_get_wtime( );
            tim_arr[omp_get_thread_num()]+=tim_end-tim_st;
        }
        no_of_iterations += this->row - beg;
        return ret;
    }
};

#endif