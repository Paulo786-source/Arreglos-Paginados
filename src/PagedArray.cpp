#include "PagedArray.h"
#include <iostream>

PagedArray::PagedArray(const char* file_path, int p_size, int p_count)
{
    // Guardamos los parametros
    page_size = p_size;
    page_count = p_count;

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
    data_frames = new int*[page_count];

    for (int i = 0; i < page_count; i++)
    {
        loaded_frames[i] = -1;
        dirty_bit[i] = false;
        last_used[i] = 0;
        data_frames[i] = new int[page_size];
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
        delete[] data_frames[i];
    }

    delete[] data_frames;

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

void PagedArray::load_page_to_frame(int page_num, int frame_num)
{
    // Calculamos la posición en bytes
    long long pos_num = (long long) page_num * page_size * sizeof(int);

    // Buscamos la posición
    // fseek(archivo, movimiento, punto de partica)
    fseek(file, pos_num, SEEK_SET); // SEEK_SET: Valor ya establecido que marca el inicio

    // Leemos del disco a la RAM
    // fread(destino, tamaño del elemento, cantidad de elementos, archivo)
    fread(data_frames[frame_num], sizeof(int), page_size, file);

    // Actualizamos informacion en RAM
    loaded_frames[frame_num] = page_num;

    // Actualizamos la variable dirty_bit
    dirty_bit[frame_num] = false;
}

void PagedArray::save_page_to_disk(int frame_num)
{
    int page_num = loaded_frames[frame_num];

    // Verificamos si hay algo en el frame
    if (page_num == -1)
    {
        return;
    }

    // Donde lo vamos a guardar
    long long pos_page = (long long) page_num * page_size * sizeof(int);

    // Buscamos donde queremos guardar
    fseek(file, pos_page, SEEK_SET);

    // Sobreescribimos la informacion
    // fwrite(origen, tamaño, cantidad, archivo)
    fwrite(data_frames[frame_num], sizeof(int), page_size, file);

    // Actualizamos la variable dirty_bit
    dirty_bit[frame_num] = false;
}

int& PagedArray::operator[](long long index)
{
    // Calculamos la página
    int page_number = index/page_size;

    // Calculamos la posición del valor
    int pos_in_page = index % page_size;

    // Buscamos la página en RAM
    int frame = find_page_in_RAM(page_number);

    if (frame == -1) // Si no encontramos la página en RAM
    {
        page_faults += 1; // Sumamos un page fault
        int lru = find_lru_frame(); // Buscamos el espacio menos usado
        if (dirty_bit[lru] == true) // Si fue modificado, guardamos los cambios
        {
            save_page_to_disk(lru);
        }

        load_page_to_frame(page_number, lru); // Cargamos la página a la memoria RAM

        frame = lru;
    }

    else
    {
        page_hits += 1;
    }

    time_counter++;
    last_used[frame] = time_counter;
    dirty_bit[frame] = true;

    return data_frames[frame][pos_in_page];
}
