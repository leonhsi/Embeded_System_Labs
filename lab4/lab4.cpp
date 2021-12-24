#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

struct framebuffer_info {
    uint32_t bits_per_pixel;  // framebuffer depth
    uint32_t xres_virtual;    // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info(
    const char *framebuffer_device_path);

int main(int argc, const char *argv[]) {
    cv::Mat image1,image2,image3;
    cv::Size2f image1_size,image2_size,image3_size;
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");
    image1 = imread("/run/media/mmcblk1p2/bonus.png", cv::IMREAD_COLOR);
    image2 = imread("/run/media/mmcblk1p2/b.png", cv::IMREAD_COLOR);
    image3 = imread("/run/media/mmcblk1p2/sample.bmp", cv::IMREAD_COLOR);
    image1_size = image1.size();
    image2_size = image2.size();
    image3_size = image3.size();
    
    std::cout << image1.size() << std::endl;
    cvtColor(image1, image1, cv::COLOR_BGR2BGR565);
    cvtColor(image2, image2, cv::COLOR_BGR2BGR565);
    cvtColor(image3, image3, cv::COLOR_BGR2BGR565);
    int count = 0;

    while(1){
        // output to framebufer row by row
        //if(count == 0){    
            for (int y = 0; y < image1_size.height; y++) {
                int pos = (fb_info.xres_virtual * y * fb_info.bits_per_pixel) / 8;
                ofs.seekp(pos);
                ofs.write(
                    (const char *)image1.ptr(y),
                    (image1_size.width * fb_info.bits_per_pixel) / 8);
            }
     //   }
        sleep(5);
     //   else if(count == 200){
            for (int y = 0; y < image2_size.height; y++) {
                int pos = (fb_info.xres_virtual * y * fb_info.bits_per_pixel) / 8;
                ofs.seekp(pos);
                ofs.write(
                    (const char *)image2.ptr(y),
                    (image2_size.width * fb_info.bits_per_pixel) / 8);
            }
      //  }
        sleep(5);
      // else{
            for (int y = 0; y < image3_size.height; y++) {
                int pos = (fb_info.xres_virtual * y * fb_info.bits_per_pixel) / 8;
                ofs.seekp(pos);
                ofs.write(
                    (const char *)image3.ptr(y),
                    (image3_size.width * fb_info.bits_per_pixel) / 8);
            }
        sleep(5);
      //  }
       // count++;
    }

    return 0;
}

struct framebuffer_info get_framebuffer_info(
    const char *framebuffer_device_path) {
    struct framebuffer_info fb_info;  // Used to return the required attrs.
    struct fb_var_screeninfo
        screen_info;  // Used to get attributes of the device from OS kernel.
    int fbfd = -1;
    fbfd = open(framebuffer_device_path, O_RDWR);
    if(!fbfd){
    	printf("cannot not open framebuffer device ? \n");
    	exit(-1);
    }
    ioctl(fbfd, FBIOGET_VSCREENINFO, &screen_info);
    fb_info.xres_virtual = screen_info.xres_virtual;
    fb_info.bits_per_pixel = screen_info.bits_per_pixel;

    return fb_info;
};

