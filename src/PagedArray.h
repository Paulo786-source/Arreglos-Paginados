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
    int  get_frame_for_index(long long index);

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

    // --- API pública: get y set explícitos ---
    // Sin Proxy, sin ambigüedad, exactamente una llamada a get_frame_for_index
    // por operación. Los algoritmos de ordenamiento los usan directamente.
    int  get(long long index);
    void set(long long index, int value);
};

#endif