#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define INTERVAL_USEC 1000 // delay time

/* send data */
int can_write(int s, struct can_frame frame) {
    int i;
    int nbytes;
    
    // send frame
    nbytes = write(s, &frame, sizeof(struct can_frame));
    if(nbytes != sizeof(struct can_frame)){
        printf("can: write frame err,wrote %d bytes\n", nbytes);
        return -1;
    }

    // print datas
    printf("ID:0x%02x  DATA:  ", frame.can_id);
    for (i = 0; i < frame.can_dlc; i++) {
	printf("0x%02x ", frame.data[i]);
    }
    printf("\n");
    return 0;
}

/* recv data */
int can_read(int s, struct can_frame frame) {
    int i;
    int nbytes ;

    // read frame
    nbytes  = read(s, &frame, sizeof(frame));
    if(nbytes != sizeof(struct can_frame)){
        printf("can: read frame err,read %d bytes\n", nbytes);
        return -1;
    }

    // print datas
    printf("ID:0x%02x  DATA:  ", frame.can_id);
    for (i = 0; i < frame.can_dlc; i++) {
        printf("0x%02x ", frame.data[i]);
    }
    printf("\n");
    return 0;
}


int main(int argc, char *argv[]) {
    int frame_cnt = 10;
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    const char *ifname = argv[1];

    if (argc < 3 || argc > 4) {
        printf("please check your input.\n"
               "usage: %s <dev> <read/write> <frame count>\n"
               "       default frame count is 10\n\n"
               "eg: %s can0 read 10\n", argv[0], argv[0]);
        return -1;
    }
    if(argc == 4)
        frame_cnt = atoi(argv[3]);
        
	
    // init can raw socket
    if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Error while opening socket");
        return -1;
    }
	
    // get interface index
    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr);
	
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

    // bind the socket with etn
    if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error in socket bind");
        close(s);
        return -1;
    }
	
    // init test frame 
    struct can_frame frame = { 
        .can_id = 0x123,
        .can_dlc = 8,
        .data = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}
    };
	
    // send data
    if (!strcmp(argv[2], "write")){
        while(frame_cnt --){
            can_write(s, frame);
            usleep(INTERVAL_USEC);
        }
    }

    // recv data
    else if (!strcmp(argv[2], "read")){
        while(frame_cnt --){
            can_read(s, frame);
            usleep(INTERVAL_USEC);
        }
    }

    else { 
        printf("argument err\n");
        close (s);
        return -1;
    }
	
    close (s);
    return 0;
}
