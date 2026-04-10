#ifndef PAGEDARRAY_H
#define PAGEDARRAY_H
#include <cstdio>

class PagedArray
{
private:

    // --- Contadores ---
    long long page_hits;
    long long page_faults;

    // --- Archivo ---
    FILE* file;
    long long total_elements;

    // --- Configuración ---
    int page_size;
    int page_count;

    // --- RAM simulada ---
    int** data_frames;
    int* loaded_frames;
    int* last_used;
    bool* dirty_bit;

    // --- HashMap page_number → frame (O(1)) ---
    static const int EMPTY_SLOT = -1;
    int* hash_keys;
    int* hash_values;
    int  map_size;

    int time_counter;

    // --- Funciones privadas ---
    int  find_lru_frame();
    void load_page_to_frame(int page_number, int frame_num);
    void save_page_to_disk(int frame_num);

    int  hash_slot(int page_number) const;
    void hash_insert(int page_number, int frame);
    void hash_remove(int page_number);
    static int next_prime(int n);

    // find_page_in_RAM inline — se llama en cada acceso, debe ser lo más rápido posible
    inline int find_page_in_RAM(int page_number) const
    {
        int slot = page_number % map_size;
        while (hash_keys[slot] != EMPTY_SLOT)
        {
            if (hash_keys[slot] == page_number) return hash_values[slot];
            slot = (slot + 1) % map_size;
        }
        return -1;
    }

public:

    PagedArray(const char* file_path, int p_size, int p_count);
    ~PagedArray();

    long long get_total_elements() const { return total_elements; }
    long long get_page_hits()      const { return page_hits; }
    long long get_page_faults()    const { return page_faults; }

    // -------------------------------------------------------
    // get — path caliente: si la página está en RAM, solo lee.
    // LRU se actualiza SOLO en page faults, no en cada acceso.
    // Con miles de millones de hits esto elimina miles de
    // millones de incrementos y escrituras innecesarias.
    // -------------------------------------------------------
    inline int get(long long index)
    {
        int page_number = (int)(index / page_size);
        int frame = find_page_in_RAM(page_number);

        if (frame == -1)
        {
            // PAGE FAULT — cargar página y actualizar LRU
            page_faults++;

            int lru = find_lru_frame();

            if (loaded_frames[lru] != EMPTY_SLOT)
            {
                hash_remove(loaded_frames[lru]);
                if (dirty_bit[lru]) save_page_to_disk(lru);
            }

            load_page_to_frame(page_number, lru);

            // Actualizar LRU solo aquí, no en cada hit
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
            // PAGE HIT — solo contar, sin overhead de LRU
            page_hits++;
        }

        return data_frames[frame][(int)(index % page_size)];
    }

    // -------------------------------------------------------
    // set — mismo principio: LRU solo en page faults
    // -------------------------------------------------------
    inline void set(long long index, int value)
    {
        int page_number = (int)(index / page_size);
        int frame = find_page_in_RAM(page_number);

        if (frame == -1)
        {
            // PAGE FAULT — cargar página y actualizar LRU
            page_faults++;

            int lru = find_lru_frame();

            if (loaded_frames[lru] != EMPTY_SLOT)
            {
                hash_remove(loaded_frames[lru]);
                if (dirty_bit[lru]) save_page_to_disk(lru);
            }

            load_page_to_frame(page_number, lru);

            // Actualizar LRU solo aquí
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
            // PAGE HIT — solo contar
            page_hits++;
        }

        data_frames[frame][(int)(index % page_size)] = value;
        dirty_bit[frame] = true;
    }
};

#endif