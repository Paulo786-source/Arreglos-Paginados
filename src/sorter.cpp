#include <iostream>
#include <string>
#include <cstdio>
#include <chrono>
#include "PagedArray.h"
#include "SortAlgorithms.h"

using namespace std;
using namespace std::chrono;

// ============================================================
// sorter — Ordenador de archivos binarios grandes
//
// Lee un archivo binario de enteros, lo ordena usando el
// algoritmo especificado y escribe el resultado en un nuevo
// archivo binario y en un archivo de texto legible.
//
// Internamente usa PagedArray para manejar archivos que no
// caben completamente en RAM, cargando solo las páginas
// necesarias en cada momento.
//
// Uso:
//   sorter -input <INPUT> -output <OUTPUT> -alg <ALG>
//          -pageSize <SIZE> -pageCount <COUNT>
//
// Algoritmos disponibles:
//   QS → QuickSort     (O(n log n) promedio, recomendado)
//   MS → MergeSort     (O(n log n) garantizado)
//   IS → InsertionSort (O(n²), solo para archivos pequeños)
//   SS → SelectionSort (O(n²), solo para archivos pequeños)
//   BS → BubbleSort    (O(n²), solo para archivos pequeños)
// ============================================================

// -------------------------------------------------------
// clon_file — copia el archivo de entrada al de salida
//
// El ordenamiento se realiza sobre una copia del archivo
// original para no modificarlo. Se copia en bloques de
// 1 MB para no cargar todo en RAM de una vez.
// -------------------------------------------------------
bool clon_file(const string& in_path, const string& out_path)
{
    // Abrir archivo origen en modo lectura binaria
    FILE* in_file = fopen(in_path.c_str(), "rb");
    if (in_file == nullptr)
    {
        cout << "Error: No se pudo abrir el archivo de entrada." << endl;
        return false;
    }

    // Crear archivo destino en modo escritura binaria
    FILE* out_file = fopen(out_path.c_str(), "wb");
    if (out_file == nullptr)
    {
        cout << "Error: No se pudo crear el archivo de salida." << endl;
        fclose(in_file);
        return false;
    }

    // Copiar en bloques de 1 MB para eficiencia
    const int buffer_copy_size = 1024 * 1024;
    char* buffer_copy = new char[buffer_copy_size];
    size_t bytes_leidos;

    while ((bytes_leidos = fread(buffer_copy, 1, buffer_copy_size, in_file)) > 0)
    {
        fwrite(buffer_copy, 1, bytes_leidos, out_file);
    }

    fclose(in_file);
    fclose(out_file);
    delete[] buffer_copy;

    return true;
}

// -------------------------------------------------------
// export_to_text — convierte el binario ordenado a texto
//
// Lee el archivo binario entero por entero y escribe los
// valores separados por comas en un archivo .txt legible.
// Esto permite verificar visualmente que el ordenamiento
// fue correcto.
// -------------------------------------------------------
void export_to_text(const string& binary_path, const string& txt_path)
{
    FILE* binary = fopen(binary_path.c_str(), "rb");
    FILE* text = fopen(txt_path.c_str(), "w");

    if (!binary || !text)
    {
        cout << "Error al generar el archivo legible." << endl;
        if (binary) fclose(binary);
        if (text)   fclose(text);
        return;
    }

    int numero;
    bool primero = true;

    // Leer un entero a la vez del binario y escribirlo al texto
    while (fread(&numero, sizeof(int), 1, binary))
    {
        if (!primero)
            fprintf(text, ", ");  // Separador entre números
        fprintf(text, "%d", numero);
        primero = false;
    }

    fclose(binary);
    fclose(text);
}

