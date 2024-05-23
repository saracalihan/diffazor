#include <stdio.h>
#include <string.h>
#include "plugin.h"

typedef unsigned long int matrix_length_t;

TextVector lcs(TextVector *X, TextVector *Y) {
    matrix_length_t
        m = X->count,
        n = Y->count,
        size = m > n ? m : n;
    TextVector diff = {0};
    diff.items= malloc(sizeof(TextDiff) * size*2);
    diff.count = 0;
    diff.line_count =
        X->line_count > Y->line_count
            ? X->line_count
            : Y->line_count;

    // TODO: use malloc
    // short int** L= malloc(sizeof(short) * ((m+1)+(n+1)));
    short int L[m+1][n+1];
    if(L == NULL){
        perror("LCS algorithm matris malloc error!\n");
    }

    // Set the LCS table
    for (matrix_length_t i = 0; i <= m; i++) {
        for (matrix_length_t j = 0; j <= n; j++) {
            if (i == 0 || j == 0)
                L[i][j] = 0;
            else if (X->items[i-1].c == Y->items[j-1].c)
                L[i][j] = L[i-1][j-1] + 1;
            else
                L[i][j] = (L[i-1][j] > L[i][j-1]) ? L[i-1][j] : L[i][j-1];
        }
    }

    // Find diffs
    matrix_length_t i = m, j = n;
    matrix_length_t index = L[m][n];
    char lcs[index + 1];
    lcs[index] = '\0'; // Null-terminate

    while (i > 0 && j > 0) {
        if (X->items[i-1].c == Y->items[j-1].c) {
            lcs[index-1] = X->items[i-1].c;
            i--;
            j--;
            index--;
        } else if (L[i-1][j] > L[i][j-1]) {
            i--;
        } else {
            j--;
        }
    }

    // Set diffs
    i = 0;
    j = 0;
    matrix_length_t itemIndex =0;
    matrix_length_t k = 0;
    while (i < m && j < n) {
        if (k < (matrix_length_t)strlen(lcs) && X->items[i].c == lcs[k] && Y->items[j].c == lcs[k]) {
            diff.items[itemIndex] = (TextDiff){
                .c = X->items[i].c,
                .diff = SAME,
            };
            i++;
            j++;
            k++;
        } else if (k < (matrix_length_t)strlen(lcs) && X->items[i].c == lcs[k]) {
            diff.items[itemIndex] = (TextDiff){
                .c = Y->items[j].c,
                .diff = ADD,
            };
            j++;
        } else if (k < (matrix_length_t)strlen(lcs) && Y->items[j].c == lcs[k]) {
            diff.items[itemIndex] = (TextDiff){
                .c = X->items[i].c,
                .diff = DELETE,
            };
            i++;
        } else {
            diff.items[itemIndex] = (TextDiff){
                .c = X->items[i].c,
                .diff = DELETE,
            };
            i++;
        }
        diff.count++;
        itemIndex++;
    }

    // Add suffixed letters
    while (i < m) {
        diff.items[itemIndex] = (TextDiff){
                .c = X->items[i].c,
                .diff = DELETE,
        };
        i++;
        itemIndex++;
        diff.count++;
    }
    while (j < n) {
        diff.items[itemIndex] = (TextDiff){
                .c = Y->items[j].c,
                .diff = ADD,
        };
        j++;
        itemIndex++;
        diff.count++;
    }
    return diff;
}

