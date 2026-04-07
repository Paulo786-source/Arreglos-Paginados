#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <ctime>

using namespace std;

int main(int arg_count, char* argv[])
{
    cout << "Programa iniciado..." << endl;

    srand((unsigned int)time(0));

    if (arg_count < 5)
    {
        cout << "Error: too few arguments." << endl;
        cout << "Uso: generator -size <SIZE> -output <OUTPUT FILE PATH>" << endl;
        cout << "Tamaños válidos: SMALL, MEDIUM, LARGE, TEST_1, TEST_2, TEST_3" << endl;
        return 1;
    }

    string size_arg    = argv[2];
    string output_path = argv[4];

    long long cant_num = 0;

    if (size_arg == "TEST_1")
    {
        cant_num = (10LL * 1024 * 1024) / sizeof(int);
    }
    else if (size_arg == "TEST_2")
    {
        cant_num = (20LL * 1024 * 1024) / sizeof(int);
    }
    else if (size_arg == "TEST_3")
    {
        cant_num = (30LL * 1024 * 1024) / sizeof(int);
    }
    else if (size_arg == "SMALL")
    {
        cant_num = (128LL * 1024 * 1024) / sizeof(int);
    }
    else if (size_arg == "MEDIUM")
    {
        cant_num = (1024LL * 1024 * 1024) / sizeof(int);
    }
    else if (size_arg == "LARGE")
    {
        cant_num = (2048LL * 1024 * 1024) / sizeof(int);
    }
    else
    {
        cout << "Error: Size '" << size_arg << "' no reconocido." << endl;
        cout << "Valores válidos: SMALL (512MB), MEDIUM (1GB), LARGE (2GB)" << endl;
        return 1;
    }

    FILE* file = fopen(output_path.c_str(), "wb");

    if (file == NULL)
    {
        cout << "Error: no se pudo abrir el archivo para escritura." << endl;
        return 1;
    }

    const long long tamano_bloque = 262144; // 256 KB por bloque

    int* buffer = new int[tamano_bloque];

    cout << "Generando archivo binario, por favor espere..." << endl;
    cout << "Total de enteros a generar: " << cant_num << endl;

    long long i = 0;
    while (i < cant_num)
    {
        // CORRECCIÓN: calcular cuántos enteros quedan para no pasarse del total
        long long a_escribir = tamano_bloque;
        if (i + tamano_bloque > cant_num)
        {
            a_escribir = cant_num - i;
        }

        for (long long j = 0; j < a_escribir; j++)
        {
            buffer[j] = rand();
        }

        fwrite(buffer, sizeof(int), (size_t)a_escribir, file);
        i += a_escribir;
    }

    delete[] buffer;
    fclose(file);

    cout << "Archivo generado exitosamente en: " << output_path << endl;
    return 0;
}