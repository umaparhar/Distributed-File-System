#include <stdio.h>
//#include "udp.h"

#include "mfs.h"

int main(int argc, char *argv[]) {
    int a = MFS_Init("localhost", 15000);
    a = MFS_Creat(0, MFS_DIRECTORY, "foo");
    a = MFS_Creat(1, MFS_DIRECTORY, "boo");
    a = MFS_Creat(1, MFS_DIRECTORY, "uma");
    // a = MFS_Read(1, "", 2);

    // a = MFS_Creat(0, MFS_REGULAR_FILE, 'yo.txt');
    // a = MFS_Creat(0, MFS_DIRECTORY, 'folder');
    return a;
}