// -------------------------------------------------------
// main — punto de entrada del programa
//
// Parsea los argumentos, clona el archivo, construye el
// PagedArray sobre la copia, ejecuta el algoritmo elegido,
// mide el tiempo y reporta el resumen de ejecución.
// -------------------------------------------------------
int main(int argc, char* argv[])
{
    // Validar que se pasaron todos los argumentos requeridos
    if (argc < 11)
    {
        cout << "Error: argumentos insuficientes." << endl;
        cout << "Uso: sorter -input <INPUT> -output <OUTPUT> -alg <ALG> "
            << "-pageSize <SIZE> -pageCount <COUNT>" << endl;
        cout << "Algoritmos: QS (QuickSort), MS (MergeSort), IS (InsertionSort), "
            << "SS (SelectionSort), BS (BubbleSort)" << endl;
        return 1;
    }

    // Leer argumentos por posición
    // El orden esperado es exactamente el indicado en el uso
    string input_file = argv[2];   // Archivo binario de entrada
    string output_file = argv[4];   // Archivo binario de salida (la copia ordenada)
    string alg = argv[6];   // Código del algoritmo a usar
    int page_size = stoi(argv[8]);   // Enteros por página
    int page_count = stoi(argv[10]);  // Páginas simultáneas en RAM

    // Validar parámetros de paginación
    if (page_size <= 0 || page_count <= 0)
    {
        cout << "Error: pageSize y pageCount deben ser mayores a cero." << endl;
        return 1;
    }

    // Advertir sobre algoritmos O(n²) que son inviables para archivos grandes
    if (alg == "BS" || alg == "IS" || alg == "SS")
    {
        cout << "Advertencia: " << alg
            << " es O(n^2) y puede tardar mucho en archivos grandes." << endl;
    }

    // --- Paso 1: Clonar el archivo de entrada ---
    // El ordenamiento trabaja sobre la copia para no alterar el original
    cout << "Clonando archivo..." << endl;
    if (!clon_file(input_file, output_file))
    {
        cout << "Error al clonar el archivo." << endl;
        return 1;
    }
    cout << "Archivo clonado exitosamente." << endl;

    // --- Paso 2: Construir el PagedArray sobre la copia ---
    // PagedArray abre el archivo y calcula cuántos enteros tiene.
    // No carga nada en RAM todavía — lo hace bajo demanda al ordenar.
    PagedArray arr(output_file.c_str(), page_size, page_count);
    long long cantidad_datos = arr.get_total_elements();

    cout << "Total de elementos: " << cantidad_datos << endl;
    cout << "Algoritmo: " << alg << " | pageSize: " << page_size
        << " | pageCount: " << page_count << endl;
    cout << "Iniciando ordenamiento..." << endl;

    // --- Paso 3: Ejecutar el algoritmo y medir el tiempo ---
    auto inicio_tiempo = high_resolution_clock::now();

    if (alg == "QS")
    {
        // QuickSort iterativo con mediana de tres — el más rápido en promedio
        quickSort(arr, 0, cantidad_datos - 1);
    }
    else if (alg == "IS")
    {
        // InsertionSort — bueno para arreglos pequeños o casi ordenados
        insertionSort(arr, cantidad_datos);
    }
    else if (alg == "SS")
    {
        // SelectionSort — genera muchos page faults por acceso no secuencial
        selectionSort(arr, cantidad_datos);
    }
    else if (alg == "BS")
    {
        if (cantidad_datos > 10000)
            cout << "Advertencia: BubbleSort tardará mucho en archivos grandes." << endl;
        // BubbleSort — el más lento, solo para archivos muy pequeños
        bubbleSort(arr, cantidad_datos);
    }
    else if (alg == "MS")
    {
        // MergeSort iterativo bottom-up — acceso secuencial, mínimos page faults
        mergeSort(arr, 0, cantidad_datos - 1);
    }
    else
    {
        cout << "Error: Algoritmo '" << alg << "' no reconocido." << endl;
        return 1;
    }

    auto fin_tiempo = high_resolution_clock::now();
    auto duracion = duration_cast<milliseconds>(fin_tiempo - inicio_tiempo);

    // --- Paso 4: Imprimir resumen de ejecución ---
    // Al destruirse arr aquí, el destructor de PagedArray guarda
    // automáticamente en disco todas las páginas modificadas que
    // quedaron en RAM (dirty_bit = true)
    cout << "=========================================" << endl;
    cout << "          RESUMEN DE EJECUCION           " << endl;
    cout << "=========================================" << endl;
    cout << "Algoritmo: " << alg << endl;
    cout << "Tiempo durado: " << duracion.count() << " milisegundos." << endl;
    cout << "Page Hits:   " << arr.get_page_hits() << endl;
    cout << "Page Faults: " << arr.get_page_faults() << endl;
    cout << "=========================================" << endl;

    // --- Paso 5: Exportar versión legible en texto ---
    // Genera un .txt con los enteros separados por comas
    // para poder verificar visualmente que el resultado es correcto
    string output_txt = output_file + ".txt";
    cout << "Exportando versión legible a: " << output_txt << endl;
    export_to_text(output_file, output_txt);
    cout << "Listo." << endl;

    return 0;
}