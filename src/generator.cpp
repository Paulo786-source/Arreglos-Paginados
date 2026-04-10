#include <iostream>
#include <string>
#include <cstdio>
#include <random>

using namespace std;

// ============================================================
// generator — Generador de archivos binarios de prueba
//
// Genera un archivo binario con números enteros aleatorios
// para ser ordenados por el programa sorter.
//
// Uso:
//   generator -size <SIZE> -output <OUTPUT FILE PATH>
//
// Tamaños disponibles:
//   TEST_1 →  10 MB   (para pruebas rápidas)
//   TEST_2 →  20 MB
//   TEST_3 →  30 MB
//   SMALL  →  32 MB
//   MEDIUM →  64 MB
//   LARGE  → 128 MB
//
// El archivo generado es completamente binario — cada 4 bytes
// corresponden a un entero. No hay separadores ni cabeceras.
// ============================================================

int main(int arg_count, char* argv[])
{
    cout << "Programa iniciado..." << endl;

    // Validar que se pasaron los argumentos mínimos necesarios
    if (arg_count < 5)
    {
        cout << "Error: too few arguments." << endl;
        cout << "Uso: generator -size <SIZE> -output <OUTPUT FILE PATH>" << endl;
        cout << "Tamaños válidos: SMALL, MEDIUM, LARGE, TEST_1, TEST_2, TEST_3" << endl;
        return 1;
    }

    // argv[0] = nombre del programa
    // argv[1] = "-size"
    // argv[2] = el tamaño deseado
    // argv[3] = "-output"
    // argv[4] = la ruta del archivo de salida
    string size_arg = argv[2];
    string output_path = argv[4];

    // Calcular cuántos enteros generar según el tamaño solicitado
    // Se usa long long para soportar archivos de más de 2 GB sin overflow
    long long cant_num = 0;

    if (size_arg == "TINY")
        cant_num = 100000;  // ~400 KB, para IS/SS/BS
    else if (size_arg == "TEST_1")
        cant_num = (10LL * 1024 * 1024) / sizeof(int);  // 10 MB
    else if (size_arg == "TEST_2")
        cant_num = (20LL * 1024 * 1024) / sizeof(int);  // 20 MB
    else if (size_arg == "TEST_3")
        cant_num = (30LL * 1024 * 1024) / sizeof(int);  // 30 MB
    else if (size_arg == "SMALL")
        cant_num = (32LL * 1024 * 1024) / sizeof(int);  // 32 MB
    else if (size_arg == "MEDIUM")
        cant_num = (64LL * 1024 * 1024) / sizeof(int);  // 64 MB
    else if (size_arg == "LARGE")
        cant_num = (128LL * 1024 * 1024) / sizeof(int); // 128 MB
    else
    {
        cout << "Error: Size '" << size_arg << "' no reconocido." << endl;
        return 1;
    }

    // Abrir el archivo de salida en modo escritura binaria
    // "wb" = Write Binary: crea el archivo o lo sobreescribe si existe
    FILE* file = fopen(output_path.c_str(), "wb");
    if (file == NULL)
    {
        cout << "Error: no se pudo abrir el archivo para escritura." << endl;
        return 1;
    }

    // --- Configurar el generador de números aleatorios ---
    //
    // Se usa mt19937 (Mersenne Twister) en lugar de rand() por dos razones:
    //
    // 1. rand() en Windows tiene RAND_MAX = 32,767 — solo 32,768 valores
    //    distintos. Con millones de enteros hay altísima repetición, lo que
    //    hace que QuickSort elija pivotes muy comunes y se degrade a O(n²).
    //
    // 2. mt19937 tiene un periodo de 2^19937 y genera distribuciones
    //    verdaderamente uniformes en el rango especificado.
    //
    // random_device obtiene una semilla del sistema operativo (entropía real),
    // garantizando que cada ejecución genere un archivo diferente.
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 1000000000);  // Rango de 0 a 1,000,000,000

    // Buffer intermedio para escribir en bloques de 1 MB
    // Esto es más eficiente que escribir un entero a la vez
    const long long tamano_bloque = 262144;  // 262,144 enteros = 1 MB
    int* buffer = new int[tamano_bloque];

    cout << "Generando archivo binario, por favor espere..." << endl;
    cout << "Total de enteros a generar: " << cant_num << endl;

    long long i = 0;
    while (i < cant_num)
    {
        // Calcular cuántos enteros escribir en esta iteración
        // En la última iteración puede ser menos que tamano_bloque
        long long a_escribir = tamano_bloque;
        if (i + tamano_bloque > cant_num)
            a_escribir = cant_num - i;

        // Llenar el buffer con números aleatorios
        for (long long j = 0; j < a_escribir; j++)
            buffer[j] = dis(gen);

        // Escribir el bloque al archivo en formato binario
        fwrite(buffer, sizeof(int), (size_t)a_escribir, file);
        i += a_escribir;
    }

    // Liberar recursos
    delete[] buffer;
    fclose(file);

    cout << "Archivo generado exitosamente en: " << output_path << endl;
    return 0;
}