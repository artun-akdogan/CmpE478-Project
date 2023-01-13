#ifndef CSRMATRIX_H
#define CSRMATRIX_H

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <limits.h>

// Thrust
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/generate.h>
#include <thrust/sort.h>
#include <thrust/copy.h>

// Uncomment when building for production (disables assert)
// #define NDEBUG
#include <assert.h>

#include "csv.h"

using namespace std;
typedef unsigned int uint;

template<typename T>
struct saxpy_abs{
    __host__ __device__ T operator()(const T &a)const{
        return abs(a);
    }
};

struct saxpy_ops{
    const uint * row_begin;
    const uint * col_indices;
    const double * values;
    const double * vec;
    const float sca, add;

    saxpy_ops(uint *_row_begin, uint *_col_indices, double *_values, double *_vec, double _sca, double _add) 
        : row_begin(_row_begin), col_indices(_col_indices), values(_values), vec(_vec), sca(_sca), add(_add) {}

    __host__ __device__ double operator()(const size_t idx) const{
        double res = add;
        uint l, end;
        // Skip zero rows
        for(end=idx+1; row_begin[end]==UINT_MAX; end++);
        // Matrix multiplication
        for(l=row_begin[idx]; l<row_begin[end]; l++){
            // While multiplying, also multiply with the scaler
            res += (values[l] * vec[col_indices[l]] * sca);
        }
        return res;
    }
};

template<typename T>
class CSR_Matrix{
    private:
    uint row, col;
    // Value indices. Must be ordered
    thrust::device_vector<uint> _row_begin;
    thrust::device_vector<uint> _col_indices;
    //Non zero values in the matrix
    thrust::device_vector<T> _values;
    vector<uint> row_begin;
    vector<uint> col_indices;
    //Non zero values in the matrix
    vector<T> values;

    public:
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

    // This function is used 
    void transfer_device(){
        _row_begin = row_begin;
        _col_indices = col_indices;
        _values = values;
        row_begin.clear();
        col_indices.clear();
        values.clear();
    }

    // Get matrix size
    pair<uint, uint> get_size() const{
        return {row, col};
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
                row_begin.push_back(UINT_MAX);
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

    void ops(thrust::device_vector<double> &ret, thrust::device_vector<double> &vec, T sca, T add){
        assert(vec.size()==this->col);
        //uint i, end, l, beg;
        // Initialize with multiplied C vector
        //vector<T> ret(this->row, add/this->col);
        // add/this->col
        
        thrust::transform(  thrust::counting_iterator<uint>(0),
                            thrust::counting_iterator<uint>(this->row),
                            ret.begin(),
                            saxpy_ops(  thrust::raw_pointer_cast(_row_begin.data()),
                                        thrust::raw_pointer_cast(_col_indices.data()),
                                        thrust::raw_pointer_cast(_values.data()),
                                        thrust::raw_pointer_cast(vec.data()),
                                        sca,
                                        add/this->col)
                            );
        thrust::device_vector<double> _temp(this->row);
        thrust::transform(ret.begin(), ret.end(), vec.begin(), _temp.begin(), thrust::minus<double>());
        two_vec_diff = thrust::transform_reduce(_temp.begin(), _temp.end(), saxpy_abs<double>(), 0.0, thrust::plus<double>());

        /*
        // Skip zero rows
        for(beg=0; row_begin[beg]==UINT_MAX; beg++);
        two_vec_diff=0;

        // Parallelised for loop
        for(i=beg; i<this->row; i++){
            // Skip zero rows
            for(end=i+1; row_begin[end]==UINT_MAX; end++);
            // Matrix multiplication
            for(l=row_begin[i]; l<row_begin[end]; l++){
                // While multiplying, also multiply with the scaler
                ret[i] += (values[l] * vec[col_indices[l]] * sca);
            }
            // Log vector difference
            two_vec_diff+=abs(ret[i]-vec[i]);
        }
        */
    }
};

#endif