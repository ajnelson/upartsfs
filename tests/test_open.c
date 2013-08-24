#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv) {
    char* file_path;
    char* mode_string;
    int file_descriptor;
    int mode_flag;

    if (argc < 3) {
        fprintf(stderr, "test_open: Must have a mode (r,w,rw) and file path.\n");
        exit(1);
    }

    mode_string = argv[1];
    file_path = argv[2];

    if (! strncmp("rw", mode_string, 2)) {
        mode_flag = O_RDWR;
    } else if (! strncmp("r", mode_string, 1)) {
        mode_flag = O_RDONLY;
    } else if (! strncmp("w", mode_string, 1)) {
        mode_flag = O_WRONLY;
    } else {
        fprintf(stderr, "test_open: Unexpected mode ('%s').  Must be in (r,w,rw).\n", mode_string);
        exit(1);
    }
   
    file_descriptor = open(file_path, mode_flag);
    if (file_descriptor == -1)
        return -errno;

    return 0;
}
