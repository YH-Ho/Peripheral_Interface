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

int serial_send(int *fd, const char *data, size_t size){
    int ret = write(*fd, data, size);
    if( ret < 0 ){
        perror("write");
        tcflush(*fd, TCOFLUSH);
    }
    else
        printf("send: %s\nsize: %d\n",data ,size);

    return ret;
}

int serial_recv(int *fd, char *data, size_t size){
    int ret = 0;
    size_t read_left = size;
    size_t read_size = 0;
    char *read_ptr = data;

    memset(data, 0, size);

    fd_set rfds;
    while(1){
        struct timeval tv = {5, 0};

        FD_ZERO(&rfds);
        FD_SET(*fd, &rfds);
        ret = select(*fd+1, &rfds, NULL, NULL, &tv);
        
        if (ret > 0){
            if(FD_ISSET(*fd,&rfds)){
                read_size = read(*fd, read_ptr, read_left);

		if(read_size == 0)
                    break;

	        read_ptr += read_size;
                read_left -= read_size;
            }
        }
        else if(ret == 0){
            printf("exit in 5 seconds without recv data!\n");
            break;
	}
        else{
            perror("select()");
            break;
	}   
    }
    printf("recv: %s\nsize: %d\n",data, strlen(data));
    return strlen(data);
}

int main(int argc, char *argv[]) {
    int fd;
    size_t read_size;
    size_t write_size;
    char *read_buf;
    struct termios opt;

    if(argc != 4){
        printf("please check your input.\n");
        printf("Usage: %s <device path> <read> <size>\n"
               "       %s <device path> <write> <string>\n", argv[0] ,argv[0]);
        return -1;
    }

    if(!strcmp(argv[2], "read")){
        read_size = atoi(argv[3]);
        read_buf = (char*)malloc(read_size);
    }
    
    else if(!strcmp(argv[2], "write"))
        write_size = strlen(argv[3]);
	
    // open serial uart1
    if ((fd = open(argv[1], O_RDWR)) < 0) {
        perror("open");
        return -1;
    }
	
    // define termois
    if(tcgetattr(fd, &opt) < 0)
        return -1; 

    opt.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
    opt.c_oflag  &= ~OPOST;

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
    if(tcflush(fd, TCIFLUSH) < 0)       // Overflow data can be received, but not read
        return -1;
    if(tcsetattr(fd, TCSANOW, &opt) < 0)
        return -1;

    // recv data
    if (!strcmp(argv[2], "read"))
        serial_recv(&fd, read_buf, read_size);

    // send data
    else if (!strcmp(argv[2], "write")) 
        serial_send(&fd, argv[3], write_size);

    else 
        printf("please check the third argument\n");
	
    if(!strcmp(argv[2], "read"))
        free(read_buf);

    close (fd);

    return 0;
}
