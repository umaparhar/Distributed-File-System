#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#include "messageStruct.h"

#define BUFFER_SIZE (4096 * 15)

struct sockaddr_in addrSnd, addrRcv;
int sd, rc;

 

int MFS_Init(char *hostname, int port) { // ID = 7
    sd = UDP_Open(20000); // Opens up a different port just in case we run these two in the same machine, which we will
    rc = UDP_FillSockAddr(&addrSnd, hostname, port);
    struct data data;
    data.id = 7;
    rc = UDP_Write(sd, &addrSnd, (void *) &data, BUFFER_SIZE);
    printf("init inside mfs init %s %d\n", hostname, port);
    return 0;
}

int MFS_Lookup(int pinum, char *name) { //0
    struct data data;
    data.id = 0;
    data.pinum = pinum;
    //data.name = name;
    sprintf(data.name, name);
    rc = UDP_Write(sd, &addrSnd, (void *) &data, BUFFER_SIZE);
    return 0;
}
int MFS_Stat(int inum, MFS_Stat_t *m) { //1
    struct data data;
    data.id = 1;
    data.inum = inum;
    data.type = m->type;
    data.size = m->size;
    rc = UDP_Write(sd, &addrSnd, (void *) &data, BUFFER_SIZE);
    return 0;
}
int MFS_Write(int inum, char *buffer, int block) { //2
    struct data data;
    data.id = 2;
    data.inum = inum;
    sprintf(data.buffer, buffer);
    //data.buffer = buffer;
    data.block = block;
    rc = UDP_Write(sd, &addrSnd, (void*)&data, BUFFER_SIZE);
    return 0;
}
int MFS_Read(int inum, char *buffer, int block) { //3
    struct data data;
    data.id = 3;
    data.inum = inum;
    sprintf(data.buffer, buffer);
    data.block = block;
    rc = UDP_Write(sd, &addrSnd, (void*)&data, BUFFER_SIZE);
    return 0;
}
int MFS_Creat(int pinum, int type, char *name) { //4
    struct data data;
    data.id = 4;
    data.pinum = pinum;
    data.type = type;
    sprintf(data.name, name); // Copyign the name of the file/dirctory to the struct
    rc = UDP_Write(sd, &addrSnd, (void*)&data, BUFFER_SIZE);
    return 0;
}
int MFS_Unlink(int pinum, char *name){ //5
    struct data data;
    data.id = 5;
    data.pinum = pinum;
    sprintf(data.name, name);
    rc = UDP_Write(sd, &addrSnd, (void*)&data, BUFFER_SIZE);
    return 0;
}
int MFS_Shutdown() {
    struct data data;
    data.id = 6;
    rc = UDP_Write(sd, &addrSnd, (void*)&data, BUFFER_SIZE);
    return 0;
}
