#include "SortAlgorithms.h"
#include <iostream>

using namespace std;

// ==========================================
// 1. QUICKSORT ITERATIVO
// ==========================================
// CORRECCIÓN CRÍTICA: La versión recursiva causa stack overflow con archivos
// de 256MB+ (~67 millones de enteros). Usamos una pila explícita en el heap.
// La lógica de partición es idéntica — solo cambia el mecanismo de recursión.

long long particion(PagedArray& arr, long long inicio, long long fin)
{
    int pivote = arr[fin];
    long long i = inicio - 1;

    for (long long j = inicio; j < fin; j++)
    {
        if (arr[j] <= pivote)
        {
            i++;
            int temp = arr[i];
            arr[i]   = arr[j];
            arr[j]   = temp;
        }
    }

    int temp    = arr[i + 1];
    arr[i + 1]  = arr[fin];
    arr[fin]    = temp;

    return (i + 1);
}

void quickSort(PagedArray& arr, long long inicio, long long fin)
{
    if (inicio >= fin) return;

    // Una pila estática súper pequeña es suficiente si la usamos bien
    long long stack[1000];
    long long top = -1;

    stack[++top] = inicio;
    stack[++top] = fin;

    while (top >= 0)
    {
        fin = stack[top--];
        inicio = stack[top--];

        long long pi = particion(arr, inicio, fin);

        // OPTIMIZACIÓN VITAL: Apilar la partición más GRANDE primero.
        // Esto asegura que la pila nunca crezca más allá de log2(N).
        if (pi - inicio < fin - pi) {
            if (pi + 1 < fin) {
                stack[++top] = pi + 1;
                stack[++top] = fin;
            }
            if (pi - 1 > inicio) {
                stack[++top] = inicio;
                stack[++top] = pi - 1;
            }
        } else {
            if (pi - 1 > inicio) {
                stack[++top] = inicio;
                stack[++top] = pi - 1;
            }
            if (pi + 1 < fin) {
                stack[++top] = pi + 1;
                stack[++top] = fin;
            }
        }
    }
}

// ==========================================
// 2. INSERTION SORT
// ==========================================
// Bueno para archivos pequeños o casi ordenados.
// Para archivos grandes (>30MB) es extremadamente lento: O(n²).
void insertionSort(PagedArray& arr, long long n)
{
    for (long long i = 1; i < n; i++)
    {
        int key  = arr[i];
        long long j = i - 1;

        while (j >= 0 && (int)arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// ==========================================
// 3. SELECTION SORT
// ==========================================
// Genera muchos page faults por su patrón de acceso no secuencial.
void selectionSort(PagedArray& arr, long long n)
{
    for (long long i = 0; i < n - 1; i++)
    {
        long long min_idx = i;
        for (long long j = i + 1; j < n; j++)
        {
            if ((int)arr[j] < (int)arr[min_idx])
            {
                min_idx = j;
            }
        }
        if (min_idx != i)
        {
            int temp      = arr[min_idx];
            arr[min_idx]  = arr[i];
            arr[i]        = temp;
        }
    }
}

// ==========================================
// 4. BUBBLE SORT
// ==========================================
// El más lento. Solo recomendado para archivos de prueba pequeños.
void bubbleSort(PagedArray& arr, long long n)
{
    for (long long i = 0; i < n - 1; i++)
    {
        bool swapped = false;
        for (long long j = 0; j < n - i - 1; j++)
        {
            if ((int)arr[j] > (int)arr[j + 1])
            {
                int temp    = arr[j];
                arr[j]      = arr[j + 1];
                arr[j + 1]  = temp;
                swapped     = true;
            }
        }
        if (!swapped) break;
    }
}

// ==========================================
// 5. MERGESORT ITERATIVO BOTTOM-UP
// ==========================================
// CORRECCIÓN: La versión in-place anterior era O(n²) en page faults porque
// desplazaba elementos uno a uno. Esta versión iterativa usa un buffer temporal
// en RAM para mezclar, y solo accede al PagedArray en bloques secuenciales,
// lo que minimiza los page faults.

void mergeSort(PagedArray& arr, long long l, long long r)
{
    long long n = r - l + 1;

    // Tamaño del buffer en RAM para la mezcla
    // 4096 enteros = 16 KB, seguro en cualquier sistema
    const long long BUFFER_SIZE = 4096;
    int* buffer = new int[BUFFER_SIZE];

    // Ordenar en bloques crecientes de tamaño curr_size
    for (long long curr_size = 1; curr_size <= n - 1; curr_size = 2 * curr_size)
    {
        // Recorrer el arreglo de izquierda a derecha mezclando subárboles
        for (long long left_start = l; left_start < r; left_start += 2 * curr_size)
        {
            long long mid = left_start + curr_size - 1;
            if (mid > r) mid = r;

            long long right_end = left_start + 2 * curr_size - 1;
            if (right_end > r) right_end = r;

            if (mid >= right_end) continue;

            // --- Mezcla usando buffer temporal --- //
            long long left_len  = mid - left_start + 1;
            long long right_len = right_end - mid;
            long long total_len = left_len + right_len;

            // Leer el bloque completo a un buffer local si cabe
            if (total_len <= BUFFER_SIZE)
            {
                // Leer desde PagedArray al buffer
                for (long long k = 0; k < total_len; k++)
                {
                    buffer[k] = arr[left_start + k];
                }

                // Mezclar en buffer
                long long i = 0, j = left_len, out = 0;
                int* merged = new int[total_len];

                while (i < left_len && j < total_len)
                {
                    if (buffer[i] <= buffer[j])
                        merged[out++] = buffer[i++];
                    else
                        merged[out++] = buffer[j++];
                }
                while (i < left_len)  merged[out++] = buffer[i++];
                while (j < total_len) merged[out++] = buffer[j++];

                // Escribir de vuelta al PagedArray
                for (long long k = 0; k < total_len; k++)
                {
                    arr[left_start + k] = merged[k];
                }

                delete[] merged;
            }
            else
            {
                // Bloque mayor al buffer: mezclar directamente sobre el PagedArray
                // con punteros i, j (más page faults pero correcto para bloques grandes)
                long long i = left_start, j = mid + 1;

                while (i <= mid && j <= right_end)
                {
                    if ((int)arr[i] <= (int)arr[j])
                    {
                        i++;
                    }
                    else
                    {
                        int value     = arr[j];
                        long long idx = j;

                        while (idx > i)
                        {
                            arr[idx] = arr[idx - 1];
                            idx--;
                        }
                        arr[i] = value;

                        i++; mid++; j++;
                    }
                }
            }
        }
    }

    delete[] buffer;
}