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
        long long index; // CORRECCIÓN: Guardamos el índice, no el frame.

    public:
        Proxy(PagedArray& a, long long idx) : arr(a), index(idx) {}

        // Lectura: Busca el frame justo en el momento de leer
        operator int() const
        {
            int frame = arr.get_frame_for_index(index);
            return arr.data_frames[frame][index % arr.page_size];
        }

        // Escritura: Busca el frame justo en el momento de escribir
        Proxy& operator=(int value)
        {
            int frame = arr.get_frame_for_index(index);
            arr.data_frames[frame][index % arr.page_size] = value;
            arr.dirty_bit[frame] = true;
            return *this;
        }

        // Asignacion entre Proxies (Vital para el swap arr[i] = arr[j])
        Proxy& operator=(const Proxy& other)
        {
            int val = (int)other; // Extrae el valor primero de forma segura
            return operator=(val); // Lo escribe después
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