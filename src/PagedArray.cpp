#include "PagedArray.h"
#include <iostream>

PagedArray::PagedArray(const char* file_path, int p_size, int p_count)
{
    page_size  = p_size;
    page_count = p_count;

    page_hits   = 0;
    page_faults = 0;
    time_counter = 0;

    // Abrir el archivo en modo lectura/escritura binario
    file = fopen(file_path, "rb+");

    if (file == nullptr)
    {
        std::cout << "Error: no se pudo abrir el archivo." << std::endl;
        return;
    }

    // --- Calcular el total de elementos --- //
    // CORRECCIÓN: usar fseeko/ftello o _fseeki64 para soportar archivos >2GB
    // En Linux/macOS fseeko con off_t de 64-bit es suficiente.
    // En Windows usar _fseeki64 / _ftelli64.
#ifdef _WIN32
    _fseeki64(file, 0, SEEK_END);
    long long tamano_bytes = _ftelli64(file);
    _fseeki64(file, 0, SEEK_SET);
#else
    fseeko(file, 0, SEEK_END);
    long long tamano_bytes = (long long)ftello(file);
    fseeko(file, 0, SEEK_SET);
#endif

    total_elements = tamano_bytes / sizeof(int);

    // --- Inicializar la RAM simulada --- //
    loaded_frames = new int[page_count];
    last_used     = new int[page_count];
    dirty_bit     = new bool[page_count];
    data_frames   = new int*[page_count];

    for (int i = 0; i < page_count; i++)
    {
        loaded_frames[i] = -1;
        dirty_bit[i]     = false;
        last_used[i]     = 0;
        data_frames[i]   = new int[page_size];
    }
}

PagedArray::~PagedArray()
{
    // Guardar en disco las páginas modificadas que quedan en RAM
    for (int i = 0; i < page_count; i++)
    {
        if (dirty_bit[i] == true && loaded_frames[i] != -1)
        {
            save_page_to_disk(i);
        }
    }

    if (file != nullptr)
    {
        fflush(file); // Forzar flush antes de cerrar
        fclose(file);
    }

    for (int i = 0; i < page_count; i++)
    {
        delete[] data_frames[i];
    }

    delete[] data_frames;
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
    // Si hay un frame vacío, úsalo directamente
    for (int i = 0; i < page_count; i++)
    {
        if (loaded_frames[i] == -1) return i;
    }

    // Si no, elegir el frame menos recientemente usado
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
    // CORRECCIÓN: usar fseeko/fseeki64 para offsets >2GB
    long long byte_offset = (long long)page_num * page_size * sizeof(int);

#ifdef _WIN32
    _fseeki64(file, byte_offset, SEEK_SET);
#else
    fseeko(file, (off_t)byte_offset, SEEK_SET);
#endif

    // Leer la página del disco — puede ser parcial si es la última página
    size_t leidos = fread(data_frames[frame_num], sizeof(int), page_size, file);

    // Rellenar con cero si la página es parcial (última página del archivo)
    for (size_t k = leidos; k < (size_t)page_size; k++)
    {
        data_frames[frame_num][k] = 0;
    }

    loaded_frames[frame_num] = page_num;
    dirty_bit[frame_num]     = false; // Recién cargada, no modificada
}

void PagedArray::save_page_to_disk(int frame_num)
{
    int page_num = loaded_frames[frame_num];

    if (page_num == -1) return;

    long long byte_offset = (long long)page_num * page_size * sizeof(int);

#ifdef _WIN32
    _fseeki64(file, byte_offset, SEEK_SET);
#else
    fseeko(file, (off_t)byte_offset, SEEK_SET);
#endif

    fwrite(data_frames[frame_num], sizeof(int), page_size, file);

    dirty_bit[frame_num] = false;
}

// Función auxiliar: devuelve el frame donde quedó cargado el índice
int PagedArray::get_frame_for_index(long long index)
{
    int page_number = (int)(index / page_size);
    int frame = find_page_in_RAM(page_number);

    if (frame == -1)
    {
        page_faults++;
        int lru = find_lru_frame();

        if (dirty_bit[lru] == true && loaded_frames[lru] != -1)
        {
            save_page_to_disk(lru);
        }

        load_page_to_frame(page_number, lru);
        frame = lru;
    }
    else
    {
        page_hits++;
    }

    // Resetear el contador periódicamente para evitar overflow con archivos grandes
    if (time_counter >= 2000000000)
    {
        // Reescalar todos los last_used de forma proporcional
        for (int i = 0; i < page_count; i++)
        {
            last_used[i] = last_used[i] / 2;
        }
        time_counter = time_counter / 2;
    }

    time_counter++;
    last_used[frame] = time_counter;

    // NOTA: NO marcamos dirty_bit aquí. El Proxy lo hará solo si hay escritura.

    return frame;
}

PagedArray::Proxy PagedArray::operator[](long long index)
{
    // Solo le pasamos el índice, el Proxy se encargará de buscar el frame cuando se use.
    return Proxy(*this, index);
}