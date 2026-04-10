#include "PagedArray.h"
#include <iostream>

// -------------------------------------------------------
// next_prime — calcula el primo más cercano >= n
//
// Se usa para determinar el tamaño del HashMap. Un tamaño
// primo mejora la distribución de claves con linear probing
// y reduce las colisiones.
// -------------------------------------------------------
int PagedArray::next_prime(int n)
{
    if (n <= 2) return 2;
    if (n % 2 == 0) n++;  // Empezar en impar

    while (true)
    {
        bool es_primo = true;
        for (int i = 3; i * i <= n; i += 2)
            if (n % i == 0) { es_primo = false; break; }
        if (es_primo) return n;
        n += 2;  // Saltar números pares
    }
}

// -------------------------------------------------------
// Constructor
//
// Abre el archivo binario, calcula cuántos enteros tiene,
// e inicializa todas las estructuras de paginación:
// frames de RAM, arrays de control y el HashMap.
// -------------------------------------------------------
PagedArray::PagedArray(const char* file_path, int p_size, int p_count)
{
    page_size = p_size;
    page_count = p_count;

    page_hits = 0;
    page_faults = 0;
    time_counter = 0;

    // Abrir el archivo en modo lectura/escritura binario
    file = fopen(file_path, "rb+");
    if (file == nullptr)
    {
        std::cout << "Error: no se pudo abrir el archivo." << std::endl;
        return;
    }

    // Calcular el total de enteros en el archivo
    // Se usa fseeko/_fseeki64 en lugar de fseek para soportar
    // archivos mayores a 2 GB en sistemas de 32 bits
#ifdef _WIN32
    _fseeki64(file, 0, SEEK_END);
    long long tamano_bytes = _ftelli64(file);
    _fseeki64(file, 0, SEEK_SET);
#else
    fseeko(file, 0, SEEK_END);
    long long tamano_bytes = (long long)ftello(file);
    fseeko(file, 0, SEEK_SET);
#endif

    // Cada entero ocupa sizeof(int) bytes (4 bytes en la mayoría de sistemas)
    total_elements = tamano_bytes / sizeof(int);

    // --- Inicializar la RAM simulada ---
    // Cada frame es un arreglo de page_size enteros
    loaded_frames = new int[page_count];
    last_used = new int[page_count];
    dirty_bit = new bool[page_count];
    data_frames = new int* [page_count];

    for (int i = 0; i < page_count; i++)
    {
        loaded_frames[i] = EMPTY_SLOT;  // Frame vacío al inicio
        dirty_bit[i] = false;       // Sin modificaciones pendientes
        last_used[i] = 0;           // No usado aún
        data_frames[i] = new int[page_size];  // Reservar espacio para los datos
    }

    // --- Inicializar el HashMap ---
    // Tamaño primo con factor de carga ~0.5 para minimizar colisiones
    map_size = next_prime(page_count * 2 + 1);
    hash_keys = new int[map_size];
    hash_values = new int[map_size];

    for (int i = 0; i < map_size; i++)
    {
        hash_keys[i] = EMPTY_SLOT;  // Sin entradas al inicio
        hash_values[i] = EMPTY_SLOT;
    }
}

// -------------------------------------------------------
// Destructor
//
// Antes de cerrar, guarda en disco todas las páginas que
// fueron modificadas y quedaron en RAM (dirty_bit = true).
// Luego libera toda la memoria dinámica reservada.
// -------------------------------------------------------
PagedArray::~PagedArray()
{
    // Guardar páginas modificadas que todavía están en RAM
    for (int i = 0; i < page_count; i++)
        if (dirty_bit[i] && loaded_frames[i] != EMPTY_SLOT)
            save_page_to_disk(i);

    // Cerrar el archivo solo si fue abierto exitosamente
    if (file != nullptr)
    {
        fflush(file);   // Forzar escritura de cualquier dato en buffer del SO
        fclose(file);
    }

    // Liberar la matriz de frames
    for (int i = 0; i < page_count; i++)
        delete[] data_frames[i];

    // Liberar los arreglos de control
    delete[] data_frames;
    delete[] loaded_frames;
    delete[] last_used;
    delete[] dirty_bit;

    // Liberar el HashMap
    delete[] hash_keys;
    delete[] hash_values;
}

// -------------------------------------------------------
// hash_slot — calcula el slot inicial para una página
//
// Función hash simple: módulo del tamaño de la tabla.
// El tamaño primo reduce las colisiones con esta función.
// -------------------------------------------------------
int PagedArray::hash_slot(int page_number) const
{
    return page_number % map_size;
}

// -------------------------------------------------------
// hash_insert — inserta page_number → frame en el HashMap
//
// Usa linear probing: si el slot está ocupado, avanza
// al siguiente hasta encontrar uno libre.
// -------------------------------------------------------
void PagedArray::hash_insert(int page_number, int frame)
{
    int slot = hash_slot(page_number);

    // Buscar slot libre o la misma página (actualización)
    while (hash_keys[slot] != EMPTY_SLOT && hash_keys[slot] != page_number)
        slot = (slot + 1) % map_size;

    hash_keys[slot] = page_number;
    hash_values[slot] = frame;
}

