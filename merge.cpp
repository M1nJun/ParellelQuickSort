#include <iostream>
#include <ctime>
#include <stdlib.h>
// Include omp.h if you need to use any of the omp_ functions.
#include <omp.h>

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
    if(use != c) {
        int pivot = A[use];
        A[use] = A[r];
        A[r] = pivot;
    }
    int x = A[r];
    int i = p-1;
    for(int j = p;j < r;j++)
    {
        if(A[j] <= x) 
        {
            i++;
            int temp = A[i];
            A[i] = A[j];
            A[j] = temp;
        }
    }
    int temp = A[i+1];
    A[i+1] = A[r];
    A[r] = temp;
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

// Conduct a binary search for x in T[p..r]
// Return the index where x is found,
// or return the index of the first number
// in the range that is > x.
int binary_search(int T[],int p,int r,int x)
{
    int a = p;
    int b = r;
    int c = (a+b)/2;
    while(a < b) {
        if(T[c] == x)
            return c;
        if(T[c] < x)
            a = c + 1;
        else
            b = c;
        c = (a+b)/2;
    }
    if(T[c] > x)
        return c;
    else
        return c+1;
}

void merge(int T[],int p1,int r1,int p2,int r2,int A[],int q)
{
    int a = p1;
    int b = p2;
    int c = q;
    while(a <= r1 && b <= r2) {
        if(T[a] < T[b])
            A[c++] = T[a++];
        else
            A[c++] = T[b++];
    }
    while(a <= r1) {
        A[c++] = T[a++];
    }
    while(b <= r2) {
        A[c++] = T[b++];
    }
}

void parallel_merge(int A[],int p,int q,int r)
{
    int n = r - p + 1;
    int* T = new int[n];
    int i;
#pragma omp parallel for shared(A,p,n,T) private(i)
    for(i = 0;i < n;i++)
        T[i] = A[i+p];
    
    int p1 = 0;
    int r1 = q-p;
    int q1 = (p1+r1)/2;
    int p2 = q-p+1;
    int r2 = r-p;
    int q2 = binary_search(T,p2,r2,T[q1]);
    int p3 = p;
    int q3 = p3+(q1-p1)+(q2-p2);
    A[q3] = T[q1];
#pragma omp parallel sections shared(A,p1,r1,q1,p2,r2,q2,p3,q3)
    {
#pragma omp section
    merge(T,p1,q1-1,p2,q2-1,A,p3);
#pragma omp section
    merge(T,q1+1,r1,q2,r2,A,q3+1);
    }
    
    delete[] T;
}

void parallel_merge_sort(int A[],int p,int r,int level)
{
    if(level > 4) // Revert to sequential after 4 levels
        quicksort(A,p,r); // This limits the thread count
    else {
        int q = (p+r)/2;
#pragma omp parallel sections shared(A,p,r,q)
        {
#pragma omp section
        parallel_merge_sort(A, p, q,level+1);
#pragma omp section
        parallel_merge_sort(A, q+1, r,level+1);
        }
        parallel_merge(A,p,q,r);
        }
   }

int main (int argc, const char * argv[])
{
#define SIZE 60000000
    
    int* A = new int[SIZE];
    for(int i = 0;i < SIZE;i++)
        A[i] = rand();
    time_t now = time(NULL);
    clock_t start = clock();
    quicksort(A,0,SIZE-1);
    time_t later = time(NULL);
    clock_t end = clock();
    std::cout << "Quicksort took " << (later-now) << " seconds." << std::endl;
    std::cout << "CPU time was " << (end-start)/CLOCKS_PER_SEC << " seconds." << std::endl;
    
    // By default, OpenMP does not allow nested parallelism. 
    // We have to call this function to enable it.
    omp_set_nested(1);
    // Once we have enabled nested parallelism, we have to take 
    // care to make sure that we don't try to launch too many
    // execution threads. The level parameter in the
    // parallel merge sort function helps ensure that.
    
    std::cout << "Running in parallel with " << omp_get_max_threads() << " threads available." << std::endl;
    
    for(int i = 0;i < SIZE;i++)
        A[i] = rand();
    now = time(NULL);
    start = clock();
    parallel_merge_sort(A, 0, SIZE-1,0);
    later = time(NULL);
    end = clock();
    std::cout << "Parallel merge sort took " << (later-now) << " seconds." << std::endl;
    std::cout << "CPU time was " << (end-start)/CLOCKS_PER_SEC << " seconds." << std::endl;
    
    delete[] A;
    return 0;
}