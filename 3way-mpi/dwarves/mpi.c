#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include "mpi.h"

char* findLongestSubstring(char* a, char* b);
void threadRun(int threadNumber, int numberOfThreads, unsigned long numberOfLines, char** fileData, char** results);

// Simple linked list structure holding i and j index components.
struct ListItem {
    struct ListItem* nextItem;
    unsigned long i;
    unsigned long j;
};

int main(int argc, char *argv[]) {
    int threadId;
    int numOfThreads;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &threadId);
    MPI_Comm_size(MPI_COMM_WORLD, &numOfThreads);

    unsigned long numberOfLinesToProcess = 1000;
    unsigned long localQuota;
    unsigned long firstLine;
    unsigned long lastLine;
    char verbosity = 1;
    unsigned long lineNumber = 0;
    char** fileData = NULL;
    char** results;
    unsigned long *line_sizes;

    printf("Thread-%d: initialized\n", threadId);

    if(threadId == 0) {
        for(int i = 1; i < argc; i++) {
            if(strncmp(argv[i], "--lines=", 8) == 0) {
                numberOfLinesToProcess = strtoul(argv[i] + 8, NULL, 10);
            } else if (strcmp(argv[i], "-l") == 0 && i != (argc - 1)) {
                numberOfLinesToProcess = strtoul(argv[i + 1], NULL, 10);
                i++;
            } else if (strcmp(argv[i], "-q") == 0) {
                verbosity = 0;
            } else if (strcmp(argv[i], "-v") == 0) {
                verbosity = 2;
//        } else if (strncmp(argv[i], "--threads=", 10) == 0) {
//            numberOfThreads = (int)strtol(argv[i] + 10, NULL, 10);
//            if(numberOfThreads < 1) {
//                printf("Invalid Argument: number of threads must be at least 1.\n");
//                return -1;
//            }
//        } else if (strcmp(argv[i], "-t") == 0 && i != (argc - 1)) {
//            numberOfThreads = (int)strtol(argv[i + 1], NULL, 10);
//            if(numberOfThreads < 1) {
//                printf("Invalid Argument: number of threads must be at least 1.\n");
//                return -1;
//            }
//            i++;
            } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                printf("Arguments:\n"
                       "--lines=# (-l #)     Set the number of lines to process from the input file.\n"
                       "--threads=# (-t #)   Set the number of threads to utilize for processing.\n"
                       "-v                   Enable verbose output.\n"
                       "-q                   Silence all output.\n"
                       "--help (-h)          Display this help.\n");
                return 0;
            } else {
                printf("Invalid Argument: unknown option: %s\n", argv[i]);
                MPI_Abort(MPI_COMM_WORLD, -1);
            }
        }
        // Prepare and open the file.
        char* filePath = "/homes/dan/625/wiki_dump.txt";
