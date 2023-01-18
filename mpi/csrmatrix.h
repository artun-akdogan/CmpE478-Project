#ifndef CSRMATRIX_H
#define CSRMATRIX_H

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <limits.h>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "csv.h"

using namespace std;
typedef unsigned int uint;

template<typename T>
class CSR_Matrix{
    private:

    public:
    uint row, col;
    // Value indices. Must be ordered
    vector<uint> row_begin;
    vector<uint> col_indices;
    //Non zero values in the matrix
    vector<T> values;
    // Those values are non essential to CSR matrix's runtime.
    // But if they shouldn't be changed without caution.
    double two_vec_diff=0;
    vector<string> arr_dict;
    // Write matrix to file
    void write(const string &filename){
        string buffer;
        // Write row
        buffer += to_string(this->row);
        buffer += ",";
        // Write col
        buffer += to_string(this->col);
        buffer += "\n";
        // Write row_begin array
        buffer += join(vector_to_string(row_begin), ",");
        buffer += "\n";
        // Write values array
        buffer += join(vector_to_string(values), ",");
        buffer += "\n";
        // Write col_indices array
        buffer += join(vector_to_string(col_indices), ",");
        buffer += "\n";
        // Write arr_dict array
        buffer += join(arr_dict, ",");
        buffer += "\n";
        // Write buffer to file
        ofstream csv(filename);
        csv << buffer;
        csv.close();
    }

    // Get matrix size
    pair<uint, uint> get_size() const{
        return {row, col};
    }

    // Initialize by row/column
    CSR_Matrix(uint row, uint col){
        this->row = row;
        this->col = col;
    }

    // Initialize matrix from file
    CSR_Matrix(const string &filename){
        ifstream csv(filename);
        string str_init, str_row, str_val, str_col, str_maps;

        // Read file rows
        csv >> str_init >> str_row >> str_val >> str_col >> str_maps;
        cout << "Read file" << endl;

        // Now parse them accordingly
        vector<uint>temp = string_to_uivector(str_init, ",");
        this->row = temp[0];
        this->col = temp[1];
        row_begin = string_to_uivector(str_row, ",");
        values = string_to_dvector(str_val, ",");
        col_indices = string_to_uivector(str_col, ",");
        arr_dict = string_to_svector(str_maps, ",");

    }

    // Special array initializator optimised for double node matrices.
    // Needs approximately 3secs to run. No need to parallelise.
    CSR_Matrix( vector<vector<int>> &link_to,
                vector<vector<int>> &link_by,
                unordered_map<string, int> &name_dict,
                vector<string> &arr_dict){
        // Set basic variables
        this->arr_dict = arr_dict;
        this->col = arr_dict.size();
        this->row = arr_dict.size();

        for(int i=0; i<this->col; i++){
            // Set array structure variables
            uint old_size = values.size();
            values.resize(values.size()+link_by[i].size());
            col_indices.resize(values.size());
            // If it is a zero row, then set according row_begin index to UINT_MAX
            if(values.size()==old_size){
                if(old_size==0)
                    row_begin.push_back(UINT_MAX);
                else
                    row_begin.push_back(row_begin.back());
            }else{
                row_begin.push_back(old_size);
            }

            // Set values and according column indices.
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

    vector<T> ops(const vector<T> &vec, T sca, T add, uint vecbeg=0){
        assert(vec.size()==this->col);
        uint i, end, l, beg;
        // Initialize with multiplied C vector
        vector<T> ret(this->row, add/this->col);

        // Skip zero rows
        for(beg=0; row_begin[beg]==UINT_MAX; beg++);
        two_vec_diff=0;
        // Parallelised for loop
        for(i=beg; i<this->row; i++){
            // Skip zero rows
            for(end=i+1; end<=this->row && row_begin[end]==row_begin[end-1] ; end++);
            // Matrix multiplication
            for(l=row_begin[i]; l<row_begin[end]; l++){
                // While multiplying, also multiply with the scaler
                ret[i] += (values[l] * vec[col_indices[l]] * sca);
            }
            // Log vector difference
            two_vec_diff+=abs(ret[i]-vec[i+vecbeg]);
        }
        return ret;
    }
};

#endif