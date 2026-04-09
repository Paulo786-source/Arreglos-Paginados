#include "SortAlgorithms.h"
#include <iostream>

using namespace std;

// ==========================================
// 1. QUICKSORT ITERATIVO
// ==========================================

// Mediana de tres: compara arr[inicio], arr[mid], arr[fin]
// y devuelve el valor del medio entre los tres.
// Además reordena esos tres elementos en su lugar,
// lo que mejora el caso de arrays casi ordenados.
// Esto garantiza particiones mucho más balanceadas
// que siempre tomar el último elemento como pivote.
static int mediana_de_tres(PagedArray& arr, long long inicio, long long fin)
{
    long long mid = inicio + (fin - inicio) / 2;

    int val_ini = arr.get(inicio);
    int val_mid = arr.get(mid);
    int val_fin = arr.get(fin);

    // Ordenar los tres elementos entre sí
    if (val_ini > val_mid)
    {
        arr.set(inicio, val_mid);
        arr.set(mid, val_ini);
        int tmp = val_ini; val_ini = val_mid; val_mid = tmp;
    }
    if (val_ini > val_fin)
    {
        arr.set(inicio, val_fin);
        arr.set(fin, val_ini);
        int tmp = val_ini; val_ini = val_fin; val_fin = tmp;
    }
    if (val_mid > val_fin)
    {
        arr.set(mid, val_fin);
        arr.set(fin, val_mid);
        int tmp = val_mid; val_mid = val_fin; val_fin = tmp;
    }

    // Ahora arr[inicio] <= arr[mid] <= arr[fin]
    // El pivote es arr[mid] — lo movemos a arr[fin-1] para que
    // particion solo trabaje sobre [inicio+1 .. fin-2]
    arr.set(mid, val_fin);   // fin ya tiene val_fin (el mayor)
    arr.set(fin, val_mid);   // pivote al final

    return val_mid; // retornar el valor del pivote
}

static long long particion(PagedArray& arr, long long inicio, long long fin)
{
    // Usar mediana de tres para elegir un pivote balanceado
    int pivote = mediana_de_tres(arr, inicio, fin);

    long long i = inicio - 1;

    for (long long j = inicio; j < fin; j++)
    {
        int val_j = arr.get(j);

        if (val_j <= pivote)
        {
            i++;
            int val_i = arr.get(i);
            arr.set(i, val_j);
            arr.set(j, val_i);
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

        if (fin - inicio < 2)
        {
            // Subarreglo de 2 elementos: ordenar directamente sin particionar
            if (arr.get(inicio) > arr.get(fin))
            {
                int tmp = arr.get(inicio);
                arr.set(inicio, arr.get(fin));
                arr.set(fin, tmp);
            }
            continue;
        }

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
        int min_val = arr.get(i);

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
            int val_j = arr.get(j);
            int val_j1 = arr.get(j + 1);

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

            for (long long k = 0; k < total_len; k++)
                arr.set(left_start + k, merged[k]);

            delete[] buffer;
            delete[] merged;
        }
    }
}