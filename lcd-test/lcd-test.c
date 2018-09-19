#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

typedef struct {
    char red;
    char green;
    char blue;
    char trans;
} PIXEL;  

void drawPixel(char *fbp, unsigned long location, PIXEL *pixel, int bbp){
    unsigned short rgb = 0;
    unsigned int r = 0, g = 0, b = 0;
    switch(bbp){
        case 32:
            *(fbp + location) = pixel->red; 
            *(fbp + location + 1) = pixel->green;
            *(fbp + location + 2) = pixel->blue;
            *(fbp + location + 3) = pixel->trans; 
            break;  
        case 16:
            r = (pixel->red*0x1f)/0xff;
            g = (pixel->green*0x3f)/0xff;
            b = (pixel->blue*0x1f)/0xff;
            rgb = (r << 11) | (g << 5) | b;
            *((unsigned short*)(fbp + location)) = rgb;
            break;
        default:
            printf("No fit BBP\n");
            break;
    }   
}

int main(int argc, char *argv[]){
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    unsigned long location = 0;
    char *fbp = 0;
    int x = 0, y = 0;

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (!fbfd) {
        printf("Error: cannot open framebuffer device.\n");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("Error reading fixed information.\n");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Error reading variable information.\n");
        exit(3);
    }
    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Map the device to memory
    fbp = (char *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        printf("Error: failed to map framebuffer device to memory.\n");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

    // Display test
    PIXEL pixel = { 0x00, 0x00, 0x00, 0x00 };

    int section = vinfo.yres/8;
    int factor = 0;
    int i = 3;
	
    while(i--){
        for ( y = 0; y < vinfo.yres; y++ ){ 
            factor = y / section;
            if( i%2 == 0 )
                factor = 7 - factor; 
            for ( x = 0; x < vinfo.xres; x++ ) {
                location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length; 
                switch (factor){
                    case 0:
                        pixel.red = 0x00;
                        pixel.green = 0x00;
                        pixel.blue = 0x00;
                        pixel.trans = 0x00;
                        break;
                    case 1:
                        pixel.red = 0xff;
                        pixel.green = 0x00;
                        pixel.blue = 0x00;
                        pixel.trans = 0x00;
                        break;
                    case 2:
                        pixel.red = 0xff;
                        pixel.green = 0x00;
                        pixel.blue = 0xff;
                        pixel.trans = 0x00;
                        break;
                    case 3:
                        pixel.red = 0xff;
                        pixel.green = 0xff;
                        pixel.blue = 0x00;
                        pixel.trans = 0x00;
                        break;
                    case 4:
                        pixel.red = 0x00;
                        pixel.green = 0xff;
                        pixel.blue = 0x00;
                        pixel.trans = 0x00;
                        break;
                    case 5:
                        pixel.red = 0x00;
                        pixel.green = 0xff;
                        pixel.blue = 0xff;
                        pixel.trans = 0x00;
                        break;
                    case 6:
                        pixel.red = 0x00;
                        pixel.green = 0x00;
                        pixel.blue = 0xff;
                        pixel.trans = 0x00;
                        break;
                    case 7:
                        pixel.red = 0xff;
                        pixel.green = 0xff;
                        pixel.blue = 0xff;
                        pixel.trans = 0x00;
                        break;
                    default:
                        break;
                }
                drawPixel(fbp, location, &pixel, vinfo.bits_per_pixel);
            }
        }
        sleep(2);
    }

    // Release Framebuffer
    munmap(fbp, finfo.smem_len);
    close(fbfd);
    return 0;
}


