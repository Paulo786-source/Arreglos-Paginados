#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>

using namespace std;

int main (int arg_count, char* argv[])
{
    srand(time(0));

    if (arg_count < 5) //Si el usuario ingresa menos argumentos de los necesarios da "error"
    {
        cout << "Error: too few arguments." << endl;
        return 1; // Le indica al SO que el programa falló
    }

    string size_arg = argv[2];
    string output_path = argv[4];

    long long cant_num = 0; //Usamos long long debido a que son numeros grandes

    if (size_arg == "TEST_1") //En caso que queramos hacer la prueba con 10 MB
    {
        cant_num = (10LL * 1024 * 1024) / sizeof(int);
    }
    else if (size_arg == "TEST_2") //En caso que queramos hacer la prueba con 20 MB
    {
        cant_num = (20LL * 1024 * 1024) / sizeof(int);
    }
    else if (size_arg == "TEST_3") //En caso que queramos hacer la prueba con 30 MB
    {
        cant_num = (30LL * 1024 * 1024) / sizeof(int);
    }

    if (size_arg == "SMALL") //Quitar comentarios si se desea cambiar el tamaño de los archivos
    {
        // 512 MB
        cant_num = (512LL * 1024 * 1024) / sizeof(int);

        // 256 MB
        //cant_num = (256LL * 1024 * 1024) / sizeof(int);;
    }
    if (size_arg == "MEDIUM")
    {
        // 1 GB
        cant_num = (1024LL * 1024 * 1024) / sizeof(int);

        // 512 MB
        //cant_num = (512LL * 1024 * 1024) / sizeof(int);
    }
    if (size_arg == "LARGE")
    {
        // 2 GB
        cant_num = (2048LL * 1024 * 1024) / sizeof(int);

        // 1 GB
        //cant_num = (1024LL * 1024 * 1024) / sizeof(int);
    }

    FILE *file = fopen(output_path.c_str(), "wb");
    // fopen toma el nombre del archivo y el modo. "wb" significa Write Binary
    // c_str() convierte nuestro string de C++ al formato de texto viejo de C que ocupa fopen

    if (file == NULL)
    {
        cout << "Error: could not open file for writing." << endl;
        return 1; // Le indica al SO que el programa falló
    }

    const int tamano_bloque = 262144;

    int* buffer = new int[tamano_bloque];

    cout << "Generating binary fine, please wait" << endl;

    for (long long i = 0; i < cant_num; i += tamano_bloque)
    {
        for (int j = 0; j < tamano_bloque; j++)
        {
            buffer[j] = rand();
        }

        fwrite(buffer, sizeof(int), tamano_bloque, file);
    }

    delete [] buffer;
    fclose(file);

    cout << "Done." << endl;
    return 0;


}
