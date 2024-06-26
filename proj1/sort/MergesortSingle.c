#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void mergesort(int*, int, int);
void merge(int*, int, int);

int main() {
    int size; 
    scanf("%d", &size);

    int *arr = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        scanf("%d", &arr[i]);
    }

    clock_t start = clock();
    mergesort(arr, 0, size - 1);
    clock_t end = clock();
    printf("Single thread merge sort takes : %f ms\n", (double) (end - start) / CLOCKS_PER_SEC * 1000.0);
    free(arr);
    // for (int i = 0; i < size; i++) {
    //     printf("%d ", arr[i]);
    // }
    // printf("\n");
    return 0;
}

void mergesort(int* arr, int lb, int rb) {
    if (lb >= rb) return;
    int mid = lb + (rb - lb) / 2;
    mergesort(arr, lb, mid);
    mergesort(arr, mid + 1, rb);
    merge(arr, lb, rb);
}

void merge(int* arr, int l, int r) {
    /* double pointer */
    int m = l + (r - l) / 2;
    int left = l, right = m + 1;
    int *temp = malloc((r - l + 1) * sizeof(int));
    while (left <= m || right <= r) {
        if (left > m) {
            temp[left + right - l - m - 1] = arr[right];
            ++right;
        } else if (right > r) {
            temp[left + right - l - m - 1] = arr[left];
            ++left;
        } else if (arr[left] < arr[right]) {
            temp[left + right - l - m - 1] = arr[left];
            ++left;
        } else {
            temp[left + right - l - m - 1] = arr[right];
            ++right;
        }
    }
    for (int i = l; i <= r; i++) {
        arr[i] = temp[i - l];
    }
    free(temp);
}
