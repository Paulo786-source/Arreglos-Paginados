#include <iostream>
#include <string>
#include <cstdio>

#include "PagedArray.h"

using namespace std;

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

}