#include "PagedArray.h"
#include <iostream>

// -------------------------------------------------------
// Calcula el primo más cercano >= n
// Se usa para el tamaño del HashMap (mejora distribución)
// -------------------------------------------------------
int PagedArray::next_prime(int n)
{
    if (n <= 2) return 2;
    if (n % 2 == 0) n++;
    while (true)
    {
        bool es_primo = true;
        for (int i = 3; i * i <= n; i += 2)
        {
            if (n % i == 0) { es_primo = false; break; }
        }
        if (es_primo) return n;
        n += 2;
    }
}

// -------------------------------------------------------
// Constructor
// -------------------------------------------------------
PagedArray::PagedArray(const char* file_path, int p_size, int p_count)
{
    page_size = p_size;
    page_count = p_count;

    page_hits = 0;
    page_faults = 0;
    time_counter = 0;

    file = fopen(file_path, "rb+");

    if (file == nullptr)
    {
        std::cout << "Error: no se pudo abrir el archivo." << std::endl;
        return;
    }

    // Calcular total de elementos (compatible con archivos >2GB)
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

    // --- Inicializar RAM simulada --- //
    loaded_frames = new int[page_count];
    last_used = new int[page_count];
    dirty_bit = new bool[page_count];
    data_frames = new int* [page_count];

    for (int i = 0; i < page_count; i++)
    {
        loaded_frames[i] = EMPTY_SLOT;
        dirty_bit[i] = false;
        last_used[i] = 0;
        data_frames[i] = new int[page_size];
    }

    // --- Inicializar HashMap --- //
    // Factor de carga ~0.5: el mapa tiene el doble de slots que páginas.
    // Usamos un primo para minimizar colisiones con linear probing.
    map_size = next_prime(page_count * 2 + 1);
    hash_keys = new int[map_size];
    hash_values = new int[map_size];

    for (int i = 0; i < map_size; i++)
    {
        hash_keys[i] = EMPTY_SLOT;
        hash_values[i] = EMPTY_SLOT;
    }
}

// -------------------------------------------------------
// Destructor
// -------------------------------------------------------
PagedArray::~PagedArray()
{
    for (int i = 0; i < page_count; i++)
    {
        if (dirty_bit[i] && loaded_frames[i] != EMPTY_SLOT)
        {
            save_page_to_disk(i);
        }
    }

    if (file != nullptr)
    {
        fflush(file);
        fclose(file);
    }

    for (int i = 0; i < page_count; i++)
        delete[] data_frames[i];

    delete[] data_frames;
    delete[] loaded_frames;
    delete[] last_used;
    delete[] dirty_bit;
    delete[] hash_keys;
    delete[] hash_values;
}

// -------------------------------------------------------
// HashMap: calcular slot inicial (hash function)
// -------------------------------------------------------
int PagedArray::hash_slot(int page_number) const
{
    // page_number siempre es no-negativo
    return page_number % map_size;
}

// -------------------------------------------------------
// HashMap: insertar page_number → frame
// -------------------------------------------------------
void PagedArray::hash_insert(int page_number, int frame)
{
    int slot = hash_slot(page_number);

    // Linear probing: buscar slot vacío
    while (hash_keys[slot] != EMPTY_SLOT && hash_keys[slot] != page_number)
    {
        slot = (slot + 1) % map_size;
    }

    hash_keys[slot] = page_number;
    hash_values[slot] = frame;
}

// -------------------------------------------------------
// HashMap: eliminar page_number (cuando se desaloja un frame)
// Con linear probing hay que "reparar" la cadena de probing.
// -------------------------------------------------------
void PagedArray::hash_remove(int page_number)
{
    int slot = hash_slot(page_number);

    // Buscar la entrada
    while (hash_keys[slot] != EMPTY_SLOT && hash_keys[slot] != page_number)
    {
        slot = (slot + 1) % map_size;
    }

    if (hash_keys[slot] == EMPTY_SLOT) return; // No estaba

    // Marcar como eliminado y reparar la cadena (backward shift)
    hash_keys[slot] = EMPTY_SLOT;
    hash_values[slot] = EMPTY_SLOT;

    int next = (slot + 1) % map_size;
    while (hash_keys[next] != EMPTY_SLOT)
    {
        int displaced_key = hash_keys[next];
        int displaced_value = hash_values[next];

        hash_keys[next] = EMPTY_SLOT;
        hash_values[next] = EMPTY_SLOT;

        hash_insert(displaced_key, displaced_value);
        next = (next + 1) % map_size;
    }
}

