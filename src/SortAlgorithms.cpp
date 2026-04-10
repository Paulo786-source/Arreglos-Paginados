#include "SortAlgorithms.h"
#include <iostream>

using namespace std;

// ============================================================
// 1. QUICKSORT ITERATIVO
// ============================================================

// -------------------------------------------------------
// particion — reorganiza el subarreglo alrededor del pivote
//
// Toma el último elemento como pivote y reorganiza todos
// los demás de forma que los menores o iguales al pivote
// queden a su izquierda y los mayores a su derecha.
// Al terminar, el pivote queda en su posición definitiva
// y nunca vuelve a moverse.
//
// Cada valor se cachea en una variable local antes de
// usarlo para evitar llamadas dobles a get().
// -------------------------------------------------------
static long long particion(PagedArray& arr, long long inicio, long long fin)
{
    // El pivote es siempre el último elemento del subarreglo
    int pivote = arr.get(fin);

    long long i = inicio - 1;  // Índice del último elemento menor al pivote

    for (long long j = inicio; j < fin; j++)
    {
        // Cachear arr.get(j) para no llamarlo dos veces
        int val_j = arr.get(j);

        if (val_j <= pivote)
        {
            i++;
            // Intercambiar arr[i] y arr[j]
            int val_i = arr.get(i);
            arr.set(i, val_j);
            arr.set(j, val_i);
        }
    }

    // Colocar el pivote en su posición final (i+1)
    int val_i1 = arr.get(i + 1);
    arr.set(i + 1, arr.get(fin));
    arr.set(fin, val_i1);

    return i + 1;  // Índice donde quedó el pivote
}

// -------------------------------------------------------
// quickSort — versión iterativa con pila explícita
//
// La versión recursiva clásica puede causar stack overflow
// con archivos grandes. Esta versión usa una pila en el
// heap para simular la recursión sin ese riesgo.
//
// La optimización de apilar siempre la partición más grande
// primero garantiza que la pila nunca supere O(log n)
// niveles — con 2 GB son máximo 29 entradas.
// -------------------------------------------------------
void quickSort(PagedArray& arr, long long inicio, long long fin)
{
    if (inicio >= fin) return;

    // Pila estática de 128 posiciones — suficiente para log2(2^32)
    long long stack[128];
    long long top = -1;

    // Apilar el problema inicial
    stack[++top] = inicio;
    stack[++top] = fin;

    while (top >= 0)
    {
        // Desapilar el subarreglo a ordenar
        fin = stack[top--];
        inicio = stack[top--];

        if (inicio >= fin) continue;

        // Particionar y obtener la posición final del pivote
        long long pi = particion(arr, inicio, fin);

        // Apilar la partición más grande primero para mantener
        // la pila en O(log n) — optimización de Sedgewick
        if (pi - inicio < fin - pi)
        {
            // Partición derecha más grande → apilarla primero
            if (pi + 1 < fin) { stack[++top] = pi + 1; stack[++top] = fin; }
            if (inicio < pi - 1) { stack[++top] = inicio; stack[++top] = pi - 1; }
        }
        else
        {
            // Partición izquierda más grande → apilarla primero
            if (inicio < pi - 1) { stack[++top] = inicio; stack[++top] = pi - 1; }
            if (pi + 1 < fin) { stack[++top] = pi + 1; stack[++top] = fin; }
        }
    }
}

// ============================================================
// 2. INSERTION SORT
//
// Para cada elemento, lo inserta en su posición correcta
// dentro de la parte ya ordenada del arreglo.
// O(n²) en el caso general — solo para archivos pequeños.
// ============================================================
void insertionSort(PagedArray& arr, long long n)
{
    for (long long i = 1; i < n; i++)
    {
        int key = arr.get(i);
        long long j = i - 1;

        // Desplazar hacia la derecha los elementos mayores que key
        while (j >= 0 && arr.get(j) > key)
        {
            arr.set(j + 1, arr.get(j));
            j--;
        }

        // Insertar key en su posición correcta
        arr.set(j + 1, key);
    }
}

