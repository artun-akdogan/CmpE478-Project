# How to run on linux:
* sudo apt install libomp-dev
* g++ -std=c++14 -fopenmp -DNDEBUG main.cpp -oprogram
* chmod +x ./program
* ./program

If all fails, please find precompiled program_backup at this directory.

# Arguments
### ./program load backup.csv

Reads CSR matrix from specified file. (You can change backup.csv file)

### ./program save backup.csv

Initialises CSR matrix from local graph.txt file, then writes it to specified file. (You can change backup.csv file)

### ./program

Initialises CSR matrix from local graph.txt file. No other file is interfered with.

# Files read and written on runtime:
### graph.txt

This is the web graph file. It should be on the same directory with the program, or it will not work. It is read and parsed to create CSR matrix. Please refer to [Erdos Web Graph](https://web.archive.org/web/20220310125510/http://web-graph.org/index.php/download).

### log.csv

CSV file that is written to show runtime speed comparison.

### result.csv

Top 5 results from running the program.