// -------------------------------------------------------
// hash_remove — elimina una entrada del HashMap
//
// Con linear probing no se puede simplemente borrar un slot
// porque rompería las cadenas de búsqueda. Se usa
// "backward shift": se reubican los elementos que dependen
// del slot eliminado para mantener la consistencia.
// -------------------------------------------------------
void PagedArray::hash_remove(int page_number)
{
    int slot = hash_slot(page_number);

    // Buscar la entrada a eliminar
    while (hash_keys[slot] != EMPTY_SLOT && hash_keys[slot] != page_number)
        slot = (slot + 1) % map_size;

    // Si no estaba en la tabla, no hay nada que hacer
    if (hash_keys[slot] == EMPTY_SLOT) return;

    // Marcar el slot como vacío
    hash_keys[slot] = EMPTY_SLOT;
    hash_values[slot] = EMPTY_SLOT;

    // Reparar la cadena: reubicar entradas que podrían haber quedado
    // inaccesibles por el hueco recién creado
    int next = (slot + 1) % map_size;
    while (hash_keys[next] != EMPTY_SLOT)
    {
        int k = hash_keys[next], v = hash_values[next];
        hash_keys[next] = EMPTY_SLOT;
        hash_values[next] = EMPTY_SLOT;
        hash_insert(k, v);  // Reinsertar en su posición correcta
        next = (next + 1) % map_size;
    }
}

// -------------------------------------------------------
// find_lru_frame — elige el frame a desalojar
//
// Primero verifica si hay algún frame vacío disponible.
// Si todos están ocupados, elige el que tiene el valor
// de last_used más bajo (menos recientemente usado).
// -------------------------------------------------------
int PagedArray::find_lru_frame()
{
    // Prioridad 1: usar un frame vacío si existe
    for (int i = 0; i < page_count; i++)
        if (loaded_frames[i] == EMPTY_SLOT) return i;

    // Prioridad 2: desalojar el frame menos recientemente usado
    int min_val = last_used[0], min_idx = 0;
    for (int i = 1; i < page_count; i++)
        if (last_used[i] < min_val) { min_val = last_used[i]; min_idx = i; }

    return min_idx;
}

// -------------------------------------------------------
// load_page_to_frame — carga una página del disco a RAM
//
// Calcula el offset en bytes de la página, salta a esa
// posición del archivo y lee page_size enteros al frame.
// Si la página es la última y es incompleta, rellena
// el resto con ceros para no dejar memoria sin inicializar.
// -------------------------------------------------------
void PagedArray::load_page_to_frame(int page_num, int frame_num)
{
    // Calcular posición en bytes dentro del archivo
    long long byte_offset = (long long)page_num * page_size * sizeof(int);

#ifdef _WIN32
    _fseeki64(file, byte_offset, SEEK_SET);
#else
    fseeko(file, (off_t)byte_offset, SEEK_SET);
#endif

    // Leer la página — puede devolver menos enteros si es la última página
    size_t leidos = fread(data_frames[frame_num], sizeof(int), page_size, file);

    // Rellenar con cero si la página es parcial (última del archivo)
    for (size_t k = leidos; k < (size_t)page_size; k++)
        data_frames[frame_num][k] = 0;

    // Registrar qué página está en este frame
    loaded_frames[frame_num] = page_num;
    dirty_bit[frame_num] = false;  // Recién cargada, sin modificaciones

    // Registrar en el HashMap para búsquedas futuras en O(1)
    hash_insert(page_num, frame_num);
}

// -------------------------------------------------------
// save_page_to_disk — escribe un frame modificado al disco
//
// Solo se llama cuando dirty_bit es true, es decir,
// cuando el frame fue modificado desde que se cargó.
// Esto implementa la política de escritura lazy (write-back):
// se escribe al disco solo cuando es estrictamente necesario.
// -------------------------------------------------------
void PagedArray::save_page_to_disk(int frame_num)
{
    int page_num = loaded_frames[frame_num];

    // Verificar que el frame tiene datos válidos
    if (page_num == EMPTY_SLOT) return;

    // Calcular posición en bytes dentro del archivo
    long long byte_offset = (long long)page_num * page_size * sizeof(int);

#ifdef _WIN32
    _fseeki64(file, byte_offset, SEEK_SET);
#else
    fseeko(file, (off_t)byte_offset, SEEK_SET);
#endif

    // Sobreescribir la página en disco con los datos modificados
    fwrite(data_frames[frame_num], sizeof(int), page_size, file);

    // Marcar como limpio — ya no necesita guardarse de nuevo
    dirty_bit[frame_num] = false;
}