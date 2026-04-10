#include <iostream>
#include <string>
#include <cstdio>
#include <random>

using namespace std;

int main(int arg_count, char* argv[])
{
    cout << "Programa iniciado..." << endl;

    if (arg_count < 5)
    {
        cout << "Error: too few arguments." << endl;
        cout << "Uso: generator -size <SIZE> -output <OUTPUT FILE PATH>" << endl;
        cout << "Tamaños válidos: SMALL, MEDIUM, LARGE, TEST_1, TEST_2, TEST_3" << endl;
        return 1;
    }

    string size_arg = argv[2];
    string output_path = argv[4];

    long long cant_num = 0;

    if (size_arg == "TEST_1")
        cant_num = (10LL * 1024 * 1024) / sizeof(int);
    else if (size_arg == "TEST_2")
        cant_num = (20LL * 1024 * 1024) / sizeof(int);
    else if (size_arg == "TEST_3")
        cant_num = (30LL * 1024 * 1024) / sizeof(int);
    else if (size_arg == "SMALL")
        cant_num = (32LL * 1024 * 1024) / sizeof(int);
    else if (size_arg == "MEDIUM")
        cant_num = (64LL * 1024 * 1024) / sizeof(int);
    else if (size_arg == "LARGE")
        cant_num = (128LL * 1024 * 1024) / sizeof(int);
    else
    {
        cout << "Error: Size '" << size_arg << "' no reconocido." << endl;
        return 1;
    }

    FILE* file = fopen(output_path.c_str(), "wb");
    if (file == NULL)
    {
        cout << "Error: no se pudo abrir el archivo para escritura." << endl;
        return 1;
    }

    // CORRECCIÓN: mt19937 genera números en un rango de 2^32 valores posibles,
    // distribuidos uniformemente. rand() solo generaba 32,768 valores distintos
    // en Windows, lo que hacía que QuickSort se comportara como O(n²).
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 1000000000);

    const long long tamano_bloque = 262144;
    int* buffer = new int[tamano_bloque];

    cout << "Generando archivo binario, por favor espere..." << endl;
    cout << "Total de enteros a generar: " << cant_num << endl;

    long long i = 0;
    while (i < cant_num)
    {
        long long a_escribir = tamano_bloque;
        if (i + tamano_bloque > cant_num)
            a_escribir = cant_num - i;

        for (long long j = 0; j < a_escribir; j++)
            buffer[j] = dis(gen);

        fwrite(buffer, sizeof(int), (size_t)a_escribir, file);
        i += a_escribir;
    }

    delete[] buffer;
    fclose(file);

    cout << "Archivo generado exitosamente en: " << output_path << endl;
    return 0;
}