//        char* filePath = "C:\\OS-Project4\\wiki_dump.txt";

        if(verbosity == 2) {
            printf("Running with %d threads on %lu lines.\n", numOfThreads, numberOfLinesToProcess);
            printf("File path: %s\n", filePath);
        }

        FILE *fp = fopen(filePath, "r");
        if(fp == NULL) {
            printf("Error! Unable to open file!");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        // Read in the file line by line. Put data in fileData.
        long read;
        char* line = NULL;
        unsigned long lineLength;
        while (lineNumber < numberOfLinesToProcess && (read = getline(&line, &lineLength, fp)) != -1) {
            fileData = realloc(fileData, sizeof(char*) * (lineNumber + 1));
            if(fileData == NULL) {
                printf("Error! Unable to allocate memory for fileData: size %lu\n", (lineNumber + 1));
                MPI_Abort(MPI_COMM_WORLD, -1);
            }
            fileData[lineNumber++] = line;
            line = NULL;    // Dereference line to keep fileData unchanged while reading next line.
        }
        if(fileData == NULL) {
            printf("Error! Unable to read from file!\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        fclose(fp);

        line_sizes = malloc(sizeof(unsigned long) * lineNumber);
        for (int i = 0; i < lineNumber; i++) {
            line_sizes[i] = strlen(fileData[i]) + 1;
        }

        // Prepare the results.
        results = malloc(sizeof(char*) * (lineNumber - 1));     // With 100 lines, we have 99 comparisons.
        if(results == NULL) {
            printf("Error! Unable to allocate memory for results: size %lu\n", (lineNumber - 1));
            return -1;
        }

        printf("Thread-%d: file loaded\n", threadId);
    }

    printf("Thread-%d: broadcast lines\n", threadId);
    // Broadcast number of lines
    MPI_Bcast(&lineNumber, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    // Determine global quota.
    unsigned long quota = lineNumber / numOfThreads;
    if(quota * numOfThreads < lineNumber) {
        quota++;
    }

    printf("Thread-%d: global quota\n", threadId);
    // Thread 0 start dispatching work.
    if(threadId == 0) {
        printf("Thread-%d: dispatching data\n", threadId);
        /// DISPATCH DATA TO THREADS
        for (int i = 1; i < numOfThreads; i++) {
            // Determine next thread's first and last lines.
            firstLine = quota * i;           // First line we need to compare - inclusive.
            lastLine = quota * (i + 1);      // Last line we need to compare - inclusive.
            if (i == (numOfThreads - 1)) {
                lastLine =
                        lineNumber - 1;                   // If we are the last thread, ensure we get all of the lines.
            }

            // Correct quota if necessary.
            localQuota = lastLine - firstLine;
            if (quota < 0) {
                quota = 0;
            }


            printf("Thread-%d: sending data to %d\n", threadId, i);
            // Using j <= localQuota because thread x compares line n to n+1, and thread (x+1) compares line n+1 to n+2...
            for (unsigned long j = 0; j <= localQuota; j++) {
                // Send line (j + firstLine) to thread i
                MPI_Send(&line_sizes[j + firstLine], 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD);
                for (unsigned long k = 0; k < line_sizes[j + firstLine]; k++) {
                    MPI_Send(&(fileData[j + firstLine][k]), 1, MPI_UNSIGNED_CHAR, i, j * 10000 + k, MPI_COMM_WORLD);
                }
            }
        }


        printf("Thread-%d: getting personal info\n", threadId);
        // Determine next thread's first and last lines.
        firstLine = 0;           // First line we need to compare - inclusive.
        lastLine = quota;      // Last line we need to compare - inclusive.
        if ((numOfThreads) == 1) {
            lastLine = lineNumber - 1;      // If we are the last thread, ensure we get all of the lines.
        }
        // Correct quota if necessary.
        localQuota = lastLine - firstLine;
        if (quota < 0) {
            quota = 0;
        }
    } else {
        printf("Thread-%d: receiving data\n", threadId);
        /// RECEIVE DISPATCHED DATA
        firstLine = quota * threadId;
        lastLine = quota * (threadId) + 1;
        if((threadId) == (numOfThreads) - 1) {
            lastLine = lineNumber - 1;
        }

        // Correct quota if necessary.
        localQuota = lastLine - firstLine;
        if(quota < 0) {
            quota = 0;
        }

        fileData = malloc(sizeof(char*) * (localQuota + 1));
        results = malloc(sizeof(char*) * localQuota);
        for(unsigned long j = 0; j <= localQuota; j++) {
            unsigned long lineLength = 0;
            MPI_Recv(&lineLength, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD,  MPI_STATUS_IGNORE);
            fileData[j] = malloc(sizeof(char) * lineLength);
            for(unsigned long k = 0; k < lineLength; k++) {
                MPI_Recv(&(fileData[j][k]), 1, MPI_UNSIGNED_CHAR, 0, j*10000 + k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }

    int start = 0;
    if(threadId == 0) {
        start = 1;
    }
    printf("Thread-%d: awaiting start broadcast\n", threadId);
    MPI_Bcast(&start, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /// RUN OPERATION
    printf("Thread-%d: running\n", threadId);
    threadRun(threadId, numOfThreads, localQuota, fileData, results);
    printf("Thread-%d: finished\n", threadId);

    int finish = 0;
    if(threadId == 0) {
        finish = 1;
    }
    printf("Thread-%d: awaiting finish broadcast\n", threadId);
    MPI_Bcast(&finish, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(threadId == 0) {
        /// COLLECT DATA FROM THREADS
        for(int i = 1; i < numOfThreads; i++) {
            printf("Thread-%d: collectino from %d\n", threadId, i);
            // Determine next thread's first and last lines.
            unsigned long firstLine = quota * i;           // First line we need to compare - inclusive.
            unsigned long lastLine = quota * (i + 1);      // Last line we need to compare - inclusive.
            if(i == (numOfThreads - 1)) {
                lastLine = lineNumber - 1;                   // If we are the last thread, ensure we get all of the lines.
            }

            // Correct quota if necessary.
            localQuota = lastLine - firstLine;
            if(quota < 0) {
                quota = 0;
            }

            for(unsigned long j = 0; j < localQuota; j++) {
                // Put line (j + firstLine) from thread i into results
                unsigned long lineLength = 0;
                MPI_Recv(&lineLength, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                results[j + firstLine] = malloc(sizeof(char) * lineLength);
                for(unsigned long k = 0; k < lineLength; k++) {
                    MPI_Recv(&(results[j + firstLine][k]), 1, MPI_UNSIGNED_CHAR, i, j*10000 + k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
        }
        if(verbosity != 0) {
            for(unsigned long i = 0; i < (lineNumber - 1); i++) {
                printf("%lu-%lu: %s\n", i, (i + 1), results[i]);
            }
        }
    } else {
        /// SEND DATA TO BE COLLECTED
        line_sizes = malloc(sizeof(unsigned long) * localQuota);

        printf("Thread-%d: sending data to be collected\n", threadId);
        for(unsigned long j = 0; j < localQuota; j++) {
            line_sizes[j] = strlen(results[j]) + 1;
            MPI_Send(&line_sizes[j], 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
            for(unsigned long k = 0; k < line_sizes[j]; k++) {
                MPI_Send(&(results[j][k]), 1, MPI_UNSIGNED_CHAR, 0, j*10000 + k, MPI_COMM_WORLD);
            }
        }
    }





//    // Prepare the results.
//    results = calloc(sizeof(char*), (lineNumber - 1));     // With 100 lines, we have 99 comparisons.
//    if(results == NULL) {
//        printf("Error! Unable to allocate memory for results: size %lu\n", (lineNumber - 1));
//        MPI_Abort(MPI_COMM_WORLD, -1);
//    }
//
//    // For all the other threads
//    if(threadId != 0) {
//        // Create the line_sizes array before receiving info
//        line_sizes = malloc(sizeof(unsigned long) * lineNumber);
//        // Create the fileData array before receiving info
//        fileData = malloc(sizeof(char*) * (lineNumber));
//    }
//    // Broadcast the line numbers.
//    MPI_Bcast(&line_sizes, lineNumber, MPI_INT, 0, MPI_COMM_WORLD);
//    for (int i = 0; i < lineNumber; i++) {
//        // Broadcast each line in the fileData.
//        MPI_Bcast(&fileData[i], line_sizes[i], MPI_CHAR, 0, MPI_COMM_WORLD);
//    }
//
//    // Compare all of the substrings. Begin thread section.
//    threadRun(threadId, numOfThreads, lineNumber, fileData, results);
//
//    char** receiveBuffer;
//
//    if(threadId == 0) {
//        receiveBuffer = malloc(sizeof(char**) * (lineNumber - 1));
//    }
//
//    for(int i = 0; i < lineNumber; i++) {
//        unsigned long strLength;
//        if(results[i] != 0) {
//            strLength = strlen(results[i]) + 1;
//            MPI_Bcast(&strLength, 1, MPI_UNSIGNED_LONG, threadId, MPI_COMM_WORLD);
//        }
//        MPI_Reduce(results[i],  3, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
//    }


    printf("Thread-%d: freeing memory\n", threadId);

    if(threadId == 0) {
        for(unsigned long i = 0; i < (lineNumber - 1); i++) {
            free(results[i]);
            free(fileData[i]);
        }
        free(fileData[lineNumber]);
    } else {
        for(unsigned long i = 0; i < localQuota; i++) {
            free(results[i]);
            free(fileData[i]);
        }
        free(fileData[localQuota]);
    }
    // Free all memory.
    free(results);
    free(fileData);
    free(line_sizes);
    MPI_Finalize();
}

void threadRun(int threadNumber, int numberOfThreads, unsigned long numberOfLines, char** fileData, char** results) {
    // Complete quota to local results.
    for(int i = 0; i < numberOfLines; i++) {
//        printf("%s from %d\n", fileData[i], threadNumber);
        results[i] = findLongestSubstring(fileData[i], fileData[i + 1]);
//        results[i + firstLine] = findLongestSubstring(fileData[i + firstLine], fileData[i + firstLine + 1]);
//        char* line = findLongestSubstring(fileData[i + firstLine], fileData[i + firstLine + 1]);
//        printf("%lu: %s\n", (i + firstLine), line);
//        results[firstLine + i] = line;
    }
}

// Returns the longest common string between a and b. Be sure to free the returned char* when done.
char* findLongestSubstring(char* a, char* b) {
    // Get lengths of the strings.
    unsigned long aLength = strlen(a);
    unsigned long bLength = strlen(b);

    // Prepare linked list.
    struct ListItem *prefixList = malloc(sizeof(struct ListItem));
    if (prefixList == NULL) {
        printf("Error! Unable to allocate memory for prefixList: size %lu\n", sizeof(struct ListItem));
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    struct ListItem *recentList = prefixList;

    // Prepare array of (a+1) x (b+1) size.
    // The first row and column are 0's, every row after that is comparing a index to b index.
    char *set = calloc((aLength + 1) * (bLength + 1), sizeof(char));
    if (set == NULL) {
        printf("Error! Unable to allocate memory for localResults: size %lu\n",
               sizeof(char) * (aLength + 1) * (bLength + 1));
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    unsigned numOfElements = 0;

    // Go through each character. Compare a[i] to b[j] and mark it's corresponding location in set to 1 if true.
    for (unsigned long i = 0; i < aLength; i++) {
        for (unsigned long j = 0; j < bLength; j++) {
            // Mark 0 row.
            if (i == 0 || j == 0) {
                set[((i) * bLength) + j] = 0;
            }
            // Compare a at i and b at j. Note the index access in set is different since buffer at 0 row/col.
            if (a[i] == b[j]) {
                unsigned long index = ((i + 1) * bLength) + j + 1;

                // Create new item in the linked-list
                recentList->nextItem = malloc(sizeof(struct ListItem));
                if (recentList->nextItem == NULL) {
                    printf("Error! Unable to allocate memory for localResults: size %lu\n", sizeof(struct ListItem));
                    MPI_Abort(MPI_COMM_WORLD, -1);
                }
                recentList = recentList->nextItem;
                recentList->i = i;
                recentList->j = j;
                numOfElements++;

                // Set the value of the array.
                set[index] = 1;
            }
        }
    }

    // Prepare for searching set.
    unsigned long longestIndex = 0;
    unsigned long longestValue = 0;
    struct ListItem *currentList = prefixList->nextItem;
    free(prefixList);

    // For each item in the linked list (a 1 initially), start a check diagonally for the longest common substring.
    for (unsigned long k = 0; k < numOfElements; k++) {
        // Check diagonal.
        unsigned long i = currentList->i;
        unsigned long j = currentList->j;
        unsigned long it = i;
        unsigned long jt = j;
        while (set[((it + 1) * bLength) + jt + 1] == 1) {
            //set[((it + 1) * bLength) + jt + 1] = 0;
            // Optimization: set a checked value to 0 to prevent sub-checks of sub-substrings.
            // EG - If we have "always true" as the longest string, without this optimization, we will end up
            // checking "lways true" as the longest string next row. Set to 0 to prevent sub-checks.
            it++;
            jt++;
        }

        // Compare with longest value found so far.
        if (it - i > longestValue) {
            longestValue = it - i;
            longestIndex = i;
        }

        // Go to the next item.
        struct ListItem *nextList = currentList->nextItem;
        free(currentList);
        currentList = nextList;
    }
//    // Print a visual of the graph. Used for testing / visualization.
//    // Purely aesthetic, will be removed in performance.
//    printf(" %s\n", b);
//    for(int i = 0; i < aLength; i++) {
//        for(int j = 0; j < bLength; j++) {
//            if(j == 0) {
//                printf("X", a[i]);
//            }
//            if(set[((i + 1) * bLength) + j + 1] == 1) {
//                printf("\\");
//            } else {
//                printf(" ");
//            }
//        }
//        printf("\n");
//    }
    free(set);

    // Create the longest string found.
    char *longestString = malloc(sizeof(char) * (longestValue + 1));
    if (longestString == NULL) {
        printf("Error! Unable to allocate memory for longestString: size %lu\n", sizeof(longestValue + 1));
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    for (unsigned long i = 0; i < longestValue; i++) {
        longestString[i] = a[i + longestIndex];
    }
    longestString[longestValue] = '\0';
    return longestString;
}
