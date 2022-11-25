#ifndef CSV_H
#define CSV_H

#include <fstream>
#include <vector>
#include <string>

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

inline vector<double> string_to_dvector(const string &str, const string &delimiter){
    vector<double> ret;
    size_t last = 0;
    size_t next = 0;
    while ((next = str.find(delimiter, last)) != string::npos) {
        string temp = str.substr(last, next-last);
        ret.push_back(stod(temp));
        last = next + 1;
    }
    string temp = str.substr(last);
    ret.push_back(stod(temp));
    return ret;
}
inline vector<unsigned int> string_to_uivector(const string &str, const string &delimiter){
    vector<unsigned int> ret;
    size_t last = 0;
    size_t next = 0;
    while ((next = str.find(delimiter, last)) != string::npos) {
        string temp = str.substr(last, next-last);
        ret.push_back(stoul(temp));
        last = next + 1;
    }
    string temp = str.substr(last);
    ret.push_back(stoi(temp));
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

#endif