// ============================================================
// 3. SELECTION SORT
//
// En cada pasada encuentra el mínimo del subarreglo no
// ordenado y lo coloca al inicio.
// O(n²) siempre — genera muchos page faults por su patrón
// de acceso no secuencial.
// ============================================================
void selectionSort(PagedArray& arr, long long n)
{
    for (long long i = 0; i < n - 1; i++)
    {
        long long min_idx = i;
        int min_val = arr.get(i);

        // Buscar el mínimo en el subarreglo [i+1 .. n-1]
        for (long long j = i + 1; j < n; j++)
        {
            int val_j = arr.get(j);
            if (val_j < min_val)
            {
                min_val = val_j;
                min_idx = j;
            }
        }

        // Intercambiar el mínimo con el elemento en posición i
        if (min_idx != i)
        {
            int temp = arr.get(min_idx);
            arr.set(min_idx, arr.get(i));
            arr.set(i, temp);
        }
    }
}

// ============================================================
// 4. BUBBLE SORT
//
// Compara pares adyacentes e intercambia si están en orden
// incorrecto. La optimización de swapped permite terminar
// anticipadamente si el arreglo ya está ordenado.
// O(n²) — solo para archivos muy pequeños.
// ============================================================
void bubbleSort(PagedArray& arr, long long n)
{
    for (long long i = 0; i < n - 1; i++)
    {
        bool swapped = false;

        for (long long j = 0; j < n - i - 1; j++)
        {
            // Cachear ambos valores para evitar dobles llamadas a get()
            int val_j = arr.get(j);
            int val_j1 = arr.get(j + 1);

            if (val_j > val_j1)
            {
                arr.set(j, val_j1);
                arr.set(j + 1, val_j);
                swapped = true;
            }
        }

        // Si no hubo intercambios, el arreglo ya está ordenado
        if (!swapped) break;
    }
}

// ============================================================
// 5. MERGESORT ITERATIVO BOTTOM-UP
//
// Construye el resultado de abajo hacia arriba: primero
// ordena pares, luego grupos de 4, de 8, de 16, etc.
// Acceso secuencial por bloques → menos page faults que QS.
//
// Los buffers se reservan UNA sola vez antes del loop
// principal para evitar millones de asignaciones dinámicas.
// ============================================================
void mergeSort(PagedArray& arr, long long l, long long r)
{
    long long n = r - l + 1;

    // Reservar buffers una sola vez con el tamaño máximo necesario
    int* buffer = new int[n];
    int* merged = new int[n];

    // Ordenar en bloques de tamaño creciente: 1, 2, 4, 8, 16...
    for (long long curr_size = 1; curr_size <= n - 1; curr_size *= 2)
    {
        for (long long left_start = l; left_start < r; left_start += 2 * curr_size)
        {
            // Calcular límites del bloque izquierdo y derecho
            long long mid = left_start + curr_size - 1;
            if (mid > r) mid = r;

            long long right_end = left_start + 2 * curr_size - 1;
            if (right_end > r) right_end = r;

            if (mid >= right_end) continue;

            long long total_len = right_end - left_start + 1;
            long long left_len = mid - left_start + 1;

            // Leer bloque completo al buffer — acceso secuencial
            for (long long k = 0; k < total_len; k++)
                buffer[k] = arr.get(left_start + k);

            // Mezclar las dos mitades en merged
            long long i = 0, j = left_len, out = 0;

            while (i < left_len && j < total_len)
            {
                if (buffer[i] <= buffer[j]) merged[out++] = buffer[i++];
                else                        merged[out++] = buffer[j++];
            }
            while (i < left_len)  merged[out++] = buffer[i++];
            while (j < total_len) merged[out++] = buffer[j++];

            // Escribir resultado ordenado de vuelta al PagedArray
            for (long long k = 0; k < total_len; k++)
                arr.set(left_start + k, merged[k]);
        }
    }

    // Liberar buffers al terminar — una sola vez
    delete[] buffer;
    delete[] merged;
}