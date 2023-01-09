#include "fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


// Help - available usages of command.
void print_help() {
        printf("Available usages:\n");
        printf("  vd create <disk_name> <size> <block_size> - Creates a virtual disk with given disk name, size (B) & block size (B)\n");
        printf("  vd delete <disk_name> - Deletes virtual disk that is called disk_name.vd\n");
        printf("  vd rm <disk_name> <file_name> - Removes file from disk\n");
        printf("  vd todisk <disk_name> <src_file_name> <dest_file_name> - Copies file from outside the disk to disk\n");
        printf("  vd fromdisk <disk_name> <src_file_name> <dest_file_name> - Copies file from disk to system\n");
        printf("  vd ls <disk_name> - Prints disk directory contents\n");
        printf("  vd blocks <disk_name> - Prints disk block map\n");
}

int main (int argc, char *argv[]) {
    if (argc < 2)
    {
        print_help();
        return 0;
    }


    // Creating Virtual Disk.
    if (strcmp(argv[1], "create") == 0) {
        // checking if argc is not too big
        if (argc < 5) {
            print_help();
            return 0;
        }
        // checking if size and block size are digits
        if (!isdigit(*argv[3]) || !isdigit(*argv[4])) {
            printf("Size and Block size have to be numbers!\n");
            return 0;
        }
        // checking if size % block size equals 0
        if (atoi(argv[3]) % atoi(argv[4]) != 0) {
            printf("Size has to be divisable by block size!\n");
            return 0;
        }
        // creating virutal disk
        struct VirtualDisk* disk = create_virtual_disk(atoi(argv[3]), atoi(argv[4]));
        if (disk == NULL) {
            printf("Error allocating memory!\n");
            return 0;
        }
        save_virtual_disk(disk, strcat(argv[2], ".vd"));
        printf("Disk created!\n");
        return 0;
    }

    // Deleting Virtual Disk.
    if (strcmp(argv[1], "delete") == 0) {
        // checking if file ends with .vd
        if (argv[2][strlen(argv[2])-3] != '.' ||
            argv[2][strlen(argv[2])-2] != 'v' || 
            argv[2][strlen(argv[2])-1] != 'd')
        {
            printf("Can't delete non-disk file!\n");
            return 0;
        }
        // confirmation for deletion
        printf("Are you sure? You won't be able to revert deletion!\n");
        printf("Type 'YES' to confirm: ");
        char choice[100];
        scanf("%s", choice);
        if (strcmp(choice, "YES") == 0) {
            if (remove(argv[2]) == 0)
                printf("Virtual disk has been deleted!\n");
            else
                printf("Unable to delete the disk! Maybe disk with that name doesn't exist?\n");
        return 0;
        }
        // user did not confirm deletion
        printf("Aborting...\n");
        return 0;
    }

    // Removing file from Virtual Disk.
    if (strcmp(argv[1], "rm") == 0) {
        // loading disk from file
        struct VirtualDisk* disk = load_virtual_disk(argv[2]);
        if (disk == NULL) {
            printf("Wrong disk name!\n");
            return 0;
        }
        // error handling
        switch (remove_file_from_virtual_disk(disk, argv[3]))
        {
        case FILE_NOT_FOUND_ON_DISK:
            printf("ERROR: File with that name (%s) not found on virtual disk!\n", argv[3]);
            break;
        default:
            // saving disk if no errors occured
            save_virtual_disk(disk, argv[2]);
            printf("File removed successfully!\n");
            break;
        }
        return 0;
    }

    // Copying file from system to Virtual Disk.
    if (strcmp(argv[1], "todisk") == 0) {
        // loading disk from file
        struct VirtualDisk* disk = load_virtual_disk(argv[2]);
        if (disk == NULL) {
            printf("Wrong disk name!\n");
            return 0;
        }
        // error handling
        switch (copy_file_to_virtual_disk(disk, argv[3], argv[4]))
        {
        case CANT_ACCESS_FILE:
            printf("ERROR: File with name %s doesn't exist!\n", argv[3]);
            break;
        case MEMORY_ALLOC_ERROR:
            printf("ERROR: Can't allocate memory for data!\n");
            break;
        case NOT_ENOUGH_FREE_MEMORY:
            printf("ERROR: Not enough free memory on disk to copy this file!\n");
            break;
        case FILE_WITH_THAT_NAME_EXISTS:
            printf("ERROR: Source file with that name (%s) already exists!\n", argv[4]);
            break;
        case FREE_BLOCK_NOT_FOUND:
            printf("ERROR: No free blocks of memory found on disk!\n");
            break;
        default:
            // saving disk if no errors occured
            save_virtual_disk(disk, argv[2]);
            printf("File copied successfully!\n");
            break;
        }
        return 0;
    }

    // Copying file from Virtual Disk to system.
    if (strcmp(argv[1], "fromdisk") == 0) {
        // loading disk from file
        struct VirtualDisk* disk = load_virtual_disk(argv[2]);
        if (disk == NULL) {
            printf("Wrong disk name!\n");
            return 0;
        }
        // error handling
        switch (copy_file_from_virtual_disk(disk, argv[3], argv[4]))
        {
        case FILE_NOT_FOUND_ON_DISK:
            printf("ERROR: File with that name doesn't exist on virtual disk!\n");
            break;
        case CANT_ACCESS_FILE:
            printf("ERROR: Can't access output file (%s)!\n", argv[4]);
            break;
        default:
            // saving disk if no errors occured
            save_virtual_disk(disk, argv[2]);
            printf("File copied successfully!\n");
            break;
        }
        return 0;
    }

    // Showing contents of Virtual Disk.
    if (strcmp(argv[1], "ls") == 0) {
        // loading disk from file
        struct VirtualDisk* disk = load_virtual_disk(argv[2]);
        if (disk == NULL) {
            printf("Wrong disk name!\n");
            return 0;
        }
        // printing contents of disk
        print_directory(disk);
        return 0;
    }

    // Showing Virtual Disk memory map.
    if (strcmp(argv[1], "blocks") == 0) {
        // loading disk from file
        struct VirtualDisk* disk = load_virtual_disk(argv[2]);
        if (disk == NULL) {
            printf("Wrong disk name!\n");
            return 0;
        }
        // printing map
        print_disk_map(disk);
        return 0;
    }

    // printing help if no command matched
    print_help();
    return 0;
}