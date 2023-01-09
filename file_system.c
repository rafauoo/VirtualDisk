#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Zwraca 1, jeśli blok o zadanym indeksie jest wolny, lub 0 w przeciwnym razie.
int is_block_free(const struct VirtualDisk* disk, int index) {
    return disk->data[index * disk->block_size] == 0;
}

// Zwraca ilość wolnego miejsca na dysku.
long get_available_space(const struct VirtualDisk* disk)
{
    long available_size = 0;
    for (int i = 0; i < disk->num_blocks; i++)
    {
        if (is_block_free(disk, i))
        {
        available_size += disk->block_size;
        }
    }
    return available_size;
}

// Zwraca indeks pierwszego wolnego bloku o wystarczającym rozmiarze lub -1, jeśli taki blok nie istnieje.
int get_free_block(const struct VirtualDisk* disk, long size)
{
    int block_index = -1;
    long free_size = 0;
    for (int i = 0; i < disk->num_blocks; i++) {
        if (is_block_free(disk, i)) {
            if (block_index == -1)
                block_index = i;
            
            free_size += disk->block_size;

            if (free_size >= size)
                return block_index;
        }
        else {
            block_index = -1;
            free_size = 0;
        }
    }
    return -1;
}

// Oznaczenie bloków o zadanym zakresie jako zajęte.
void mark_blocks_as_used(struct VirtualDisk* disk, int start_index, long size)
{
    for (long i = start_index * disk->block_size; i < start_index * disk->block_size + size; i++)
        disk->data[i] = 1;
}

// Oznaczenie bloków o zadanym zakresie jako wolne.
void mark_blocks_as_free(struct VirtualDisk* disk, int start_index, long size)
{
    for (long i = start_index * disk->block_size; i < start_index * disk->block_size + size; i++)
        disk->data[i] = 0;
}

// Tworzenie nowego dysku o zadanych rozmiarze i rozmiarze bloku.
// Zwraca wskaźnik na utworzony dysk lub NULL, jeśli nie udało się zaalokować pamięci.
struct VirtualDisk* create_virtual_disk(int size, int block_size)
{
    struct VirtualDisk* disk = malloc(sizeof(struct VirtualDisk));
    if (disk == NULL) return NULL;

    disk->size = size;
    disk->block_size = block_size;
    disk->num_blocks = size / block_size;
    disk->data = malloc(size);
    
    if (disk->data == NULL) {
        free(disk);
        return NULL;
    }

    disk->num_files = 0;
    disk->files = NULL;

    return disk;
}

// Usuwanie dysku i zwalnianie pamięci.
void delete_virtual_disk(struct VirtualDisk* disk)
{
    if (disk == NULL) return;

    if (disk->files != NULL) free(disk->files);
    
    free(disk->data);
    free(disk);
}

// Wczytywanie dysku z pliku o zadanej nazwie.
// Zwraca wskaźnik na wczytany dysk lub NULL, jeśli wystąpił błąd.
struct VirtualDisk* load_virtual_disk(const char* filename)
{
    // otwarcie pliku
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) return NULL;

    // odczytanie rozmiaru i rozmiaru bloku
    int size, block_size;
    fread(&size, sizeof(int), 1, fp);
    fread(&block_size, sizeof(int), 1, fp);

    // utworzenie nowego dysku
    struct VirtualDisk* disk = create_virtual_disk(size, block_size);
    if (disk == NULL) {
        fclose(fp);
        return NULL;
    }

    // odczytanie liczby plików
    fread(&disk->num_files, sizeof(int), 1, fp);

    // odczytanie plików
    disk->files = malloc(disk->num_files * sizeof(struct VirtualFile));

    if (disk->files == NULL) {
        delete_virtual_disk(disk);
        fclose(fp);
        return NULL;
    }

    fread(disk->files, sizeof(struct VirtualFile), disk->num_files, fp);

    // odczytanie danych bloków danych
    fread(disk->data, 1, size, fp);

    // zamknięcie pliku
    fclose(fp);

    return disk;
}
// Zapisywanie dysku do pliku o zadanej nazwie.
// Zwraca 0 w przypadku powodzenia lub wartość różną od 0, jeśli wystąpił błąd.
int save_virtual_disk(const struct VirtualDisk* disk, const char* filename)
{
    // otwarcie pliku
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) return 1;

    // zapisanie rozmiaru i rozmiaru bloku
    fwrite(&disk->size, sizeof(disk->size), 1, fp);
    fwrite(&disk->block_size, sizeof(disk->block_size), 1, fp);

    // zapisanie liczby plików
    fwrite(&disk->num_files, sizeof(disk->num_files), 1, fp);

    // zapisanie plików
    fwrite(disk->files, sizeof(struct VirtualFile), disk->num_files, fp);

    // zapisanie danych bloków danych
    fwrite(disk->data, 1, disk->size, fp);

    // zamknięcie pliku
    fclose(fp);

    return 0;
}

