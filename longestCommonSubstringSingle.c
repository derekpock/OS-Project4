
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>

struct ListItem {
    struct ListItem* nextItem;
    size_t i;
    size_t j;
};

/*
 * Dual linked list and array
 * Link list values of 1, array values to get diagonal entries
 */

void findLongestSubstring(char* a, char* b);

int main() {
    char* string1 = "This is a test string.";
    char* string2 = "This is also a nice alpha test string.";
    findLongestSubstring(string1, string2);
}

void findLongestSubstring(char* a, char* b) {
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
//                recentList->valueIndex = index;
//                recentList->valueAddr = set + index;

                // Set the value of the array.
                set[index] = 1;
            }
        }
    }
//
//    // Print a visual of the graph. Used for testing / visualization.
//    // Purely aesthetic, will be removed in performance.
//    printf(" %s\n", b);
//    for(int i = 0; i < aLength; i++) {
//        for(int j = 0; j < bLength; j++) {
//            if(j == 0) {
//                printf("%c", a[i]);
//            }
//            if(set[((i + 1) * bLength) + j + 1] == 1) {
//                printf("\\");
//            } else {
//                printf(" ");
//            }
//        }
//        printf("\n");
//    }

    // Search the graph for the longest common sequences of diagonal 1's.
    size_t longestIndex = 0;
    size_t longestValue = 0;
    struct ListItem* currentList = prefixList->nextItem;
    while(currentList != NULL) {
        // Match found, check diagonal.
        size_t i = currentList->i;
        size_t j = currentList->j;
        size_t it = i;
        size_t jt = j;
        //Cannot optimize set to zero (below) with multiple threads. Linked list is not in order.
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
        currentList = currentList->nextItem;
    }
//
//    for(size_t i = 0; i < aLength; i++) {
//        for(size_t j = 0; j < bLength; j++) {
//            if(set[((i + 1) * bLength) + j + 1] == 1) {
//                // Match found, check diagonal.
//                size_t it = i + 1;
//                size_t jt = j + 1;
//                // Optimization: set checked values to 0 to prevent them from being re-checked later.
//                // Example, 22 length diagonal will be checked next row for 21 length, 20 length, etc...
//                set[((i + 1) * bLength) + j + 1] = 0;   // Optimization ^
//                while(set[((it + 1) * bLength) + jt + 1] == 1) {
//                    set[((it + 1) * bLength) + jt + 1] = 0; // Optimization ^
//                    it++;
//                    jt++;
//                }
//                // Compare with longest value found so far.
//                if(it - i > longestValue) {
//                    longestValue = it - i;
//                    longestIndex = i;
//                }
//            }
//        }
//    }
    // Print the longest string found.
    printf("\n'");
    for(size_t i = 0; i < longestValue; i++) {
        printf("%c", a[i + longestIndex]);
    }
    printf("'\n");
}