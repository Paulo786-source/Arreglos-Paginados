#include <iostream>
#include <string>
#include <cstdio>
#include <chrono>
#include "PagedArray.h"
#include "SortAlgorithms.h"

using namespace std;
using namespace std::chrono;

// Clona el archivo de entrada al archivo de salida
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

    const int buffer_copy_size = 1024 * 1024; // 1MB por bloque
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

// Exporta el archivo binario ordenado a texto legible (separado por comas)
void export_to_text(const string& binary_path, const string& txt_path)
{
    FILE* binary = fopen(binary_path.c_str(), "rb");
    FILE* text   = fopen(txt_path.c_str(), "w");

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
        {
            fprintf(text, ", ");
        }
        fprintf(text, "%d", numero);
        primero = false;
    }

    fclose(binary);
    fclose(text);
}

int main(int argc, char* argv[])
{
    if (argc < 11)
    {
        cout << "Error: argumentos insuficientes." << endl;
        cout << "Uso: sorter -input <INPUT> -output <OUTPUT> -alg <ALG> "
             << "-pageSize <SIZE> -pageCount <COUNT>" << endl;
        cout << "Algoritmos: QS (QuickSort), MS (MergeSort), IS (InsertionSort), "
             << "SS (SelectionSort), BS (BubbleSort)" << endl;
        return 1;
    }

    string input_file  = argv[2];
    string output_file = argv[4];
    string alg         = argv[6];
    int page_size      = stoi(argv[8]);
    int page_count     = stoi(argv[10]);

    // Validaciones básicas
    if (page_size <= 0 || page_count <= 0)
    {
        cout << "Error: pageSize y pageCount deben ser mayores a cero." << endl;
        return 1;
    }

    // Advertencias sobre algoritmos lentos en archivos grandes
    if (alg == "BS" || alg == "IS" || alg == "SS")
    {
        cout << "Advertencia: " << alg << " es O(n²) y puede tardar mucho en archivos grandes." << endl;
    }

    cout << "Clonando archivo..." << endl;

    if (!clon_file(input_file, output_file))
    {
        cout << "Error al clonar el archivo." << endl;
        return 1;
    }

    cout << "Archivo clonado exitosamente." << endl;

    PagedArray arr(output_file.c_str(), page_size, page_count);
    long long cantidad_datos = arr.get_total_elements();

    cout << "Total de elementos: " << cantidad_datos << endl;
    cout << "Algoritmo: " << alg << " | pageSize: " << page_size
         << " | pageCount: " << page_count << endl;
    cout << "Iniciando ordenamiento..." << endl;

    auto inicio_tiempo = high_resolution_clock::now();

    if (alg == "QS")
    {
        quickSort(arr, 0, cantidad_datos - 1);
    }
    else if (alg == "IS")
    {
        insertionSort(arr, cantidad_datos);
    }
    else if (alg == "SS")
    {
        selectionSort(arr, cantidad_datos);
    }
    else if (alg == "BS")
    {
        if (cantidad_datos > 10000)
        {
            cout << "Advertencia: BubbleSort tardará mucho en archivos grandes." << endl;
        }
        bubbleSort(arr, cantidad_datos);
    }
    else if (alg == "MS")
    {
        mergeSort(arr, 0, cantidad_datos - 1);
    }
    else
    {
        cout << "Error: Algoritmo '" << alg << "' no reconocido." << endl;
        return 1;
    }

    auto fin_tiempo = high_resolution_clock::now();
    auto duracion   = duration_cast<milliseconds>(fin_tiempo - inicio_tiempo);

    cout << "=========================================" << endl;
    cout << "          RESUMEN DE EJECUCION           " << endl;
    cout << "=========================================" << endl;
    cout << "Algoritmo: " << alg << endl;
    cout << "Tiempo durado: " << duracion.count() << " milisegundos." << endl;
    cout << "Page Hits:   " << arr.get_page_hits()   << endl;
    cout << "Page Faults: " << arr.get_page_faults() << endl;
    cout << "=========================================" << endl;

    string output_txt = output_file + ".txt";
    cout << "Exportando versión legible a: " << output_txt << endl;
    export_to_text(output_file, output_txt);
    cout << "Listo." << endl;

    return 0;
}