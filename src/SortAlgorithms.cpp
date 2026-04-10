#include "SortAlgorithms.h"
#include <iostream>

using namespace std;

// ============================================================
// 1. QUICKSORT ITERATIVO con mediana de tres
// ============================================================

// -------------------------------------------------------
// mediana_de_tres — elige un pivote balanceado
//
// Compara los valores en arr[inicio], arr[mid] y arr[fin],
// los ordena entre sí, y coloca el valor del medio (la mediana)
// en arr[fin] para usarlo como pivote.
//
// Esto garantiza que el pivote nunca sea el mínimo ni el
// máximo de esos tres elementos, produciendo particiones
// más balanceadas que tomar siempre el último elemento.
// Con datos aleatorios reduce el número de comparaciones
// y elimina los peores casos más comunes de QuickSort.
// -------------------------------------------------------
static int mediana_de_tres(PagedArray& arr, long long inicio, long long fin)
{
    long long mid = inicio + (fin - inicio) / 2;

    // Leer los tres candidatos a pivote
    int val_ini = arr.get(inicio);
    int val_mid = arr.get(mid);
    int val_fin = arr.get(fin);

    // Ordenar los tres valores entre sí con tres comparaciones
    // Al terminar: val_ini <= val_mid <= val_fin
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

    // Colocar la mediana (val_mid) en arr[fin] para que particion
    // la use como pivote. arr[inicio] ya es el menor de los tres
    // y arr[fin] queda con val_mid.
    // CORRECCIÓN: intercambio explícito y correcto entre mid y fin
    arr.set(mid, val_fin);
    arr.set(fin, val_mid);

    // Retornar el valor del pivote (la mediana)
    return val_mid;
}

