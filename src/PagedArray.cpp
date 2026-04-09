#include "PagedArray.h"
#include <iostream>

int PagedArray::next_prime(int n)
{
    if (n <= 2) return 2;
    if (n % 2 == 0) n++;
    while (true)
    {
        bool es_primo = true;
        for (int i = 3; i * i <= n; i += 2)
            if (n % i == 0) { es_primo = false; break; }
        if (es_primo) return n;
        n += 2;
    }
}

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

    map_size = next_prime(page_count * 2 + 1);
    hash_keys = new int[map_size];
    hash_values = new int[map_size];

    for (int i = 0; i < map_size; i++)
    {
        hash_keys[i] = EMPTY_SLOT;
        hash_values[i] = EMPTY_SLOT;
    }
}

PagedArray::~PagedArray()
{
    for (int i = 0; i < page_count; i++)
        if (dirty_bit[i] && loaded_frames[i] != EMPTY_SLOT)
            save_page_to_disk(i);

    if (file != nullptr) { fflush(file); fclose(file); }

    for (int i = 0; i < page_count; i++) delete[] data_frames[i];
    delete[] data_frames;
    delete[] loaded_frames;
    delete[] last_used;
    delete[] dirty_bit;
    delete[] hash_keys;
    delete[] hash_values;
}

int PagedArray::hash_slot(int page_number) const
{
    return page_number % map_size;
}

void PagedArray::hash_insert(int page_number, int frame)
{
    int slot = hash_slot(page_number);
    while (hash_keys[slot] != EMPTY_SLOT && hash_keys[slot] != page_number)
        slot = (slot + 1) % map_size;
    hash_keys[slot] = page_number;
    hash_values[slot] = frame;
}

void PagedArray::hash_remove(int page_number)
{
    int slot = hash_slot(page_number);
    while (hash_keys[slot] != EMPTY_SLOT && hash_keys[slot] != page_number)
        slot = (slot + 1) % map_size;
    if (hash_keys[slot] == EMPTY_SLOT) return;

    hash_keys[slot] = EMPTY_SLOT;
    hash_values[slot] = EMPTY_SLOT;

    int next = (slot + 1) % map_size;
    while (hash_keys[next] != EMPTY_SLOT)
    {
        int k = hash_keys[next], v = hash_values[next];
        hash_keys[next] = EMPTY_SLOT;
        hash_values[next] = EMPTY_SLOT;
        hash_insert(k, v);
        next = (next + 1) % map_size;
    }
}

int PagedArray::find_page_in_RAM(int page_number)
{
    int slot = hash_slot(page_number);
    while (hash_keys[slot] != EMPTY_SLOT)
    {
        if (hash_keys[slot] == page_number) return hash_values[slot];
        slot = (slot + 1) % map_size;
    }
    return -1;
}

int PagedArray::find_lru_frame()
{
    for (int i = 0; i < page_count; i++)
        if (loaded_frames[i] == EMPTY_SLOT) return i;

    int min_val = last_used[0], min_idx = 0;
    for (int i = 1; i < page_count; i++)
        if (last_used[i] < min_val) { min_val = last_used[i]; min_idx = i; }
    return min_idx;
}

void PagedArray::load_page_to_frame(int page_num, int frame_num)
{
    long long byte_offset = (long long)page_num * page_size * sizeof(int);
#ifdef _WIN32
    _fseeki64(file, byte_offset, SEEK_SET);
#else
    fseeko(file, (off_t)byte_offset, SEEK_SET);
#endif
    size_t leidos = fread(data_frames[frame_num], sizeof(int), page_size, file);
    for (size_t k = leidos; k < (size_t)page_size; k++)
        data_frames[frame_num][k] = 0;

    loaded_frames[frame_num] = page_num;
    dirty_bit[frame_num] = false;
    hash_insert(page_num, frame_num);
}

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