// Dodawanie pliku o zadanej nazwie i rozmiarze do dysku.
// Zwraca 0 w przypadku powodzenia lub wartość różną od 0, jeśli wystąpił błąd.
int add_file_to_virtual_disk(struct VirtualDisk* disk, const char* filename, long size, const char* data)
{
    // sprawdzenie, czy dysk ma wystarczająco dużo miejsca
    long available_size = get_available_space(disk);
    if (size > available_size) {
        return NOT_ENOUGH_FREE_MEMORY;
    }

    // sprawdzenie, czy nazwa pliku jest unikalna
    for (int i = 0; i < disk->num_files; i++)
    {
        if (strcmp(disk->files[i].name, filename) == 0) {
        return FILE_WITH_THAT_NAME_EXISTS;
        }
    }
    // zwiększenie liczby plików na dysku
    disk->num_files++;

    // przydzielenie nowej tablicy plików
    struct VirtualFile* new_files = realloc(disk->files, disk->num_files * sizeof(struct VirtualFile));
    if (new_files == NULL){
        disk->num_files--;
        return MEMORY_ALLOC_ERROR;
    }
    disk->files = new_files;

    // szukanie wolnego bloku o wystarczającym rozmiarze
    int start_block = get_free_block(disk, size);
    if (start_block == -1) {
        return FREE_BLOCK_NOT_FOUND;
    }

    // zaznaczenie bloków jako zajęte
    mark_blocks_as_used(disk, start_block, size);

    // dodanie pliku do katalogu
    disk->num_files--;
    strcpy(disk->files[disk->num_files].name, filename);
    disk->files[disk->num_files].size = size;
    disk->files[disk->num_files].start_block = start_block;
    disk->num_files++;

    // skopiowanie danych do dysku wirtualnego
    memcpy(disk->data + start_block * disk->block_size, data, size);
    return 0;
}

// Usuwanie pliku o zadanej nazwie z dysku.
// Zwraca 0 w przypadku powodzenia lub wartość różną od 0, jeśli wystąpił błąd.
int remove_file_from_virtual_disk(struct VirtualDisk* disk, const char* filename)
{
    // wyszukanie pliku o podanej nazwie
    int file_index = -1;
    for (int i = 0; i < disk->num_files; i++) {
        if (strcmp(disk->files[i].name, filename) == 0) {
            file_index = i;
            break;
        }
    }
    if (file_index == -1) return FILE_NOT_FOUND_ON_DISK;

    // odblokowanie bloków danych zajętych przez plik
    mark_blocks_as_free(disk, disk->files[file_index].start_block, disk->files[file_index].size);

    // usunięcie pliku z tablicy plików
    for (int i = file_index; i < disk->num_files - 1; i++)
        disk->files[i] = disk->files[i + 1];

    disk->num_files--;

    // zmniejszenie tablicy plików
    struct VirtualFile* new_files = realloc(disk->files, disk->num_files * sizeof(struct VirtualFile));
    if (new_files != NULL || disk->num_files == 0) disk->files = new_files;

    return 0;
}

