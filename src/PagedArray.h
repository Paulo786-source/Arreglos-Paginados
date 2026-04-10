#ifndef PAGEDARRAY_H
#define PAGEDARRAY_H
#include <string>
#include <cstdio>

using namespace std;

class PagedArray
{
    // --- Clase interna Proxy --- //
    // Permite distinguir lectura de escritura en operator[]
    class Proxy
    {
    private:
        PagedArray& arr;
        int frame;
        int pos_in_page;

    public:
        Proxy(PagedArray& a, int f, int p)
            : arr(a), frame(f), pos_in_page(p) {
        }

        operator int() const
        {
            return arr.data_frames[frame][pos_in_page];
        }

        Proxy& operator=(int value)
        {
            arr.data_frames[frame][pos_in_page] = value;
            arr.dirty_bit[frame] = true;
            return *this;
        }

        Proxy& operator=(const Proxy& other)
        {
            return operator=((int)other);
        }
    };

private:

    // --- Contadores --- //
    int page_hits;
    int page_faults;

    // --- Archivo --- //
    FILE* file;
    long long total_elements;

    // --- Configuración de paginación --- //
    int page_size;
    int page_count;

    // --- RAM simulada --- //
    int** data_frames;
    int* loaded_frames;
    int* last_used;
    bool* dirty_bit;

    // --- HashMap: page_number → frame_index --- //
    // Convierte find_page_in_RAM de O(pageCount) a O(1).
    // Usa open addressing con linear probing.
    // map_size debe ser primo y bastante mayor que page_count
    // para mantener el factor de carga bajo (~0.5).
    static const int EMPTY_SLOT = -1;
    int* hash_keys;    // número de página almacenado en cada slot
    int* hash_values;  // frame index correspondiente
    int  map_size;     // tamaño de la tabla (primo)

    int time_counter;

    // --- Funciones privadas --- //
    int  find_page_in_RAM(int page_number);
    int  find_lru_frame();
    void load_page_to_frame(int page_number, int frame_num);
    void save_page_to_disk(int frame_num);
    int  get_frame_for_index(long long index);

    // HashMap helpers
    int  hash_slot(int page_number) const;
    void hash_insert(int page_number, int frame);
    void hash_remove(int page_number);

    // Calcula el primo más cercano >= n (para el tamaño del HashMap)
    static int next_prime(int n);

public:

    PagedArray(const char* file_path, int p_size, int p_count);
    ~PagedArray();

    long long get_total_elements() const { return total_elements; }

    Proxy operator[](long long index);

    int get_page_hits()   const { return page_hits; }
    int get_page_faults() const { return page_faults; }

    friend class Proxy;
};

#endif