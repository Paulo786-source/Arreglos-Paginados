#ifndef PAGEDARRAY_H
#define PAGEDARRAY_H
#include <string>
#include <cstdio>

using namespace std;

class PagedArray
{
    // --- Clase interna Proxy --- //
    // Permite distinguir lectura de escritura en operator[]
    // Cuando solo se lee, NO activa dirty_bit.
    // Cuando se asigna (=), SÍ activa dirty_bit.
    class Proxy
    {
    private:
        PagedArray& arr;
        int frame;
        int pos_in_page;

    public:
        Proxy(PagedArray& a, int f, int p)
            : arr(a), frame(f), pos_in_page(p) {}

        // Lectura: convierte el Proxy a int sin marcar dirty
        operator int() const
        {
            return arr.data_frames[frame][pos_in_page];
        }

        // Escritura: asigna y marca dirty_bit = true
        Proxy& operator=(int value)
        {
            arr.data_frames[frame][pos_in_page] = value;
            arr.dirty_bit[frame] = true;
            return *this;
        }

        // Asignacion entre Proxies (necesario para swap entre dos elementos)
        Proxy& operator=(const Proxy& other)
        {
            return operator=((int)other);
        }
    };

    private:

    // --- Variables --- //

    // Variables de paginación
    int page_hits;
    int page_faults;

    // Manejo de archivo
    FILE* file;
    long long total_elements;

    // Configuración de paginación
    int page_size; // Cantidad de datos que caben en una página
    int page_count; // Cantidad de páginas que van a estar activas en RAM

    // Información RAM simulada
    int** data_frames; //Es un puntero doble, un arreglo de arreglos (Matriz)
    int* loaded_frames;
    int* last_used;
    bool* dirty_bit; // Indica si el frame fue modificado

    int time_counter; // Nos va a ayudar con el algoritmo LRU

    // --- Funciones --- //

    // Verifica si la página esta en la RAM
    int find_page_in_RAM(int page_number);

    // Nos indica cual fue el frame que menos se ha usado
    int find_lru_frame();

    // Carga una página del disco a la RAM
    void load_page_to_frame(int page_number, int frame_num);

    // Guarda los frames en el disco
    void save_page_to_disk(int frame_num);

    // Acceso interno al frame/pos (usado por Proxy)
    int get_frame_for_index(long long index);

    public:

    // --- Constructor y Destructor --- //

    PagedArray(const char* file_path, int p_size, int p_count);
    ~PagedArray();

    long long get_total_elements() const { return total_elements; }

    // --- operator[] devuelve un Proxy en lugar de int& --- //
    // Esto permite distinguir lectura de escritura
    Proxy operator[](long long index);

    // --- Getters para resultados finales --- //

    int get_page_hits()   const { return page_hits; }
    int get_page_faults() const { return page_faults; }

    // Necesario para que Proxy acceda a los internos
    friend class Proxy;
};

#endif