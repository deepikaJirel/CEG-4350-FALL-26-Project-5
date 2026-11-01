#include "project4.h"

// FAT Copies
// First copy is fat0 stored at 
// Second copy is fat1
// There were issues declaring the FATs as non-pointers
// When they would get read from floppy, it would overwrite wrong areas of memory
fat_t *fat0;
fat_t *fat1;

directory_t currentDirectory;  // The current directory we have opened
directory_entry_t rootDirectoryEntry;   // The root directory's directory entry (this does not exist on the disk since the root is not inside of another directory)
file_t currentFile;            // The current file we have opened

#if PROJECT == 4
int kernel()
{
    startkernel(startshell);
    return 0;
}
#endif

int writeFATs()
{
    // Check that both FAT copies match
    for (int i = 0; i < FAT_ENTRIES_MAX; i++)
    {
        if (fat0->clusters[i] != fat1->clusters[i])
        {
            print("Error: FATs are not consistent!\n");
            return -1;
        }
    }

   // Write FAT 0 to sectors 1-11
    if (floppy_write(0, FAT0_SECTOR, (void *)fat0, FAT_SIZE))
    {
        print("Error: could not write FAT 0 to floppy!\n");
        return -2;
    }

    // Write FAT 1 to sectors 12-22
   if (floppy_write(0, FAT1_SECTOR, (void *)fat1, FAT_SIZE))
    {
        print("Error: could not write FAT 1 to floppy!\n");
        return -3;
    }

    return 0;
}

int writeCurrentDirectory()
{
    if (!currentDirectory.isOpened)
    {
        print("Error: cannot write directory! Current directory is not opened!\n");
        return -1;
    }
    if (currentDirectory.directoryEntry->startingCluster == 0)
    {
        if (floppy_write(0, ROOT_DIR_SECTOR,
                         (void *)currentDirectory.startingAddress,
                        ROOT_SIZE))
        {
            print("Error: could not write directory to floppy!\n");
            return -2;
        }
    }
    return 0;
}

int writeCurrentFile()
{
    if (!currentFile.isOpened)
    {
        print("Error: No file is currently open!\n");
        return -1;
    }

    uint16 nextCluster =
        currentFile.directoryEntry->startingCluster;

    uint16 clusterCount = 1;

    while (nextCluster != 0xFFFF)
    {
    uint16 sector = nextCluster + CLUSTER_TO_SECTOR;
        if (floppy_write(
            0,
            sector,
            currentFile.startingAddress + (ROOT_SIZE * (clusterCount - 1)),
                ROOT_SIZE))
                {
            print("Error: could not write file to floppy!\n");
            return -2;
        }
        
        nextCluster = fat0->clusters[nextCluster];

        clusterCount++;
    }
    return 0;
}

int resizeFile(uint16 startingCluster, uint32 newFileSize)
{

    // Calculate how many 512-byte clusters are needed
uint32 clustersRequired =
    (newFileSize + ROOT_SIZE - 1) / ROOT_SIZE;

if (clustersRequired < 1 || clustersRequired > FAT_ENTRIES_MAX)
{
    print("Error: Invalid new file size!\n");
    return -1;
}
    // Count how many clusters are already assigned
    uint32 clustersAssigned = 0;
    uint16 currentCluster = startingCluster;

    while (currentCluster != 0xFFFF)
    {
        clustersAssigned++;
        currentCluster = fat0->clusters[currentCluster];
    }

if (clustersRequired == clustersAssigned)
    {
        return 0;
    }
    else if (clustersRequired < clustersAssigned)
    {
            uint16 lastCluster = startingCluster;

    // Move to the cluster that should become the new last cluster
    for (uint32 i = 1; i < clustersRequired; i++)
    {
        lastCluster = fat0->clusters[lastCluster];
    }

    // Remember the first cluster that will be removed
    uint16 clusterToFree = fat0->clusters[lastCluster];

    // Mark the new last cluster as end-of-file
    fat0->clusters[lastCluster] = 0xFFFF;
    fat1->clusters[lastCluster] = 0xFFFF;

    // Free all clusters that are no longer needed
    while (clusterToFree != 0xFFFF)
    {
        uint16 nextCluster = fat0->clusters[clusterToFree];

        fat0->clusters[clusterToFree] = 0x0000;
        fat1->clusters[clusterToFree] = 0x0000;

        clusterToFree = nextCluster;
    }

    }
    else
{
    uint32 clustersNeeded =
        clustersRequired - clustersAssigned;

    uint16 lastCluster = startingCluster;

    while (fat0->clusters[lastCluster] != 0xFFFF)
    {
        lastCluster = fat0->clusters[lastCluster];
    }

    for (uint32 i = 0; i < clustersNeeded; i++)
    {
        uint16 newCluster = nextFreeCluster();

        if (newCluster == 0xFFFF)
        {
            return -2;
        }

        fat0->clusters[lastCluster] = newCluster;
        fat1->clusters[lastCluster] = newCluster;

        fat0->clusters[newCluster] = 0xFFFF;
        fat1->clusters[newCluster] = 0xFFFF;

        lastCluster = newCluster;
    }
}

    return writeFATs();
}