#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

int init_serial(int *fd, const char *device){
    *fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY); 
    if (*fd < 0) { 
        perror("open"); 
        return -1; 
    }

    struct termios opt;

    if(tcgetattr(*fd, &opt) < 0)
        return -1; 
    /* define termois */
    opt.c_cflag |= (CLOCAL | CREAD);    // Set control mode state, local connection, receive enable
    opt.c_cflag &= ~CSIZE;              // Character length, make sure to screen out this bit before setting the data bit
    opt.c_cflag &= ~CRTSCTS;            // No hardware flow control
    opt.c_cflag |= CS8;                 // 8-bit data length
    opt.c_cflag &= ~CSTOPB;             // 1-bit stop bit
    opt.c_iflag |= IGNPAR;              // No parity bit
    opt.c_oflag = 0;                    // Output mode
    opt.c_lflag = 0;                    // No active terminal mode 

    if(cfsetispeed(&opt, B115200) < 0)  // Input baud rate
        return -1;
    if(cfsetospeed(&opt, B115200) < 0)  // Output baud rate
        return -1;
    if(tcflush(*fd, TCIFLUSH) < 0)      // Overflow data can be received, but not read
        return -1;
    if(tcsetattr(*fd, TCSANOW, &opt) < 0)
        return -1;

    return 0;
} 

int serial_send(int *fd, const char *data, size_t size){
    int ret = write(*fd, data, size);
    if( ret < 0 )
        tcflush(*fd, TCOFLUSH);
    return ret;
}

int serial_recv(int *fd, char *data, size_t size){
    int ret = 0;
    struct timeval tv = {1, 0};

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(*fd, &rfds);
    ret = select(*fd+1, &rfds, NULL, NULL, &tv);
    if (ret > 0){
        if(FD_ISSET(*fd,&rfds)){
            ret = read(*fd, data, size);
        }
    }
    else{
        perror("select()");
    }
    return ret;
}

int loopback(char *device){
    int fd;
    int ret;
    ret = init_serial(&fd, device);
    if(ret < 0)
        return ret;

    printf("Start uart loopback testing.\n");

    size_t size = 1024;                // Serial port buffer size generally defaults to 2k - 4k
    char *write_buf = (char*)malloc(size);
    char *read_buf = (char*)malloc(size);
    memset(write_buf, rand() % 26 + 65, size);
    memset(read_buf, 0, size);

    ret = serial_send(&fd, write_buf, size);
    printf("write : %d byte\n", ret);

    usleep(90000);                    // delay > 1024 / 115200 * 1000000 

    ret = serial_recv(&fd, read_buf, size);
    printf("Read  : %d byte\n", ret);

    ret = memcmp(read_buf,write_buf,size);
    if(ret != 0)
        printf("Test not pass : Inconsistent data\n");
    else
        printf("Test pass : Consistent data\n");

    free(write_buf);
    free(read_buf);
    close (fd);
    printf("Loopback test completed.\n");
    return 0;
}

int main(int argc, char *argv[]) {
    /* check input arguments */
    if (argc != 2) {
        printf("Usage: %s <device path>\n", argv[0]);
        return -1;
    }
    
    int ret = loopback(argv[1]);

    return ret;
}
