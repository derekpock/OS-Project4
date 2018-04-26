#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

char* findLongestSubstring(char* a, char* b);
void threadRun(int threadNumber, int numberOfThreads, int numberOfLines, char** fileData, char** results);

// Simple linked list structure holding i and j index components.
struct ListItem {
    struct ListItem* nextItem;
    unsigned long i;
    unsigned long j;
};

int main() {
    char* filePath; // Contains path of the file.
    FILE *fp;       // Actual file data structure.
    char* line;     // Holds file lines while reading.
    unsigned long lineLength;   // Length of the previous line being read. Unused.
    char** fileData;    // Holds all of the file data information.
    char** results;     // Holds the resulting lines of each comparison.
    long read;          // Holds the number of characters read. Unused except for check.
    int lineNumber;     // Number of lines read, and number of total lines (when finished) in the file.
    int i;          // Index.

    // Prepare and open the file.
    filePath = "/homes/dan/625/wiki_dump.txt";
    fp = fopen(filePath, "r");
    if(fp == NULL) {
        printf("Error! Unable to read file!");
        return -1;
    }

    // Read in the file line by line. Put data in fileData.
    lineNumber = 0;
    fileData = NULL;
    while ((read = getline(&line, &lineLength, fp)) != -1) {
        fileData = realloc(fileData, sizeof(char*) * (lineNumber + 1));
        if(fileData == NULL) {
            printf("Error! Unable to allocate memory for fileData: size %d\n", (lineNumber + 1));
            return -1;
        }
        fileData[lineNumber++] = line;
        line = NULL;    // Dereference line to keep fileData unchanged while reading next line.
    }
    fclose(fp);

    // Prepare the results.
    results = malloc(sizeof(char*) * (lineNumber - 1));     // With 100 lines, we have 99 comparisons.
    if(results == NULL) {
        printf("Error! Unable to allocate memory for results: size %d\n", (lineNumber - 1));
        return -1;
    }

    // Compare all of the substrings. Begin thread section.
    threadRun(0, 1, lineNumber, fileData, results);

    // End thread section. Free all of the used data.
    for(i = 0; i < lineNumber; i++) {
        free(fileData[i]);
    }
    free(fileData);
}

void threadRun(int threadNumber, int numberOfThreads, int numberOfLines, char** fileData, char** results) {
    int quota;
    int firstLine;
    int lastLine;
    char** localResults;
    int i;

    // Determine our quota.
    quota = numberOfLines / numberOfThreads;
    if(quota * numberOfThreads < numberOfLines) {
        quota++;
    }

    // Determine our first and last lines.
    firstLine = quota * threadNumber;           // First line we need to compare - inclusive.
    lastLine = quota * (threadNumber + 1);      // Last line we need to compare - inclusive.
    if(threadNumber == (numberOfThreads - 1)) {
        lastLine = numberOfLines;                   // If we are the last thread, ensure we get all of the lines.
    }

    // Correct quota if necessary.
    quota = lastLine - firstLine;
    if(quota < 0) {
        quota = 0;
    }

    // Complete quota to local results.
    localResults = malloc(sizeof(char*) * quota);
    for(i = 0; i < quota; i++) {
        localResults[i] = findLongestSubstring(fileData[i + firstLine], fileData[i + firstLine +1]);
    }

    // Copy local results to final results.
    for(i = 0; i < quota; i++) {
        results[i + firstLine] = localResults[i];
    }
    free(localResults);
}

// Returns the longest common string between a and b. Be sure to free the returned char* when done.
char* findLongestSubstring(char* a, char* b) {
    unsigned long aLength;
    unsigned long bLength;
    unsigned long i;
    unsigned long j;
    unsigned long it;
    unsigned long jt;
    unsigned long index;
    unsigned long longestIndex;
    unsigned long longestValue;

    struct ListItem *prefixList;
    struct ListItem *recentList;
    struct ListItem *nextList;
    char* set;
    char* longestString;

    // Get lengths of the strings.
    aLength = strlen(a);
    bLength = strlen(b);

    // Prepare linked list.
    prefixList = malloc(sizeof(struct ListItem));
    recentList = prefixList;

    // Prepare array of (a+1) x (b+1) size.
    // The first row and column are 0's, every row after that is comparing a index to b index.
    set = calloc((aLength + 1) * (bLength + 1), sizeof(char));

    // Go through each character. Compare a[i] to b[j] and mark it's corresponding location in set to 1 if true.
    for(i = 0; i < aLength; i++) {
        for(j = 0; j < bLength; j++) {
            // Mark 0 row.
            if(i == 0 || j == 0) {
                set[((i)*bLength) + j] = 0;
            }
            // Compare a at i and b at j. Note the index access in set is different since buffer at 0 row/col.
            if(a[i] == b[j]) {
                index = ((i + 1) * bLength) + j + 1;

                // Create new item in the linked-list
                recentList->nextItem = malloc(sizeof(struct ListItem));
                recentList = recentList->nextItem;
                recentList->i = i;
                recentList->j = j;

                // Set the value of the array.
                set[index] = 1;
            }
        }
    }

    // Prepare for searching set.
    longestIndex = 0;
    longestValue = 0;
    recentList = prefixList->nextItem;
    free(prefixList);

    // For each item in the linked list (a 1 initially), start a check diagonally for the longest common substring.
    while(recentList != NULL) {
        // Check diagonal.
        i = recentList->i;
        j = recentList->j;
        it = i;
        jt = j;
        while(set[((it + 1) * bLength) + jt + 1] == 1) {
            set[((it + 1) * bLength) + jt + 1] = 0;
                // Optimization: set a checked value to 0 to prevent sub-checks of sub-substrings.
                // EG - If we have "always true" as the longest string, without this optimization, we will end up
                // checking "lways true" as the longest string next row. Set to 0 to prevent sub-checks.
            it++;
            jt++;
        }

        // Compare with longest value found so far.
        if(it - i > longestValue) {
            longestValue = it - i;
            longestIndex = i;
        }

        // Go to the next item.
        nextList = recentList->nextItem;
        free(recentList);
        recentList = nextList;
    }
    free(set);

    // Create the longest string found.
    longestString = malloc(sizeof(char) * (longestValue + 1));
    for(i = 0; i < longestValue; i++) {
        longestString[i] = a[i + longestIndex];
    }
    longestString[longestValue] = '\0';

    return longestString;
}