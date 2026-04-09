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
    int  find_page_in_RAM(int page_number);
    int  find_lru_frame();
    void load_page_to_frame(int page_number, int frame_num);
    void save_page_to_disk(int frame_num);

    int  hash_slot(int page_number) const;
    void hash_insert(int page_number, int frame);
    void hash_remove(int page_number);
    static int next_prime(int n);

public:

    PagedArray(const char* file_path, int p_size, int p_count);
    ~PagedArray();

    long long get_total_elements() const { return total_elements; }
    long long get_page_hits()      const { return page_hits; }
    long long get_page_faults()    const { return page_faults; }

    // --- get y set inline ---
    // Al estar definidos en el .h el compilador los expande directamente
    // en el punto de llamada, eliminando el overhead de llamada de función.
    // Con ~10,000 millones de accesos esto marca una diferencia real.

    inline int get(long long index)
    {
        int page_number = (int)(index / page_size);
        int frame = find_page_in_RAM(page_number);

        if (frame == -1)
        {
            page_faults++;
            int lru = find_lru_frame();

            if (loaded_frames[lru] != EMPTY_SLOT)
            {
                hash_remove(loaded_frames[lru]);
                if (dirty_bit[lru]) save_page_to_disk(lru);
            }

            load_page_to_frame(page_number, lru);
            frame = lru;
        }
        else
        {
            page_hits++;
        }

        if (time_counter >= 2000000000)
        {
            for (int i = 0; i < page_count; i++) last_used[i] /= 2;
            time_counter /= 2;
        }
        time_counter++;
        last_used[frame] = time_counter;

        return data_frames[frame][(int)(index % page_size)];
    }

    inline void set(long long index, int value)
    {
        int page_number = (int)(index / page_size);
        int frame = find_page_in_RAM(page_number);

        if (frame == -1)
        {
            page_faults++;
            int lru = find_lru_frame();

            if (loaded_frames[lru] != EMPTY_SLOT)
            {
                hash_remove(loaded_frames[lru]);
                if (dirty_bit[lru]) save_page_to_disk(lru);
            }

            load_page_to_frame(page_number, lru);
            frame = lru;
        }
        else
        {
            page_hits++;
        }

        if (time_counter >= 2000000000)
        {
            for (int i = 0; i < page_count; i++) last_used[i] /= 2;
            time_counter /= 2;
        }
        time_counter++;
        last_used[frame] = time_counter;

        data_frames[frame][(int)(index % page_size)] = value;
        dirty_bit[frame] = true;
    }
};

#endif