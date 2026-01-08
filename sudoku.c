#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9

int sudoku[SIZE][SIZE];  
int valid[27];            

typedef struct {
    int row;
    int col;
    int index;  // شماره thread در آرایه valid
} parameters;

/----------------- چک کردن یک ردیف -----------------/
void *check_row(void *param) {
    parameters *p = (parameters *) param;
    int r = p->row;
    int seen[SIZE + 1] = {0};

    for (int j = 0; j < SIZE; j++) {
        int num = sudoku[r][j];
        if (num < 1 || num > 9 || seen[num]) {
            valid[p->index] = 0;
            pthread_exit(NULL);
        }
        seen[num] = 1;
    }

    valid[p->index] = 1;
    pthread_exit(NULL);
}

/ ----------------- چک کردن یک ستون ---------------/
void *check_col(void *param) {
    parameters *p = (parameters *) param;
    int c = p->col;
    int seen[SIZE + 1] = {0};

    for (int i = 0; i < SIZE; i++) {
        int num = sudoku[i][c];
        if (num < 1 || num > 9 || seen[num]) {
            valid[p->index] = 0;
            pthread_exit(NULL);
        }
        seen[num] = 1;
    }

    valid[p->index] = 1;
    pthread_exit(NULL);
}

/------------ چک کردن یک subgrid 3x3 --------------/
void *check_subgrid(void *param) {
    parameters *p = (parameters *) param;
    int startRow = p->row;
    int startCol = p->col;
    int seen[SIZE + 1] = {0};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int num = sudoku[startRow + i][startCol + j];
            if (num < 1 || num > 9 || seen[num]) {
                valid[p->index] = 0;
                pthread_exit(NULL);
            }
            seen[num] = 1;
        }
    }

    valid[p->index] = 1;
    pthread_exit(NULL);
}

/ ----------------- تابع اصلی ----------------- /
int main() {
    printf("Enter Sudoku (9x9) row by row:\n");
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (scanf("%d", &sudoku[i][j]) != 1) {
                printf("Invalid input!\n");
                return 1;
            }

    pthread_t threads[27];
    parameters params[27];
    int t = 0;

    // ردیف‌ها
    for (int i = 0; i < SIZE; i++) {
        params[t].row = i;
        params[t].index = t;
        pthread_create(&threads[t], NULL, check_row, &params[t]);
        t++;
    }

    // ستون‌ها
    for (int i = 0; i < SIZE; i++) {
        params[t].col = i;
        params[t].index = t;
        pthread_create(&threads[t], NULL, check_col, &params[t]);
        t++;
    }

    // subgridا
    for (int i = 0; i < SIZE; i += 3) {
        for (int j = 0; j < SIZE; j += 3) {
            params[t].row = i;
            params[t].col = j;
            params[t].index = t;
            pthread_create(&threads[t], NULL, check_subgrid, &params[t]);
            t++;
        }
    }

    for (int i = 0; i < 27; i++)
        pthread_join(threads[i], NULL);

    // بررسی نهایی
    int is_valid = 1;
    for (int i = 0; i < 27; i++) {
        if (valid[i] == 0) {
            is_valid = 0;
            break;
        }
    }

    if (is_valid)
        printf("Sudoku is valid!\n");
    else
        printf("Sudoku is NOT valid!\n");

    return 0;
}
