#ifndef SORT_ALGORITHMS_H
#define SORT_ALGORITHMS_H

#include "PagedArray.h"

// ============================================================
// SortAlgorithms — Algoritmos de ordenamiento sobre PagedArray
//
// Todos los algoritmos reciben un PagedArray y operan sobre
// él usando get() y set() explícitos. El PagedArray se encarga
// de toda la lógica de paginación de forma transparente —
// los algoritmos no saben ni les importa que los datos
// están parcialmente en disco.
//
// Complejidades:
//   QS (QuickSort)     → O(n log n) promedio, O(n^2) peor caso
//   MS (MergeSort)     → O(n log n) garantizado
//   IS (InsertionSort) → O(n^2) — solo viable para archivos pequeños
//   SS (SelectionSort) → O(n^2) — solo viable para archivos pequeños
//   BS (BubbleSort)    → O(n^2) — solo viable para archivos pequeños
// ============================================================

void quickSort(PagedArray& arr, long long inicio, long long fin);
void insertionSort(PagedArray& arr, long long n);
void selectionSort(PagedArray& arr, long long n);
void bubbleSort(PagedArray& arr, long long n);
void mergeSort(PagedArray& arr, long long left, long long right);

#endif