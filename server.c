#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "messageStruct.h"

// Represents the max number of bytes the server can recieve
#define BUFFER_SIZE (4096 * 15) 
#define END_OF_LOG_PTR 1024

int currInum = 1;

// server code
int main(int argc, char *argv[]) {
    int sd = UDP_Open(15000);
    assert(sd > -1);
    while (1) {
        struct sockaddr_in addr;
        
        int fd;
        struct data data;
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &addr, (void *) &data, BUFFER_SIZE);
        
        if(data.id == 1) {
            printf("looking up file, pinum is: %d", data.inum);

        } else if (data.id == 2) { // MFS_write
            lseek(fd, 0, SEEK_SET);
            write(fd, data.buffer, 40);

        } else if (data.id == 3) { // MFS_Read
            lseek(fd, 0, SEEK_SET);
            read(fd, data.buffer, 40);
            printf("data: %s\n", data.buffer);

        } else if (data.id == 4) { // MFS_Create
            // Steps required for this method
            // Find the parent
                // We have the parent inode number
                
                int parSegNum = (int) data.pinum / 16; // Calculating which segment parent inode pointer is in
                lseek(fd, parSegNum * sizeof(int), SEEK_SET); // Seeking to the correct index in checkpoint region

                int parSegAddr;
                read(fd, &parSegAddr, 4); // Reading the address of the imap segment that the parent's inode pointer is in
                lseek(fd, parSegAddr, SEEK_SET);
                
                // Reading the 16 pointers in the inode map's segment
                int inodeMap[16];
                for (int j = 0; j < 16; j++) {
                    read(fd, &inodeMap[j], sizeof(int));
                }
                int parInodeAddr = inodeMap[data.pinum % 16]; // parInodeAddr has the address of the parent's inode
                
                lseek(fd, parInodeAddr, SEEK_SET); // Lseeking to beginning of parent inode
                struct __MFS_Stat_t parInode;
                read(fd, &parInode, sizeof(parInode));

                // Reading in the 14 direct pointers in the parent inode
                int directPointers[14];
                int toChangePtrInInode = -1; // Represents the index of the data block's direct pointer we'll be rewriting
                for (int j = 0; j < 14; j++)
                {
                    read(fd, &directPointers[j], sizeof(int));
                    if (directPointers[j] == -1 && toChangePtrInInode == -1)
                    {
                        toChangePtrInInode = j - 1;
                    }
                    
                }
                lseek(fd, directPointers[toChangePtrInInode], SEEK_SET); // Seeking to appropriate data block of parent

                // Each data block that belongs to a directory will hold 128 directory entries
                struct __MFS_DirEnt_t dirEntries[128];
                int firstEmptyDirEntryIdx = -1; // Will hold the index value of the first empty directory entry (This is where the new child will go)
                for (int j = 0; j < 128; j++)
                {
                    read(fd, &dirEntries[j], sizeof(struct __MFS_DirEnt_t));
                    if (dirEntries[j].inum == -1 && firstEmptyDirEntryIdx == -1)
                    {
                        firstEmptyDirEntryIdx = j;
                    }
                }

                dirEntries[firstEmptyDirEntryIdx].inum = currInum;
                strcpy(dirEntries[firstEmptyDirEntryIdx].name, data.name);

                // Read the current value of the pointer that represents the end of our LFS
                lseek(fd, END_OF_LOG_PTR, SEEK_SET);
                int ptr;
                read(fd, &ptr, sizeof(int));

                // This is where the new data block for the parent will go
                directPointers[toChangePtrInInode] = ptr;
                inodeMap[data.pinum % 16] = ptr + 4096; // Location of parent's new inode
                
                // Seeking to current end of LFS
                lseek(fd, ptr, SEEK_SET);
                write(fd, dirEntries, sizeof(dirEntries));
                write(fd, &parInode, sizeof(parInode));
                write(fd, directPointers, sizeof(directPointers));
                write(fd, inodeMap, sizeof(inodeMap));
                ptr += 4096 + 64 + 64;


                // fix checkpoint region after updating just the parent stuff
                lseek(fd, parSegNum * sizeof(int), SEEK_SET);
                int newImapAddr = ptr - 64;
                write(fd, &newImapAddr, sizeof(int));

                lseek(fd, END_OF_LOG_PTR, SEEK_SET);
                write(fd, &ptr, sizeof(int));


                // NOW WORKING ON CHILD DATA BLOCK, INODE, AND ITS IMAP SEGMENT


                // build data block for new empty dir
                sprintf(dirEntries[0].name, ".."); // parent
                dirEntries[0].inum = data.pinum;
                sprintf(dirEntries[1].name, "."); // itself
                dirEntries[1].inum = currInum;

                struct __MFS_DirEnt_t blank;
                blank.inum = -1;
                for (int i = 2; i < 128; i++)
                {
                    dirEntries[i] = blank;
                }  
                
                //build inode for new dir
                struct __MFS_Stat_t newInode;
                newInode.size = 4095;
                newInode.type = MFS_DIRECTORY;

                directPointers[0] = ptr;
                for (int j = 1; j < 14; j++)
                {
                    directPointers[j] = -1;
                }
                

                // build inode map for new dir
                int childSegNum = (int) currInum / 16;
                lseek(fd, childSegNum * sizeof(int), SEEK_SET); // Going to the child inode's segment's index in the checkpoint region
                
                int childSegAddr;
                read(fd, &childSegAddr, sizeof(int));
                lseek(fd, childSegAddr, SEEK_SET);

                for(int j = 0; j < 16; j++){
                    read(fd, &inodeMap[j], sizeof(int));
                }
                inodeMap[currInum % 16] = ptr + 4096; 

                lseek(fd, ptr, SEEK_SET);
                write(fd, dirEntries, sizeof(dirEntries));
                write(fd, &newInode, sizeof(newInode));
                write(fd, directPointers, sizeof(directPointers));
                write(fd, inodeMap, sizeof(inodeMap));
                ptr += 4096 + 64 + 64;

                // update checkpoint region with the new address of the imap segment
                lseek(fd, childSegNum * sizeof(int), SEEK_SET);
                newImapAddr = ptr - 64;
                write(fd, &newImapAddr, sizeof(int));

                lseek(fd, END_OF_LOG_PTR, SEEK_SET);
                write(fd, &ptr, sizeof(int));

                currInum++;

                //MF TEST TIMEEE
                // printf("\n");
                // lseek(fd, 22148, SEEK_SET);
                // for(int y = 0; y < 6; y++){
                //     struct __MFS_DirEnt_t te;
                //     read(fd, &te, sizeof(te));
                //     printf("name: %s | inum: %d\n", te.name, te.inum);
                // }
                // printf("\n");
                printf("\n");
                int addrOfImapSeg;
                lseek(fd, 0, SEEK_SET);
                read(fd, &addrOfImapSeg, sizeof(int));
                printf("The addrOfImapSeg is %d bytes\n", addrOfImapSeg);

                lseek(fd, addrOfImapSeg, SEEK_SET);
                int test;
                for(int i = 0; i < 4; i++) {
                    read(fd, &test, sizeof(int));
                    printf("%d\n", test);
                }
                printf("\n");
                //END TEST   

                // Use checkpoint region to find the segment of the inode map that contains the pointer to the parent's inode
                // Go to the checkpoint region, find which of the 256 segments points to the segment that parent's inode pointer is stored in
                // Go to that inode map segment. Find the specific index in the inode map that the pointer to the parent's inode is stored in
                // Go to the parent's inode
                // We now have pointers to data blocks that contain the parent directory's entries
                // Use size of directory to find the direct pointer of the last valid (has space for a new entry) data block
                // Go to that data block and use size where to write new directory entry
                // Create a __MFS_DirEnt_t struct containing the inum of the new file/directory and set the name of the new file/directory
                // Rewrite the datablock for the parent directory, so that it includes the information for our new directory struct
                // Rewrite the inode for our parent directory so that it points to the new datablock
                // Rewrite the entire segment(all 16 indices) of the inode map that contains the pointer to the parent directory's inode
                // Update the checkpoint region so that the pointer to this inode map segment is now pointing to the just written inode map segment

                // Check if newly created file/directory is a file or directory
                // Assign inum to new child
                // if directory:
                    // Write a data block for new directory that contains two entries. One for itself, and one for its parent. . = child's inum, .. = parent's inum
                    // Write the inode that points to the newly written block for that directory.
                    // Write the inode map segment that contains the updated pointer to the newly written inode.
                    // Update the checkpoint region so that the pointer to this inode map segment is now pointing to the just written inode map segment
                // if file:
                    // 
                    
            // Check if its a file or directory

        } else if (data.id == 5) {

        } else if (data.id == 6) {
            printf("shutting down file system.");

        } else if (data.id == 7) { // MFS_Init
            printf("Initializing MFS; Creating root directory.");
            fd = open("img", O_RDWR);

            // Writing the address of the first inode map segment to checkpoint region
            int a = 5188;
            lseek(fd, 0, SEEK_SET);
            write(fd, &a, 4);
            
            // Writing the address for the current end of log pointer
            a = 5252;
            lseek(fd, 1024, SEEK_SET);
            write(fd, &a, 4);
            
            // Reading the end of log pointer
            int b;
            lseek(fd, 1024, SEEK_SET);
            read(fd, &b, 4);

            // printf("End of log ptr value is: %d", b);
            // Writing the two entries in the root's datablock
            struct __MFS_DirEnt_t rootParent;
            sprintf(rootParent.name, "..");
            rootParent.inum = 0;

            struct __MFS_DirEnt_t root;
            sprintf(root.name, ".");
            root.inum = 0;

            lseek(fd, 1028, SEEK_SET);
            write(fd, &rootParent, sizeof(rootParent));
            write(fd, &root, sizeof(root));

            struct __MFS_DirEnt_t blank;
            blank.inum = -1;
            for (int i = 2; i < 128; i++)
            {
                write(fd, &blank, sizeof(blank));
            }            

            // Writing inode for root directory
            struct __MFS_Stat_t rootInode;
            //rootInode.size = 2 * sizeof(struct __MFS_DirEnt_t)
            rootInode.size = 4095;
            rootInode.type = MFS_DIRECTORY;
            lseek(fd, 5124, SEEK_SET);
            write(fd, &rootInode, sizeof(rootInode));
            a = 1028; // First direct pointer for root inode
            write(fd, &a, sizeof(int));
            a = -1;
            for (int i = 1; i < 14; i++) {
                write(fd, &a, sizeof(int));
            }

            // Writing the first segment of the inode map 
            lseek(fd, 5188, SEEK_SET);
            a = 5124;
            write(fd, &a, sizeof(int));
        }


        if (rc > 0) {
            char reply[BUFFER_SIZE];
            sprintf(reply, "gbyestr");
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
            printf("server:: reply\n");
        } 
    }
    return 0; 
}
