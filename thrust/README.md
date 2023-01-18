This program is the same algorithm implemented on thrust.

# Notes
* You need to have cuda installed on your computer. (Other configurations have not been tested)
* You need to install [thrust](https://github.com/NVIDIA/thrust)
* While building the application, please replace xx fields with your compute capability
  * For compute capability 5.0: nvcc main.cu -o program -gencode=arch=compute_50,code=sm_50

# How to run on linux:
* nvcc main.cu -o program -gencode=arch=compute_xx,code=sm_xx
* chmod +x ./program
* ./program
