#include <iostream>
#include <string>
#include <cstdio>
#include <chrono>
#include "PagedArray.h"
#include "SortAlgorithms.h"

using namespace std;
using namespace std::chrono;

// Función que clona el archivo
bool clon_file(const string& in_path, const string& out_path)
{
    // Abrimos el archivo en modo lectura
    FILE* in_file = fopen(in_path.c_str(), "rb");

    // Verificacion de que se pudo acceder al archivo
    if (in_file == nullptr)
    {
        cout << "Error: No se pudo abrir el archivo" << endl;
        return false;
    }

    // Creamos el archivo destino
    FILE* out_file = fopen(out_path.c_str(), "wb");

    // Verificacion de que se pudo crear el archivo destino
    if (out_file == nullptr)
    {
        cout << "Error: No se pudo generar el archivo" << endl;
        fclose(in_file);
        return false;
    }

    // Creamos el buffer para copiar el archivo por lotes
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

int main(int argc, char* argv[])
{
    // Validacion de que se ingresen la cantidad de argumentos correcta
    if (argc < 11)
    {
        cout << "Error: too few arguments." << endl;
        return 1;
    }
    string input_file = argv[2];
    string output_file = argv[4];
    string alg = argv[6];
    int page_size = stoi(argv[8]);
    int page_count = stoi(argv[10]);

    // Clonamos el archivo

    cout << "Clonando archivo..." << endl;

    if (clon_file(input_file, output_file) == false)
    {
        cout << "Se produjo un error al clonar el archivo" << endl;
        return 1;
    }

    cout << "Se ha clonado el archivo con exito" << endl;

    PagedArray arr(output_file.c_str(), page_size, page_count);
    long long cantidad_datos = arr.get_total_elements();

    cout << "Iniciando ordenamiento..." << endl;

    // Iniciamos el tiempo
    auto inicio_tiempo = high_resolution_clock::now();

    // Seleción de algoritmo
    if (alg == "QS") {
        quickSort(arr, 0, cantidad_datos - 1);
    }
    else if (alg == "IS") {
        insertionSort(arr, cantidad_datos);
    }
    else if (alg == "SS") {
        selectionSort(arr, cantidad_datos);
    }
    else if (alg == "BS") {
        // Validación de seguridad para archivos gigantes
        if (cantidad_datos > 10000) {
            cout << "Advertencia: BubbleSort tomara mucho tiempo en archivos grandes." << endl;
        }
        bubbleSort(arr, cantidad_datos);
    }
    else if (alg == "MS") {
        mergeSort(arr, 0, cantidad_datos - 1);
    }
    else {
        cout << "Error: Algoritmo no reconocido." << endl;
        return 1;
    }

    // Paramos el tiempo
    auto fin_tiempo = high_resolution_clock::now();
    auto duracion = duration_cast<milliseconds>(fin_tiempo - inicio_tiempo);

    // Imprimimos el resumen
    cout << "=========================================" << endl;
    cout << "          RESUMEN DE EJECUCION           " << endl;
    cout << "=========================================" << endl;
    cout << "Tiempo durado: " << duracion.count() << " milisegundos." << endl;
    cout << "Page Hits: " << arr.get_page_hits() << endl;
    cout << "Page Faults: " << arr.get_page_faults() << endl;
    cout << "=========================================" << endl;

    return 0;
}