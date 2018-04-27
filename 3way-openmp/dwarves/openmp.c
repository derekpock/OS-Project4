#define _GNU_SOURCE
#include <omp.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

char* findLongestSubstring(char* a, char* b);
void threadRun(int threadNumber, int numberOfThreads, unsigned long numberOfLines, char** fileData, char** results);

// Simple linked list structure holding i and j index components.
struct ListItem {
    struct ListItem* nextItem;
    unsigned long i;
    unsigned long j;
};

int main(int argc, char *argv[]) {
    unsigned long numberOfLinesToProcess = 1000;
    int numberOfThreads = 1;
    char verbosity = 1;

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
        } else if (strncmp(argv[i], "--threads=", 10) == 0) {
            numberOfThreads = (int)strtol(argv[i] + 10, NULL, 10);
            if(numberOfThreads < 1) {
                printf("Invalid Argument: number of threads must be at least 1.\n");
                return -1;
            }
        } else if (strcmp(argv[i], "-t") == 0 && i != (argc - 1)) {
            numberOfThreads = (int)strtol(argv[i + 1], NULL, 10);
            if(numberOfThreads < 1) {
                printf("Invalid Argument: number of threads must be at least 1.\n");
                return -1;
            }
            i++;
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
            return -1;
        }
    }

//    char* a = "asdfghj\r\n";
//    char* b = "asdfghj";
//    printf("%s\n", findLongestSubstring(a, b));
//    return 0;
    // Prepare and open the file.
    char* filePath = "/homes/dan/625/wiki_dump.txt";
//    char* filePath = "C:\\OS-Project4\\wiki_dump.txt";

    if(verbosity == 2) {
        printf("Running with %d threads on %lu lines.\n", numberOfThreads, numberOfLinesToProcess);
        printf("File path: %s\n", filePath);
    }

    FILE *fp = fopen(filePath, "r");
    if(fp == NULL) {
        printf("Error! Unable to open file!");
        return -1;
    }

    // Read in the file line by line. Put data in fileData.
    unsigned long lineNumber = 0;
    long read;
    char* line = NULL;
    unsigned long lineLength;
    char** fileData = NULL;
    while (lineNumber < numberOfLinesToProcess && (read = getline(&line, &lineLength, fp)) != -1) {
        fileData = realloc(fileData, sizeof(char*) * (lineNumber + 1));
        if(fileData == NULL) {
            printf("Error! Unable to allocate memory for fileData: size %lu\n", (lineNumber + 1));
            return -1;
        }
        fileData[lineNumber++] = line;
        line = NULL;    // Dereference line to keep fileData unchanged while reading next line.
    }
    if(fileData == NULL) {
        printf("Error! Unable to read from file!\n");
        return -1;
    }
    fclose(fp);

    // Prepare the results.
    char** results = malloc(sizeof(char*) * (lineNumber - 1));     // With 100 lines, we have 99 comparisons.
    if(results == NULL) {
        printf("Error! Unable to allocate memory for results: size %lu\n", (lineNumber - 1));
        return -1;
    }

    // Compare all of the substrings. Begin thread section.
    omp_set_num_threads(numberOfThreads);

#pragma omp parallel
    {
        threadRun(omp_get_thread_num(), numberOfThreads, lineNumber, fileData, results);
    }

    if(verbosity != 0) {
        // End thread section. Print the results.
        for(unsigned long i = 0; i < (lineNumber - 1); i++) {
//        printf("%lu-%lu: '%s'\n", i, (i + 1), results[i]);
            printf("%lu-%lu: %s\n", i, (i + 1), results[i]);
        }
    }

    // Free all memory.
    for(unsigned long i = 0; i < (lineNumber - 1); i++) {
        free(results[i]);
        free(fileData[i]);
    }
    free(fileData[lineNumber]);
    free(results);
    free(fileData);
}

void threadRun(int threadNumber, int numberOfThreads, unsigned long numberOfLines, char** fileData, char** results) {
    // Determine our quota.
    unsigned long quota = numberOfLines / numberOfThreads;
    if(quota * numberOfThreads < numberOfLines) {
        quota++;
    }

    // Determine our first and last lines.
    unsigned long firstLine = quota * threadNumber;           // First line we need to compare - inclusive.
    unsigned long lastLine = quota * (threadNumber + 1);      // Last line we need to compare - inclusive.
    if(threadNumber == (numberOfThreads - 1)) {
        lastLine = numberOfLines - 1;                   // If we are the last thread, ensure we get all of the lines.
    }

    // Correct quota if necessary.
    quota = lastLine - firstLine;
    if(quota < 0) {
        quota = 0;
    }

    // Complete quota to local results.
    char** localResults = malloc(sizeof(char*) * quota);
    if(localResults == NULL) {
        printf("Error! Unable to allocate memory for localResults: size %lu\n", quota * sizeof(char*));
        exit(-1);
    }
    for(int i = 0; i < quota; i++) {
        localResults[i] = findLongestSubstring(fileData[i + firstLine], fileData[i + firstLine + 1]);
//        results[i + firstLine] = findLongestSubstring(fileData[i + firstLine], fileData[i + firstLine + 1]);
//        char* line = findLongestSubstring(fileData[i + firstLine], fileData[i + firstLine + 1]);
//        printf("%lu: %s\n", (i + firstLine), line);
//        results[firstLine + i] = line;
    }

    // Copy local results to final results.
    for(int i = 0; i < quota; i++) {
        results[i + firstLine] = localResults[i];
    }
    free(localResults);
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
        exit(-1);
    }
    struct ListItem *recentList = prefixList;

    // Prepare array of (a+1) x (b+1) size.
    // The first row and column are 0's, every row after that is comparing a index to b index.
    char *set = calloc((aLength + 1) * (bLength + 1), sizeof(char));
    if (set == NULL) {
        printf("Error! Unable to allocate memory for localResults: size %lu\n",
               sizeof(char) * (aLength + 1) * (bLength + 1));
        exit(-1);
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
                    exit(-1);
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
        exit(-1);
    }
    for (unsigned long i = 0; i < longestValue; i++) {
        longestString[i] = a[i + longestIndex];
    }
    longestString[longestValue] = '\0';
    return longestString;
}