// -------------------------------------------------------
// particion — reorganiza el subarreglo alrededor del pivote
//
// Después de ejecutarse, el pivote queda en su posición
// definitiva: todos los elementos a su izquierda son
// menores o iguales, y todos a su derecha son mayores.
//
// Cada valor se cachea en una variable local antes de
// usarlo para evitar llamadas dobles a get().
// -------------------------------------------------------
static long long particion(PagedArray& arr, long long inicio, long long fin)
{
    // Elegir pivote con mediana de tres
    int pivote = mediana_de_tres(arr, inicio, fin);

    long long i = inicio - 1;  // Índice del último elemento menor al pivote

    for (long long j = inicio; j < fin; j++)
    {
        int val_j = arr.get(j);  // Cachear para no llamar get() dos veces

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
// con archivos grandes (~500M elementos = ~29 niveles de
// recursión, pero en el peor caso puede ser O(n)).
//
// Esta versión usa una pila en el heap para simular la
// recursión. La optimización de apilar siempre la partición
// más grande primero garantiza que la pila nunca supere
// O(log n) niveles — con 2 GB son máximo 29 entradas.
// -------------------------------------------------------
void quickSort(PagedArray& arr, long long inicio, long long fin)
{
    if (inicio >= fin) return;

    // Pila estática de 128 posiciones — más que suficiente para log2(2^32)
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

        // Caso base: subarreglo de 2 elementos, ordenar directamente
        if (fin - inicio < 2)
        {
            if (arr.get(inicio) > arr.get(fin))
            {
                int tmp = arr.get(inicio);
                arr.set(inicio, arr.get(fin));
                arr.set(fin, tmp);
            }
            continue;
        }

        // Particionar y obtener la posición final del pivote
        long long pi = particion(arr, inicio, fin);

        // Apilar la partición más grande primero para mantener
        // la pila en O(log n) — optimización de Sedgewick
        if (pi - inicio < fin - pi)
        {
            // La partición derecha es más grande → apilarla primero
            if (pi + 1 < fin) { stack[++top] = pi + 1; stack[++top] = fin; }
            if (inicio < pi - 1) { stack[++top] = inicio; stack[++top] = pi - 1; }
        }
        else
        {
            // La partición izquierda es más grande → apilarla primero
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
// Eficiente para arreglos pequeños o casi ordenados.
// O(n²) en el caso general — no recomendado para archivos grandes.
// ============================================================
void insertionSort(PagedArray& arr, long long n)
{
    for (long long i = 1; i < n; i++)
    {
        // Guardar el elemento actual como "llave"
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
// En cada pasada, encuentra el mínimo del subarreglo no
// ordenado y lo coloca al inicio.
// O(n²) siempre — genera muchos page faults por su patrón
// de acceso no secuencial entre posiciones muy distantes.
// ============================================================
void selectionSort(PagedArray& arr, long long n)
{
    for (long long i = 0; i < n - 1; i++)
    {
        long long min_idx = i;
        int min_val = arr.get(i);  // Cachear para evitar releer en cada comparación

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
// Compara pares adyacentes y los intercambia si están
// en orden incorrecto. Repite hasta que no haya intercambios.
// O(n²) — el más lento de todos. Solo viable para archivos
// muy pequeños o casi ordenados (la optimización de swapped
// permite terminar temprano si ya está ordenado).
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
// En lugar de dividir recursivamente (top-down), construye
// el resultado de abajo hacia arriba: primero ordena pares,
// luego grupos de 4, de 8, de 16, etc.
//
// Ventaja sobre QuickSort con paginación: accede al arreglo
// de forma secuencial por bloques, lo que favorece la
// localidad de referencia y reduce los page faults.
//
// CORRECCIÓN de mala práctica: los buffers temporales se
// reservan UNA sola vez antes del loop principal con el
// tamaño máximo posible (n enteros), en lugar de hacer
// new/delete en cada iteración del loop interno.
// Esto elimina millones de asignaciones dinámicas innecesarias.
// ============================================================
void mergeSort(PagedArray& arr, long long l, long long r)
{
    long long n = r - l + 1;

    // Reservar buffers temporales una sola vez con el tamaño máximo
    // CORRECCIÓN: antes se hacía new/delete dentro del loop interno,
    // lo que generaba millones de asignaciones de memoria innecesarias
    int* buffer = new int[n];
    int* merged = new int[n];

    // Ordenar en bloques de tamaño creciente: 1, 2, 4, 8, 16...
    for (long long curr_size = 1; curr_size <= n - 1; curr_size *= 2)
    {
        // Recorrer el arreglo mezclando pares de bloques adyacentes
        for (long long left_start = l; left_start < r; left_start += 2 * curr_size)
        {
            // Calcular límites del bloque izquierdo y derecho
            long long mid = left_start + curr_size - 1;
            if (mid > r) mid = r;

            long long right_end = left_start + 2 * curr_size - 1;
            if (right_end > r) right_end = r;

            // Si no hay bloque derecho, no hay nada que mezclar
            if (mid >= right_end) continue;

            long long total_len = right_end - left_start + 1;
            long long left_len = mid - left_start + 1;

            // Leer el bloque completo al buffer — acceso secuencial,
            // favorece hits en el PagedArray
            for (long long k = 0; k < total_len; k++)
                buffer[k] = arr.get(left_start + k);

            // Mezclar las dos mitades del buffer en merged
            long long i = 0, j = left_len, out = 0;

            while (i < left_len && j < total_len)
            {
                if (buffer[i] <= buffer[j]) merged[out++] = buffer[i++];
                else                        merged[out++] = buffer[j++];
            }
            while (i < left_len)  merged[out++] = buffer[i++];
            while (j < total_len) merged[out++] = buffer[j++];

            // Escribir el resultado ordenado de vuelta al PagedArray
            for (long long k = 0; k < total_len; k++)
                arr.set(left_start + k, merged[k]);
        }
    }

    // Liberar los buffers al terminar — una sola vez
    delete[] buffer;
    delete[] merged;
}