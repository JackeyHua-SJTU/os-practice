#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void mergesort(int*, int, int);
void merge(int*, int, int);

struct thread_args {
    int* arr;
    int lb;
    int rb;
};

void* thread_function(void* args) {
    struct thread_args* targs = (struct thread_args*) args;
    mergesort(targs->arr, targs->lb, targs->rb);
    pthread_exit(NULL);
}

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
    printf("Multi thread takes : %f ms \n", (double) (end - start) / CLOCKS_PER_SEC * 1000.0);
    // for (int i = 0; i < size; i++) {
    //     printf("%d ", arr[i]);
    // }
    // printf("\n");
    free(arr);
    return 0;
}

void mergesort(int* arr, int lb, int rb) {
    if (lb >= rb) return;
    
    pthread_t thread1, thread2;
    pthread_attr_t attr1, attr2;
    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);
    pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setscope(&attr1, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setscope(&attr2, PTHREAD_SCOPE_SYSTEM);

    int mid = lb + (rb - lb) / 2;
    struct thread_args targs1 = {arr, lb, mid};
    struct thread_args targs2 = {arr, mid + 1, rb};
    int rc1 = pthread_create(&thread1, NULL, thread_function, &targs1);
    if (rc1) {
        printf("Error: return code from pthread_create() is %d\n", rc1);
        exit(-1);
    }
    // mergesort(arr, lb, mid);
    int rc2 = pthread_create(&thread2, NULL, thread_function, &targs2);
    if (rc2) {
        printf("Error: return code from pthread_create() is %d\n", rc2);
        exit(-1);
    }
    // mergesort(arr, mid + 1, rb);
    rc1 = pthread_join(thread1, NULL);
    rc2 = pthread_join(thread2, NULL);
    
    if (rc1 || rc2) {
        printf("Error: return code from pthread_join() is %d %d\n", rc1, rc2);
        exit(-1);
    }

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
