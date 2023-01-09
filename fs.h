#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#define MAX_FILENAME_LENGTH         256
// ERRORS
#define FILE_NOT_FOUND_ON_DISK      1001
#define CANT_ACCESS_FILE            1002
#define MEMORY_ALLOC_ERROR          1003
#define NOT_ENOUGH_FREE_MEMORY      1004
#define FILE_WITH_THAT_NAME_EXISTS  1005
#define FREE_BLOCK_NOT_FOUND        1006

struct VirtualFile {
    char name[MAX_FILENAME_LENGTH];
    int size;
    int start_block;  // indeks pierwszego bloku zajętego przez plik
};

struct VirtualDisk {
    int size;
    int block_size;
    int num_blocks;
    int num_files;
    struct VirtualFile* files;
    char* data;  // dane dysku (dane plików zawartych w strukturach VirtualFile)
};

// tworzenie nowego dysku wirtualnego o zadanym rozmiarze i rozmiarze bloku
struct VirtualDisk* create_virtual_disk(int size, int block_size);

// kopiowanie pliku z dysku systemu na dysk wirtualny
int copy_file_to_virtual_disk(struct VirtualDisk* disk, const char* src_filename, const char* dest_filename);

// kopiowanie pliku z dysku wirtualnego na dysk systemu
int copy_file_from_virtual_disk(struct VirtualDisk* disk, const char* src_filename, const char* dest_filename);

// wyświetlanie zawartości katalogu dysku wirtualnego
void print_directory(const struct VirtualDisk* disk);

// usuwanie pliku z dysku wirtualnego
int remove_file_from_virtual_disk(struct VirtualDisk* disk, const char* filename);

// usuwanie dysku wirtualnego
void delete_virtual_disk(struct VirtualDisk* disk);

// wyświetlanie mapy zajętości dysku wirtualnego
void print_disk_map(const struct VirtualDisk* disk);

// zapisanie dysku wirtualnego do pliku
int save_virtual_disk(const struct VirtualDisk* disk, const char* filename);

// ładowanie dysku wirtualnego z pliku
struct VirtualDisk* load_virtual_disk(const char* filename);

// usuwanie dysku i zwalnianie pamięci (Na potrzeby testowania z poziomu programu)
void delete_virtual_disk(struct VirtualDisk* disk);

#endif  // FILE_SYSTEM_H
