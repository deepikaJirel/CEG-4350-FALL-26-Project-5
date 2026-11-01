#include "project5.h"

#if PROJECT == 5
int kernel()
{
    startkernel(startshell);
    return 0;
}
#endif


int createSubdirectory(char *subdirectoryName)
{
    // Step i: Checks if the currentDirectory is opened
    if (!currentDirectory.isOpened)
    {
        print("Error: current directory is not opened!\n");// 1. If currentDirectory is not opened, print an error message, return non-zero value
        return -1;
    }

    // Step ii: Find the next free directory entry inside of the currentDirectory
    directory_entry_t *newSubdirectoryDirEntry =
        (directory_entry_t *)currentDirectory.startingAddress;

    while (newSubdirectoryDirEntry->filename[0] != 0)
    {
        newSubdirectoryDirEntry++;
    }

    // Step iii: Check to see if the directory that you are creating this
// subdirectory in needs to be resized
    uint32 usedBytes =
        (uint8 *)newSubdirectoryDirEntry -
        currentDirectory.startingAddress;

    if (usedBytes >= currentDirectory.directoryEntry->fileSize)
    {
        // Root directory cannot be resized
        if (currentDirectory.directoryEntry->startingCluster == 0)
        {
            print("Error: root directory is full!\n");
            return -2;
        }

        // Increase directory by one 512-byte cluster
        if (resizeFile(
                currentDirectory.directoryEntry->startingCluster,
                currentDirectory.directoryEntry->fileSize + 512))
        {
            print("Error: could not resize directory!\n");
            return -3;
        }

        currentDirectory.directoryEntry->fileSize += 512;
    }

    // Step iv: Find the next free cluster in the FAT and mark it as EOF
    uint16 next = nextFreeCluster();

    if (next == 0xFFFF)
    {
        print("Error: no free cluster available!\n");
        return -4;
    }

    fat0->clusters[next] = 0xFFFF;
    fat1->clusters[next] = 0xFFFF;

    if (writeFATs())
    {
        print("Error: could not write FATs!\n");
        return -5;
    }

    // Step v: Using the free entry found in step ii
    stringcopy(subdirectoryName,
            newSubdirectoryDirEntry->filename,
            8);

    stringcopy("   ",
            newSubdirectoryDirEntry->ext,
            3);

    newSubdirectoryDirEntry->attributes = 0x10;
    newSubdirectoryDirEntry->startingCluster = next;
    newSubdirectoryDirEntry->fileSize = 512;

    if (writeCurrentDirectory())
    {
        print("Error: could not write current directory!\n");
        return -6;
    }

    // Step vi: Create a buffer for the new sector for the subdirectory
    // and add "." and ".."
    uint8 buffer[512] = {0};

    // "." = this directory itself
    directory_entry_t *selfEntry =
        (directory_entry_t *)buffer;

    *selfEntry = *newSubdirectoryDirEntry;

    stringcopy(".       ",
            selfEntry->filename,
            8);

    stringcopy("   ",
            selfEntry->ext,
            3);

    // ".." = parent directory
    directory_entry_t *parentEntry =
        ((directory_entry_t *)buffer) + 1;

    *parentEntry = *currentDirectory.directoryEntry;

    stringcopy("..      ",
            parentEntry->filename,
            8);

    stringcopy("   ",
            parentEntry->ext,
            3);

    // Write the new directory sector to disk
    if (floppy_write(
            0,
            next + CLUSTER_TO_SECTOR,
            (void *)buffer,
            512))
    {
        print("Error: could not write subdirectory!\n");
        return -7;
    }

    return 0;
}


int openDirectory(char *directoryName)
{
    // Step i: Check if the currentDirectory is opened
    if (!currentDirectory.isOpened)
    {
        print("Error: current directory is not opened!\n");
        return -1;
    }

    // Step ii: Make sure no file is currently opened
    if (currentFile.isOpened)
    {
        print("Error: a file is currently open!\n");
        return -2;
    }

    // Step iii: Find the directory entry that has the name directoryName
    directory_entry_t *currentEntry =
        (directory_entry_t *)currentDirectory.startingAddress;

    while (currentEntry->filename[0] != 0)
    {
        if (stringcompare(
                currentEntry->filename,
                directoryName,
                8) &&
            (currentEntry->attributes & 0x10))
        {
            break;
        }

        currentEntry++;
    }

    if (currentEntry->filename[0] == 0)
    {
        print("Error: subdirectory could not be found!\n");
        return -3;
    }

    // Step iv: if we are opening the root directory
// (i.e.: directoryEntry->startingCluster == 0)

if (currentEntry->startingCluster == 0)
{
    // 1. Set currentDirectory.directoryEntry to root directory entry
    currentDirectory.directoryEntry = &rootDirectoryEntry;

    // 2. Read the root directory from the disk
    int readFailed = floppy_read(
        0,
        ROOT_DIR_SECTOR,
        (void *)currentDirectory.startingAddress,
        ROOT_SIZE
    );

    if (readFailed)
    {
        print("Error: could not read root directory!\n");// print an error message
        return -4; // return a non-zero error value
    }
}

else
{
    // 1.  Load the directory off of the disk by traversing the FATs
// from the directory entry's startingCluster found in step iii
uint16 nextCluster = currentEntry->startingCluster;
uint32 index = 0;

while (nextCluster != 0xFFFF)
{
    // floppy_read one 512-byte cluster
    // if read failed → return error
int readFailed = floppy_read(
    0,
    nextCluster + CLUSTER_TO_SECTOR,
    (void *)(currentDirectory.startingAddress + (512 * index)),
    512
);
if (readFailed)
{
    print("Error: could not read directory!\n");
    return -5;
}
    // go to next FAT cluster
    nextCluster = fat0->clusters[nextCluster];

    // move to next 512-byte slot in memory
    index++;
}


    directory_entry_t *selfEntry =
        (directory_entry_t *)currentDirectory.startingAddress;

while (selfEntry->filename[0] != 0)
{
    if (stringcompare(
            selfEntry->filename,
            ".       ",
            8))
    {
        break;
    }

    selfEntry++;
}

currentDirectory.directoryEntry = selfEntry;
}


// vi. Return 0 if no errors have occurred
return 0;
}