// Wyświetlanie zawartości katalogu dysku.
void print_directory(const struct VirtualDisk* disk)
{
    printf("Zawartość katalogu:\n");
    for (int i = 0; i < disk->num_files; i++)
        printf(" %s (%d B) [na dysku (%d B)]\n", disk->files[i].name, disk->files[i].size,
        (disk->files[i].size / disk->block_size + 1) * disk->block_size);
}

// Wyświetlanie mapy zajętości dysku.
void print_disk_map(const struct VirtualDisk* disk)
{
    printf("Mapa zajętości dysku:\n");
    for (int i = 0; i < disk->num_blocks; i++)
    {
        int occupied = 0;
        for (int j = 0; j < disk->block_size; j++) {
            if (disk->data[i*disk->block_size+j] != 0) {
                occupied++;
            }
        }
        printf(" Blok %d: ", i);
        if (is_block_free(disk, i)) {
            printf("wolny (%d B) / (%d B)", occupied, disk->block_size);
        }
        else {
            printf("zajęty (%d B) / (%d B)", occupied, disk->block_size);
        }
        printf("\n");
    }
    int occ_blocks = 0;
    for (int i = 0; i < disk->num_blocks; i++) {
        if (is_block_free(disk, i) == 0) {
            occ_blocks++;
        }
    }
    printf("Zajęte miejsce na dysku: %d B / %d B\n", occ_blocks * disk->block_size, disk->size);
}

// Kopiowanie pliku z dysku systemu Minix do dysku wirtualnego.
// Zwraca 0 w przypadku powodzenia lub wartość różną od 0, jeśli wystąpił błąd.
int copy_file_to_virtual_disk(struct VirtualDisk* disk, const char* src_filename, const char* dest_filename)
{
    // otwarcie pliku źródłowego
    FILE* src_fp = fopen(src_filename, "r");
    if (src_fp == NULL) {
        return CANT_ACCESS_FILE;
    }

    // odczytanie rozmiaru pliku
    fseek(src_fp, 0, SEEK_END);
    long size = ftell(src_fp);
    fseek(src_fp, 0, SEEK_SET);

    // alokacja pamięci na dane pliku
    char* data = malloc(size);
    if (data == NULL) {
        fclose(src_fp);
        return MEMORY_ALLOC_ERROR;
    }

    // odczyt danych z pliku
    fread(data, size, 1, src_fp);
    fclose(src_fp);

    // dodanie pliku do dysku wirtualnego
    int result = add_file_to_virtual_disk(disk, dest_filename, size, data);
    free(data);
    return result;
}

int copy_file_from_virtual_disk(struct VirtualDisk* disk, const char* src_filename, const char* dest_filename)
{
    // szukanie pliku o podanej nazwie w katalogu dysku wirtualnego
    int file_index = -1;
    for (int i = 0; i < disk->num_files; i++)
    {
        if (strcmp(disk->files[i].name, src_filename) == 0)
        {
        file_index = i;
        break;
        }
    }
    if (file_index == -1)
    {
        // plik o podanej nazwie nie został znaleziony w katalogu
        return FILE_NOT_FOUND_ON_DISK;
    }

    // otwarcie pliku docelowego do zapisu
    FILE* dest_fp = fopen(dest_filename, "w");
    if (dest_fp == NULL)
    {
        return CANT_ACCESS_FILE;
    }

    // odczyt danych pliku z dysku wirtualnego
    const struct VirtualFile* file = &disk->files[file_index];
    const char* data = disk->data + file->start_block * disk->block_size;

    // zapis danych do pliku docelowego
    fwrite(data, file->size, 1, dest_fp);
    fclose(dest_fp);
    return 0;
}


// int main () {
//     //struct VirtualDisk* disk = create_virtual_disk(1024, 16);
//     struct VirtualDisk* disk = load_virtual_disk("dysk.vd");
//     //print_directory(disk);
//     copy_file_to_virtual_disk(disk, "halo.txt", "nowehalo3.txt");
//     print_disk_map(disk);
//     save_virtual_disk(disk, "dysk.vd");
//     //print_directory(disk);
// }