// -------------------------------------------------------
// find_page_in_RAM — O(1) promedio con HashMap
// Antes era O(pageCount) con búsqueda lineal.
// -------------------------------------------------------
int PagedArray::find_page_in_RAM(int page_number)
{
    int slot = hash_slot(page_number);

    while (hash_keys[slot] != EMPTY_SLOT)
    {
        if (hash_keys[slot] == page_number)
            return hash_values[slot]; // frame encontrado
        slot = (slot + 1) % map_size;
    }

    return -1; // No está en RAM
}

// -------------------------------------------------------
// find_lru_frame — elige el frame menos recientemente usado
// -------------------------------------------------------
int PagedArray::find_lru_frame()
{
    // Slot vacío disponible → usarlo directamente
    for (int i = 0; i < page_count; i++)
    {
        if (loaded_frames[i] == EMPTY_SLOT) return i;
    }

    // Todos ocupados → elegir el LRU
    int min_val = last_used[0];
    int min_idx = 0;

    for (int i = 1; i < page_count; i++)
    {
        if (last_used[i] < min_val)
        {
            min_val = last_used[i];
            min_idx = i;
        }
    }
    return min_idx;
}

// -------------------------------------------------------
// load_page_to_frame — carga una página del disco a RAM
// -------------------------------------------------------
void PagedArray::load_page_to_frame(int page_num, int frame_num)
{
    long long byte_offset = (long long)page_num * page_size * sizeof(int);

#ifdef _WIN32
    _fseeki64(file, byte_offset, SEEK_SET);
#else
    fseeko(file, (off_t)byte_offset, SEEK_SET);
#endif

    size_t leidos = fread(data_frames[frame_num], sizeof(int), page_size, file);

    // Rellenar con cero si es la última página (parcial)
    for (size_t k = leidos; k < (size_t)page_size; k++)
        data_frames[frame_num][k] = 0;

    // Actualizar estructuras
    loaded_frames[frame_num] = page_num;
    dirty_bit[frame_num] = false;

    // Registrar en el HashMap
    hash_insert(page_num, frame_num);
}

// -------------------------------------------------------
// save_page_to_disk — escribe un frame modificado al disco
// -------------------------------------------------------
void PagedArray::save_page_to_disk(int frame_num)
{
    int page_num = loaded_frames[frame_num];

    if (page_num == EMPTY_SLOT) return;

    long long byte_offset = (long long)page_num * page_size * sizeof(int);

#ifdef _WIN32
    _fseeki64(file, byte_offset, SEEK_SET);
#else
    fseeko(file, (off_t)byte_offset, SEEK_SET);
#endif

    fwrite(data_frames[frame_num], sizeof(int), page_size, file);
    dirty_bit[frame_num] = false;
}

// -------------------------------------------------------
// get_frame_for_index — núcleo de la lógica de paginación
// -------------------------------------------------------
int PagedArray::get_frame_for_index(long long index)
{
    int page_number = (int)(index / page_size);
    int frame = find_page_in_RAM(page_number); // O(1) ahora

    if (frame == -1)
    {
        page_faults++;

        int lru = find_lru_frame();

        // Si el frame elegido tiene datos modificados, guardarlos primero
        if (dirty_bit[lru] && loaded_frames[lru] != EMPTY_SLOT)
        {
            hash_remove(loaded_frames[lru]); // Eliminar del HashMap antes de pisar
            save_page_to_disk(lru);
        }
        else if (loaded_frames[lru] != EMPTY_SLOT)
        {
            hash_remove(loaded_frames[lru]); // Eliminar entrada vieja del HashMap
        }

        load_page_to_frame(page_number, lru);
        frame = lru;
    }
    else
    {
        page_hits++;
    }

    // Prevenir overflow del contador LRU en archivos grandes
    if (time_counter >= 2000000000)
    {
        for (int i = 0; i < page_count; i++)
            last_used[i] /= 2;
        time_counter /= 2;
    }

    time_counter++;
    last_used[frame] = time_counter;

    // NO marcar dirty_bit aquí — el Proxy lo hace solo al escribir
    return frame;
}

// -------------------------------------------------------
// operator[] — devuelve un Proxy para distinguir lectura/escritura
// -------------------------------------------------------
PagedArray::Proxy PagedArray::operator[](long long index)
{
    int pos_in_page = (int)(index % page_size);
    int frame = get_frame_for_index(index);

    return Proxy(*this, frame, pos_in_page);
}