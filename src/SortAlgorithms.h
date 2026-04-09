#ifndef SORT_ALGORITHMS_H
#define SORT_ALGORITHMS_H

#include "PagedArray.h"

void quickSort(PagedArray& arr, long long inicio, long long fin);
void insertionSort(PagedArray& arr, long long n);
void selectionSort(PagedArray& arr, long long n);
void bubbleSort(PagedArray& arr, long long n);
void mergeSort(PagedArray& arr, long long left, long long right);

#endif