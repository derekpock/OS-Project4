#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    char* filePath = "/homes/dan/625/wiki_dump.txt";
//    char* filePath = "../readInFile.c";
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
    for(int i = 0; i <  lineNumber; i++) {
        free(fileData[i]);
    }
}