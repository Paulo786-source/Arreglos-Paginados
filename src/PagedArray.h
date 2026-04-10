#ifndef PAGEDARRAY_H
#define PAGEDARRAY_H
#include <cstdio>

// ============================================================
// PagedArray — Arreglo paginado sobre archivo binario
//
// Simula un arreglo de enteros de gran tamaño manteniendo
// solo una fracción de él en memoria RAM. El resto vive en
// disco y se carga bajo demanda usando un esquema de páginas.
//
// Internamente usa:
//   - Un arreglo de frames (páginas en RAM)
//   - Un HashMap de open addressing para buscar páginas en O(1)
//   - Algoritmo LRU para decidir qué página desalojar
//   - dirty_bit para escribir al disco solo cuando es necesario
// ============================================================

class PagedArray
{
private:

    // --- Contadores de rendimiento ---
    long long page_hits;    // Veces que el dato pedido ya estaba en RAM
    long long page_faults;  // Veces que hubo que cargarlo desde disco

    // --- Manejo del archivo en disco ---
    FILE* file;            // Puntero al archivo binario que se está ordenando
    long long total_elements;  // Total de enteros en el archivo

    // --- Parámetros de paginación ---
    int page_size;   // Cantidad de enteros que caben en una página
    int page_count;  // Cantidad de páginas que pueden estar en RAM al mismo tiempo

    // --- RAM simulada ---
    // data_frames es una matriz: cada fila es una página cargada en RAM
    int** data_frames;    // Los datos de cada frame en RAM
    int* loaded_frames;  // Qué número de página está cargado en cada frame
    int* last_used;      // Marca de tiempo del último uso de cada frame (para LRU)
    bool* dirty_bit;      // true si el frame fue modificado y necesita guardarse en disco

    // --- HashMap: page_number → frame_index ---
    // Permite saber en O(1) si una página está en RAM y en qué frame.
    // Usa open addressing con linear probing para resolver colisiones.
    // map_size es primo para distribuir mejor las claves.
    static const int EMPTY_SLOT = -1;  // Valor centinela para slots vacíos
    int* hash_keys;    // Número de página almacenado en cada slot del hash
    int* hash_values;  // Frame correspondiente a esa página
    int  map_size;     // Tamaño de la tabla hash (siempre primo)

    // Contador que incrementa en cada page fault para implementar LRU
    int time_counter;

    // --- Funciones privadas de soporte ---
    int  find_lru_frame();                          // Encuentra el frame menos recientemente usado
    void load_page_to_frame(int page_num, int frame_num);  // Carga página del disco a RAM
    void save_page_to_disk(int frame_num);          // Escribe frame modificado al disco

    int  hash_slot(int page_number) const;          // Calcula el slot inicial en el hash
    void hash_insert(int page_number, int frame);   // Inserta entrada en el hash
    void hash_remove(int page_number);              // Elimina entrada del hash (con reparación de cadena)
    static int next_prime(int n);                   // Calcula el primo más cercano >= n

    // -------------------------------------------------------
    // find_page_in_RAM — búsqueda O(1) con HashMap
    //
    // Se declara inline en el .h porque se llama en cada
    // get() y set(). El compilador la expande directamente
    // en el punto de llamada, eliminando el overhead de
    // llamada de función con miles de millones de accesos.
    // -------------------------------------------------------
    inline int find_page_in_RAM(int page_number) const
    {
        int slot = page_number % map_size;

        // Linear probing: avanzar hasta encontrar la página o un slot vacío
        while (hash_keys[slot] != EMPTY_SLOT)
        {
            if (hash_keys[slot] == page_number)
                return hash_values[slot];  // Página encontrada → devolver frame
            slot = (slot + 1) % map_size;
        }

        return -1;  // Página no está en RAM
    }

public:

    // Constructor: abre el archivo y prepara las estructuras de paginación
    PagedArray(const char* file_path, int p_size, int p_count);

    // Destructor: guarda páginas modificadas y libera toda la memoria
    ~PagedArray();

    // Getters básicos
    long long get_total_elements() const { return total_elements; }
    long long get_page_hits()      const { return page_hits; }
    long long get_page_faults()    const { return page_faults; }

    // -------------------------------------------------------
    // get — lectura de un elemento por índice
    //
    // Calcula a qué página pertenece el índice y verifica
    // si está en RAM. Si no está, la carga desde disco
    // (page fault). Si está, la devuelve directamente (hit).
    //
    // El LRU solo se actualiza en page faults, no en cada
    // hit. Con miles de millones de accesos esto elimina
    // miles de millones de operaciones innecesarias.
    // -------------------------------------------------------
    inline int get(long long index)
    {
        // Calcular qué página contiene este índice
        int page_number = (int)(index / page_size);
        int frame = find_page_in_RAM(page_number);

        if (frame == -1)
        {
            // PAGE FAULT: la página no está en RAM, hay que cargarla
            page_faults++;

            // Elegir el frame menos recientemente usado para desalojar
            int lru = find_lru_frame();

            // Si ese frame tiene datos, hay que liberarlo primero
            if (loaded_frames[lru] != EMPTY_SLOT)
            {
                // Eliminar la entrada vieja del HashMap
                hash_remove(loaded_frames[lru]);

                // Si fue modificado, guardar cambios en disco antes de pisar
                if (dirty_bit[lru])
                    save_page_to_disk(lru);
            }

            // Cargar la nueva página en el frame liberado
            load_page_to_frame(page_number, lru);

            // Actualizar el contador LRU — solo en faults, no en cada hit
            if (time_counter >= 2000000000)
            {
                // Reescalar para evitar overflow del contador
                for (int i = 0; i < page_count; i++) last_used[i] /= 2;
                time_counter /= 2;
            }
            time_counter++;
            last_used[lru] = time_counter;

            frame = lru;
        }
        else
        {
            // PAGE HIT: la página ya estaba en RAM, solo contabilizar
            page_hits++;
        }

        // Devolver el elemento en su posición dentro de la página
        return data_frames[frame][(int)(index % page_size)];
    }

    // -------------------------------------------------------
    // set — escritura de un elemento por índice
    //
    // Mismo mecanismo que get para localizar el frame.
    // Adicionalmente activa dirty_bit para indicar que
    // esta página fue modificada y debe guardarse en disco
    // antes de ser desalojada.
    // -------------------------------------------------------
    inline void set(long long index, int value)
    {
        // Calcular qué página contiene este índice
        int page_number = (int)(index / page_size);
        int frame = find_page_in_RAM(page_number);

        if (frame == -1)
        {
            // PAGE FAULT: la página no está en RAM, hay que cargarla
            page_faults++;

            int lru = find_lru_frame();

            if (loaded_frames[lru] != EMPTY_SLOT)
            {
                hash_remove(loaded_frames[lru]);
                if (dirty_bit[lru])
                    save_page_to_disk(lru);
            }

            load_page_to_frame(page_number, lru);

            if (time_counter >= 2000000000)
            {
                for (int i = 0; i < page_count; i++) last_used[i] /= 2;
                time_counter /= 2;
            }
            time_counter++;
            last_used[lru] = time_counter;

            frame = lru;
        }
        else
        {
            // PAGE HIT: la página ya estaba en RAM
            page_hits++;
        }

        // Escribir el valor en el frame y marcar como modificado
        data_frames[frame][(int)(index % page_size)] = value;
        dirty_bit[frame] = true;  // Este frame necesita guardarse en disco al desalojarse
    }
};

#endif