#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define MAX (4096 * 14)

struct data {
    int id;
    int pinum;
    char name[128]; // Name the of the file
    int inum;
    
    int size;
    int type;

    char buffer[MAX];
    int block;

};

// struct test {
//     char message[MAX];
//     int id;
//     char reply[MAX];
// };

// struct lookup {
//     int id;
//     int pinum;
//     char name[MAX];
// };

// struct st {
//     int id;
//     int inum;
//     int type;
//     int size;    
//     //MFS_Stat_t *m;
// };

// struct rw {
//     int id;
//     int inum;
//     char buffer[MAX];
//     int block;
// };

// struct create {
//     int id;
//     int pinum;
//     int type;
//     char name[MAX];
// };

// struct unlink {
//     int id;
//     int pinum;
//     char name[MAX];
// };

// struct shut {
//     int id;
// };
