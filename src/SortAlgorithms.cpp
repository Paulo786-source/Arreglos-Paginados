#include "SortAlgorithms.h"
#include <iostream>

using namespace std;

// ==========================================
// 1. QUICKSORT ITERATIVO
// ==========================================

static long long particion(PagedArray& arr, long long inicio, long long fin)
{
    int pivote = arr.get(fin);
    long long i = inicio - 1;

    for (long long j = inicio; j < fin; j++)
    {
        // CORRECCIÓN: cachear arr.get(j) en una variable local.
        // Antes se llamaba dos veces: una para comparar y otra dentro del swap.
        // Con n log n iteraciones eso duplicaba millones de accesos innecesarios.
        int val_j = arr.get(j);

        if (val_j <= pivote)
        {
            i++;
            int val_i = arr.get(i);  // cachear arr.get(i) también
            arr.set(i, val_j);       // usar valor cacheado, no releer
            arr.set(j, val_i);       // usar valor cacheado, no releer
        }
    }

    // Colocar pivote en su posición final
    int val_i1 = arr.get(i + 1);
    arr.set(i + 1, arr.get(fin));
    arr.set(fin, val_i1);

    return i + 1;
}

void quickSort(PagedArray& arr, long long inicio, long long fin)
{
    if (inicio >= fin) return;

    long long stack[128];
    long long top = -1;

    stack[++top] = inicio;
    stack[++top] = fin;

    while (top >= 0)
    {
        fin = stack[top--];
        inicio = stack[top--];

        long long pi = particion(arr, inicio, fin);

        // Apilar la partición más grande primero → pila nunca supera O(log n)
        if (pi - inicio < fin - pi)
        {
            if (pi + 1 < fin) { stack[++top] = pi + 1; stack[++top] = fin; }
            if (inicio < pi - 1) { stack[++top] = inicio; stack[++top] = pi - 1; }
        }
        else
        {
            if (inicio < pi - 1) { stack[++top] = inicio; stack[++top] = pi - 1; }
            if (pi + 1 < fin) { stack[++top] = pi + 1; stack[++top] = fin; }
        }
    }
}

// ==========================================
// 2. INSERTION SORT
// ==========================================
void insertionSort(PagedArray& arr, long long n)
{
    for (long long i = 1; i < n; i++)
    {
        int key = arr.get(i);
        long long j = i - 1;

        while (j >= 0 && arr.get(j) > key)
        {
            arr.set(j + 1, arr.get(j));
            j--;
        }
        arr.set(j + 1, key);
    }
}

// ==========================================
// 3. SELECTION SORT
// ==========================================
void selectionSort(PagedArray& arr, long long n)
{
    for (long long i = 0; i < n - 1; i++)
    {
        long long min_idx = i;
        int min_val = arr.get(i);  // cachear para evitar releer en cada comparación

        for (long long j = i + 1; j < n; j++)
        {
            int val_j = arr.get(j);
            if (val_j < min_val)
            {
                min_val = val_j;
                min_idx = j;
            }
        }

        if (min_idx != i)
        {
            int temp = arr.get(min_idx);
            arr.set(min_idx, arr.get(i));
            arr.set(i, temp);
        }
    }
}

// ==========================================
// 4. BUBBLE SORT
// ==========================================
void bubbleSort(PagedArray& arr, long long n)
{
    for (long long i = 0; i < n - 1; i++)
    {
        bool swapped = false;
        for (long long j = 0; j < n - i - 1; j++)
        {
            int val_j = arr.get(j);      // cachear
            int val_j1 = arr.get(j + 1);  // cachear

            if (val_j > val_j1)
            {
                arr.set(j, val_j1);
                arr.set(j + 1, val_j);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// ==========================================
// 5. MERGESORT ITERATIVO BOTTOM-UP
// ==========================================
void mergeSort(PagedArray& arr, long long l, long long r)
{
    long long n = r - l + 1;

    for (long long curr_size = 1; curr_size <= n - 1; curr_size *= 2)
    {
        for (long long left_start = l; left_start < r; left_start += 2 * curr_size)
        {
            long long mid = left_start + curr_size - 1;
            if (mid > r) mid = r;

            long long right_end = left_start + 2 * curr_size - 1;
            if (right_end > r) right_end = r;

            if (mid >= right_end) continue;

            long long total_len = right_end - left_start + 1;
            int* buffer = new int[total_len];
            int* merged = new int[total_len];

            // Leer bloque completo al buffer — acceso secuencial, mínimos faults
            for (long long k = 0; k < total_len; k++)
                buffer[k] = arr.get(left_start + k);

            long long left_len = mid - left_start + 1;
            long long i = 0, j = left_len, out = 0;

            while (i < left_len && j < total_len)
            {
                if (buffer[i] <= buffer[j]) merged[out++] = buffer[i++];
                else                        merged[out++] = buffer[j++];
            }
            while (i < left_len)  merged[out++] = buffer[i++];
            while (j < total_len) merged[out++] = buffer[j++];

            // Escribir resultado de vuelta
            for (long long k = 0; k < total_len; k++)
                arr.set(left_start + k, merged[k]);

            delete[] buffer;
            delete[] merged;
        }
    }
}