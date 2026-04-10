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
//   sorter -input <INPUT> -output <o> -alg <ALG>
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
    FILE* in_file = fopen(in_path.c_str(), "rb");
    if (in_file == nullptr)
    {
        cout << "Error: No se pudo abrir el archivo de entrada." << endl;
        return false;
    }

    FILE* out_file = fopen(out_path.c_str(), "wb");
    if (out_file == nullptr)
    {
        cout << "Error: No se pudo crear el archivo de salida." << endl;
        fclose(in_file);
        return false;
    }

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
//
// IMPORTANTE: esta función debe llamarse DESPUÉS de que
// el PagedArray haya sido destruido, para garantizar que
// todos los frames modificados fueron guardados en disco.
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

    while (fread(&numero, sizeof(int), 1, binary))
    {
        if (!primero)
            fprintf(text, ", ");
        fprintf(text, "%d", numero);
        primero = false;
    }

    fclose(binary);
    fclose(text);
}

// -------------------------------------------------------
// main — punto de entrada del programa
// -------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc < 11)
    {
        cout << "Error: argumentos insuficientes." << endl;
        cout << "Uso: sorter -input <INPUT> -output <o> -alg <ALG> "
            << "-pageSize <SIZE> -pageCount <COUNT>" << endl;
        cout << "Algoritmos: QS (QuickSort), MS (MergeSort), IS (InsertionSort), "
            << "SS (SelectionSort), BS (BubbleSort)" << endl;
        return 1;
    }

    string input_file = argv[2];
    string output_file = argv[4];
    string alg = argv[6];
    int page_size = stoi(argv[8]);
    int page_count = stoi(argv[10]);

    if (page_size <= 0 || page_count <= 0)
    {
        cout << "Error: pageSize y pageCount deben ser mayores a cero." << endl;
        return 1;
    }

    if (alg == "BS" || alg == "IS" || alg == "SS")
    {
        cout << "Advertencia: " << alg
            << " es O(n^2) y puede tardar mucho en archivos grandes." << endl;
    }

    // --- Paso 1: Clonar el archivo de entrada ---
    cout << "Clonando archivo..." << endl;
    if (!clon_file(input_file, output_file))
    {
        cout << "Error al clonar el archivo." << endl;
        return 1;
    }
    cout << "Archivo clonado exitosamente." << endl;

    // Variables para guardar los resultados fuera del scope de arr
    long long page_hits_final = 0;
    long long page_faults_final = 0;
    long long duracion_ms = 0;

    // --- Paso 2 y 3: Ordenar dentro de un bloque de scope ---
    //
    // CORRECCIÓN CRÍTICA: el PagedArray se crea dentro de un bloque
    // de llaves propio. Al cerrar ese bloque, C++ destruye arr
    // automáticamente ANTES de continuar con el resto del código.
    //
    // El destructor de PagedArray guarda en disco todos los frames
    // que fueron modificados (dirty_bit = true). Si no hiciéramos
    // esto, export_to_text leería el archivo del disco antes de que
    // los datos ordenados fueran escritos, obteniendo el archivo
    // original desordenado — especialmente cuando todo cabe en RAM
    // y no hubo evictions durante el sort.
    {
        PagedArray arr(output_file.c_str(), page_size, page_count);
        long long cantidad_datos = arr.get_total_elements();

        cout << "Total de elementos: " << cantidad_datos << endl;
        cout << "Algoritmo: " << alg << " | pageSize: " << page_size
            << " | pageCount: " << page_count << endl;
        cout << "Iniciando ordenamiento..." << endl;

        auto inicio_tiempo = high_resolution_clock::now();

        if (alg == "QS")
        {
            // QuickSort iterativo con pivote simple — O(n log n) promedio
            quickSort(arr, 0, cantidad_datos - 1);
        }
        else if (alg == "IS")
        {
            // InsertionSort — O(n²), solo para archivos pequeños
            insertionSort(arr, cantidad_datos);
        }
        else if (alg == "SS")
        {
            // SelectionSort — O(n²), genera muchos page faults
            selectionSort(arr, cantidad_datos);
        }
        else if (alg == "BS")
        {
            if (cantidad_datos > 10000)
                cout << "Advertencia: BubbleSort tardará mucho en archivos grandes." << endl;
            // BubbleSort — O(n²), el más lento
            bubbleSort(arr, cantidad_datos);
        }
        else if (alg == "MS")
        {
            // MergeSort iterativo bottom-up — O(n log n) garantizado
            mergeSort(arr, 0, cantidad_datos - 1);
        }
        else
        {
            cout << "Error: Algoritmo '" << alg << "' no reconocido." << endl;
            return 1;
        }

        auto fin_tiempo = high_resolution_clock::now();
        duracion_ms = duration_cast<milliseconds>(fin_tiempo - inicio_tiempo).count();

        // Guardar contadores antes de que arr se destruya
        page_hits_final = arr.get_page_hits();
        page_faults_final = arr.get_page_faults();

    } // <-- arr se destruye aquí: el destructor guarda todos los
      //     frames modificados al disco antes de continuar

    // --- Paso 4: Imprimir resumen ---
    cout << "=========================================" << endl;
    cout << "          RESUMEN DE EJECUCION           " << endl;
    cout << "=========================================" << endl;
    cout << "Algoritmo: " << alg << endl;
    cout << "Tiempo durado: " << duracion_ms << " milisegundos." << endl;
    cout << "Page Hits:   " << page_hits_final << endl;
    cout << "Page Faults: " << page_faults_final << endl;
    cout << "=========================================" << endl;

    // --- Paso 5: Exportar versión legible ---
    // Ahora el archivo en disco tiene los datos correctamente
    // ordenados porque arr ya fue destruido
    string output_txt = output_file + ".txt";
    cout << "Exportando versión legible a: " << output_txt << endl;
    export_to_text(output_file, output_txt);
    cout << "Listo." << endl;

    return 0;
}