#include <iostream>
#include <ctime>
#include <stdlib.h>
#include <omp.h>

class pBlock {
public:
    int start;
    int mid;
    int end;
};

int parallel_partition(int A[], int p, int r, int num_threads)
{
    // Median of three
    int a = p;
    int b = (p+r)/2;
    int c = r;
    int use = c;
    if(A[a] > A[b] && A[a] < A[c])
        use = a;
    else if(A[a] < A[b] && A[a] > A[c])
        use = a;
    else if(A[b] < A[a] && A[b] > A[c])
        use = b;
    else if(A[b] > A[a] && A[b] < A[c])
        use = b;
    // Once pivot decided, swap with last element
    if(use != c) {
        int pivot = A[use];
        A[use] = A[r];
        A[r] = pivot;
    }
    
    int subregion = (r-p) / num_threads;

    pBlock blocks[num_threads];

    for(int i=0; i < num_threads; i++){
        blocks[i].start = p + i * subregion;
        blocks[i].end = blocks[i].start + subregion;
    }

    #pragma omp parallel for
    for(int i=0; i < num_threads; i++){
        part(int A[], blocks[i], pivot);
    }

    for(int i=0; i < num_threads - 1; i++){
        blocks[i] = merge(blocks[i], blocks[i+1]);
    }

    return pivot
}

void part(int A[], qBlock block, int pivot){
    int x = pivot;
    // i keeps track of the border between elements less than or equal to the pivot and elements greater than the pivot
    int i = block.start-1;
    for(int j = block.start;j < block.end;j++)
    {
        // for each element A[j], if it's less than or equal to the pivot x, it swaps A[j] with A[i+1]
        if(A[j] <= x) 
        {
            i++;
            int temp = A[i];
            A[i] = A[j];
            A[j] = temp;
        }
    }
    block.mid = A[i+1];
}

qBlock merge(int A[], qBlock b1, qBlock b2){
    // swap: from b1.mid to b1.end    and     b2.start to b2.mid
    #pragma omp parallel for
    for(int i = b1.mid; i < b1.end + 1; i++){
        int temp = A[b1.mid + i];
        A[b1.mid + i] = A[b2.start + i];
        A[b2.start + i] = temp;
    }
    
    qBlock block;
    block.start = b1.start;
    block.mid = b1.end;
    block.end = b2.mid;
    
    return block;
    
}

int partition(int A[],int p,int r)
{
    // Median of three
    int a = p;
    int b = (p+r)/2;
    int c = r;
    int use = c;
    if(A[a] > A[b] && A[a] < A[c])
        use = a;
    else if(A[a] < A[b] && A[a] > A[c])
        use = a;
    else if(A[b] < A[a] && A[b] > A[c])
        use = b;
    else if(A[b] > A[a] && A[b] < A[c])
        use = b;
    // Once pivot decided, swap with last element
    if(use != c) {
        int pivot = A[use];
        A[use] = A[r];
        A[r] = pivot;
    }
    int x = A[r];
    // i keeps track of the border between elements less than or equal to the pivot and elements greater than the pivot
    int i = p-1;
    for(int j = p;j < r;j++)
    {
        // for each element A[j], if it's less than or equal to the pivot x, it swaps A[j] with A[i+1]
        if(A[j] <= x) 
        {
            i++;
            int temp = A[i];
            A[i] = A[j];
            A[j] = temp;
        }
    }
    // Once all elements have been traversed, pivot is placed in its correct position
    int temp = A[i+1];
    A[i+1] = A[r];
    A[r] = temp;
    // Return pivot index
    return i+1;
}

void insertion(int A[],int p,int q) {
    for(int n = p;n < q;n++) {
        int k = n;
        int x = A[k+1];
        while(k >= p && A[k] > x) {
            A[k+1] = A[k];
            k--;
        }
        A[k+1] = x;
    }
}

void quicksort(int A[],int p,int q) {
    if(q - p < 100) {
        // Use insertion sort for small chunks
        insertion(A,p,q);
    } else {
        int r = partition(A,p,q);
        quicksort(A,p,r-1);
        quicksort(A,r+1,q);
    }
}

void naive_parallel_quicksort(int A[],int p,int q,int n_threads) {
    if(n_threads >= 2){
        int r = parellel_partition(A,p,q, n_threads);
        int n = n_threads/2;
#pragma omp task shared(A)
        naive_parallel_quicksort(A,p,r-1,n);
#pragma omp task shared(A)
        naive_parallel_quicksort(A,r+1,q,n);
    } else if(q - p < 100) {
        // Use insertion sort for small chunks
        insertion(A,p,q);
    }  else {
        int r = partition(A,p,q);
        quicksort(A,p,r-1);
        quicksort(A,r+1,q);
    } 
}

int main (int argc, const char * argv[])
{
#define SIZE 500000000
    
    std::cout << "Preparing unsorted array." << std::endl;
    int* B = new int[SIZE];
    for(int i = 0;i < SIZE;i++)
        B[i] = rand() % SIZE;
    int* A = new int[SIZE];
    for(int i = 0;i < SIZE;i++)
        A[i] = B[i];
    
    std::cout << "Starting sequential sort." << std::endl;
    time_t now = time(NULL);
    clock_t start = clock();
    quicksort(A,0,SIZE-1);
    time_t later = time(NULL);
    clock_t end = clock();
    std::cout << "Quicksort took " << (later-now) << " seconds." << std::endl;
    std::cout << "CPU time was " << (end-start)/CLOCKS_PER_SEC << " seconds." << std::endl;
    
    int n = omp_get_max_threads();
    int k = 2;
    while(2*k <= n)
        k *= 2;
    
    for(int i = 0;i < SIZE;i++)
        A[i] = B[i];

    std::cout << "Naive parallel using " << k << " threads out of a maximum of " << n << "." << std::endl;
    
    now = time(NULL);
    start = clock();
#pragma omp parallel
{
#pragma omp single
    naive_parallel_quicksort(A,0,SIZE-1,k);
#pragma omp taskwait
}
    later = time(NULL);
    end = clock();
    std::cout << "Naive parallel quicksort took " << (later-now) << " seconds." << std::endl;
    std::cout << "CPU time was " << (end-start)/CLOCKS_PER_SEC << " seconds." << std::endl;

    delete[] A;
    return 0;
}