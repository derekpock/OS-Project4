cmake_minimum_required(VERSION 3.10)
project(OS_Project4 C)

set(CMAKE_C_STANDARD 11)

include_directories(3way-mpi
        3way-openmp
        3way-pthread)

add_executable(OS_Project4
#        longestCommonSubstringSingle.c
#        readInFile.c
        singleThread.c
#        3way-openmp/openmp.c
#        3way-mpi/dwarves/mpi.c
#         3way-pthread/dwarves/pthread.c
        )

