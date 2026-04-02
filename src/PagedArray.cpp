#include "PagedArray.h"
#include <iostream>

PagedArray::PagedArray(const char* file_path, int p_size, int p_count, long long total_elems)
{
    // Guardamos los parametros
    page_size = p_size;
    page_count = p_count;
    total_elements = total_elems;

    // Inicializamos los contadores
    page_hits = 0;
    page_faults = 0;
    time_counter = 0;

    // Abrir el archivo
    file = fopen(file_path, "rb+");

    if (file == nullptr)
    {
        std::cout << "Error opening file" << std::endl;
    }

    // --- Memoria RAM simulada --- //

    loaded_frames = new int[page_count];
    last_used = new int[page_count];
    dirty_bit = new bool[page_count];
    frames = new int*[page_count];

    for (int i = 0; i < page_count; i++)
    {
        loaded_frames[i] = -1;
        dirty_bit[i] = false;
        last_used[i] = 0;
        frames[i] = new int[page_size];
    }
}

PagedArray::~PagedArray()
{
    // Cerramos el archivo
    if (file != nullptr)
    {
        fclose(file);
    }

    // Destruimos el puntero doble
    for (int i = 0; i < page_count; i++)
    {
        delete[] frames[i];
    }

    delete[] frames;

    // Destruimos los demas arreglos

    delete[] loaded_frames;
    delete[] last_used;
    delete[] dirty_bit;
}

int PagedArray::find_page_in_RAM(int page_number)
{
    for (int i = 0; i < page_count; i++)
    {
        if (loaded_frames[i] == page_number)
        {
            return i;
        }
    }

    return -1;
}

int PagedArray::find_lru_frame()
{
    // Si hay un espacio vacio, lo usamos
    for (int i = 0; i < page_count; i++) {
        if (loaded_frames[i] == -1) return i;
    }

    // Si no, buscamos el menos usado
    int min_value = last_used[0];
    int min_index = 0;

    for (int i = 1; i < page_count; i++)
    {
        if (last_used[i] < min_value)
        {
            min_value = last_used[i];
            min_index = i;
        }
    }
    return min_index;
}
