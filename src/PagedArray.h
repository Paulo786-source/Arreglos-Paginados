#ifndef PAGEDARRAY_H
#define PAGEDARRAY_H
#include <string>

using namespace std;

#include <cstdio>

class PagedArray
{
    private:

    // --- Variables --- //

    // Variables de paginación
    int page_hits;
    int page_faults;

    // Manejo de archivo
    FILE* file;
    long long total_elements;

    // Configuración de paginación
    int page_size; // Cantidad de datos que caben en una página
    int page_count; // Cantidad de páginas que van a estar activas en RAM

    // Información RAM simulada
    int** data_frames; //Es un puntero doble, un arreglo de arreglos (Matriz)
    int* loaded_frames;
    int* last_used;
    bool* dirty_bit; // Indica si el frame fue modificado

    int time_counter; // Nos va a ayudar con el algoritmo LRU

    // --- Funciones --- //

    // Verifica si la página esta en la RAM
    int find_page_in_RAM(int page_number);

    // Nos indica cual fue el frame que menos se ha usado
    int find_lru_frame();

    // Carga una página del disco a la RAM
    void load_page_to_frame(int page_number, int frame_num);

    // Guarda los frames en el disco
    void save_page_to_disk(int frame_num);

    public:

    // --- Constructor --- //

    PagedArray(const char* file_path, int p_size, int p_count);

    // --- Destructor --- //

    ~PagedArray();

    long long get_total_elements() {
        return total_elements;
    }

    // --- Sobrecarga del operador [] --- //

    int& operator[](long long index);

    // --- Getters para resultados finales --- //

    int get_page_hits() { return page_hits; }
    int get_page_faults() { return page_faults; }
};

#endif