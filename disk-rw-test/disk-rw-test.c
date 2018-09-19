#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>

static int file_write(char *file, char *buf, size_t size){
    int fd = -1;
    int ret = -1;

    fd = open(file, O_WRONLY | O_CREAT | O_TRUNC);
    if( fd < 0 ){
        printf("File write: open failed\n");
        return fd;
    }

    ret = write(fd, buf, size);
    printf("write success:%d\n",ret);
    if( ret < 0 ){
        printf("File write: write failed\n");
        return ret;
    }

    close(fd);
    return 0;
} 

static int file_read(char *file, char *buf, size_t size){
    int fd = -1;
    int ret = -1;

    fd = open(file, O_RDONLY);
    if( fd < 0 ){
        printf("File read: open failed\n");
        return fd;
    }

    ret = read(fd, buf, size);
    printf("read success:%d\n", ret);
    if( ret < 0 ){
        printf("File read: read failed\n");
        return ret;
    }
    
    close(fd);
    return 0;
} 
 
int main(int argc, char *argv[]) { 
    if( argc != 2 ){
        printf("usage:\n"
               "    eg: %s /media/sdb1/\n"
               "    Size of test data is 10M\n",basename(argv[0]));
        return -1;
    }

    int ret = -1;

    /* Composite file path */
    char filename[100];
    strcpy(filename,argv[1]);
    strcat(filename, "disk_rw_test.tmp");

    size_t size = 10 * 1024 * 1024;

    /* Allocate buffer memory space */
    char *w_buf = (char*)malloc(size);
    char *r_buf = (char*)malloc(size);

    /* Initialization buffer */
    memset(w_buf, rand() % 96 + 32, size);
    memset(r_buf, 0, size);

    ret = file_write(filename, w_buf, size);
    if( ret < 0 )
        return ret;

    ret = file_read(filename, r_buf, size);
    if( ret < 0 ){
        free(r_buf);
        return ret;
    }

    printf("compared size: %d\n", size);
    ret = memcmp(w_buf, r_buf, size);
    if( ret == 0 )
        printf("The comparison data are the same.\n");
    else
        printf("There are differences in comparison data.\n");

    /* Release */
    free(w_buf);
    free(r_buf);
    exit(0); 
} 
