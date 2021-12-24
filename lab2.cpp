#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

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
    cv::Mat image;
    cv::Size2f image_size;
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");
    image = imread(argv[1], cv::IMREAD_COLOR);
    image_size = image.size();
    
    std::cout << image.size() << std::endl;
    cvtColor(image, image, cv::COLOR_BGR2BGR565);

    // output to framebufer row by row
    for (int y = 0; y < image_size.height; y++) {
        int pos = (fb_info.xres_virtual * y * fb_info.bits_per_pixel) / 8;
        ofs.seekp(pos);
        ofs.write(
            (const char *)image.ptr(y),
            (image_size.width * fb_info.bits_per_pixel) / 8);
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
