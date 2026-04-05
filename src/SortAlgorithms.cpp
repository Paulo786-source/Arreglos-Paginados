#include "SortAlgorithms.h"
#include <iostream>

using namespace std;

// ==========================================
// 1. QUICKSORT (El más rápido)
// ==========================================
long long particion(PagedArray& arr, long long inicio, long long fin) {
    int pivote = arr[fin];
    long long i = inicio - 1;

    for (long long j = inicio; j < fin; j++) {
        if (arr[j] <= pivote) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    int temp = arr[i + 1];
    arr[i + 1] = arr[fin];
    arr[fin] = temp;
    return (i + 1);
}

void quickSort(PagedArray& arr, long long inicio, long long fin) {
    if (inicio < fin) {
        long long pi = particion(arr, inicio, fin);
        quickSort(arr, inicio, pi - 1);
        quickSort(arr, pi + 1, fin);
    }
}

// ==========================================
// 2. INSERTION SORT (Bueno para casi ordenados)
// ==========================================
void insertionSort(PagedArray& arr, long long n) {
    for (long long i = 1; i < n; i++) {
        int key = arr[i];
        long long j = i - 1;

        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

// ==========================================
// 3. SELECTION SORT (Muchos Page Faults)
// ==========================================
void selectionSort(PagedArray& arr, long long n) {
    for (long long i = 0; i < n - 1; i++) {
        long long min_idx = i;
        for (long long j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            int temp = arr[min_idx];
            arr[min_idx] = arr[i];
            arr[i] = temp;
        }
    }
}

// ==========================================
// 4. BUBBLE SORT (El más lento)
// ==========================================
void bubbleSort(PagedArray& arr, long long n) {
    for (long long i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (long long j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// ==========================================
// 5. IN-PLACE MERGESORT (A prueba de RAM limitada)
// ==========================================
void inPlaceMerge(PagedArray& arr, long long start, long long mid, long long end) {
    long long start2 = mid + 1;

    // Si el final de la mitad izquierda es menor al inicio de la derecha, ya está ordenado
    if (arr[mid] <= arr[start2]) return;

    while (start <= mid && start2 <= end) {
        if (arr[start] <= arr[start2]) {
            start++;
        } else {
            int value = arr[start2];
            long long index = start2;

            // Desplazar todos los elementos un espacio a la derecha
            while (index != start) {
                arr[index] = arr[index - 1];
                index--;
            }
            arr[start] = value;

            // Actualizar todos los punteros
            start++; mid++; start2++;
        }
    }
}

void mergeSort(PagedArray& arr, long long l, long long r) {
    if (l < r) {
        long long m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        inPlaceMerge(arr, l, m, r);
    }
}