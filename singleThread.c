#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

void findLongestSubstring(char* a, char* b, int aLineNumber);

int main() {
    char* filePath = "/homes/dan/625/wiki_dump.txt";
    FILE *fp;
    fp = fopen(filePath, "r");
    if(fp == NULL) {
        printf("Error! Unable to read file!");
        return -1;
    }

    char* line = NULL;
    char** fileData = malloc(sizeof(char*));
    size_t lineLength = 0;
    long read;
    int lineNumber = 0;

    while ((read = getline(&line, &lineLength, fp)) != -1) {
        fileData[lineNumber++] = line;
        fileData = realloc(fileData, sizeof(char*) * (lineNumber + 1));
        line = NULL;
    }
    fclose(fp);

    //do work
    for(int i = 0; i < 5; i++) {
        findLongestSubstring(fileData[i], fileData[i+1], i);
    }

    for(int i = 0; i < lineNumber; i++) {
        free(fileData[i]);
    }
    free(fileData);
}

struct ListItem {
    struct ListItem* nextItem;
    size_t i;
    size_t j;
};

/*
 * Dual linked list and array
 * Link list values of 1, array values to get diagonal entries
 */

void findLongestSubstring(char* a, char* b, int aLineNumber) {
    size_t aLength = strlen(a);
    size_t bLength = strlen(b);

    struct ListItem* prefixList = malloc(sizeof(struct ListItem));
    struct ListItem* recentList = prefixList;

    // Prepare array of (a+1) x (b+1) size.
    // The first row and column are 0's, every row after that is comparing a index to b index.
    char* set = calloc((aLength + 1) * (bLength + 1), sizeof(char));

    for(size_t i = 0; i < aLength; i++) {
        for(size_t j = 0; j < bLength; j++) {
            // Mark 0 row.
            if(i == 0 || j == 0) {
                set[((i)*bLength) + j] = 0;
            }
            // Compare a at i and b at j. Note the index access in set is different.
            if(a[i] == b[j]) {
                unsigned long index = ((i + 1) * bLength) + j + 1;

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

    // Search the graph for the longest common sequences of diagonal 1's.
    size_t longestIndex = 0;
    size_t longestValue = 0;
    struct ListItem* currentList = prefixList->nextItem;
    struct ListItem* nextList;
    free(prefixList);

    while(currentList != NULL) {
        // Match found, check diagonal.
        size_t i = currentList->i;
        size_t j = currentList->j;
        size_t it = i;
        size_t jt = j;
        while(set[((it + 1) * bLength) + jt + 1] == 1) {
            set[((it + 1) * bLength) + jt + 1] = 0;
            it++;
            jt++;
        }

        // Compare with longest value found so far.
        if(it - i > longestValue) {
            longestValue = it - i;
            longestIndex = i;
        }
        nextList = currentList->nextItem;
        free(currentList);
        currentList = nextList;
    }

    // Print the longest string found.
    printf("%d-%d: ", aLineNumber, aLineNumber + 1);
    for(size_t i = 0; i < longestValue; i++) {
        printf("%c", a[i + longestIndex]);
    }
    printf("\n");
}