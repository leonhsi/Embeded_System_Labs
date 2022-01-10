#include <fcntl.h> 
#include <fstream>
#include <linux/fb.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
struct framebuffer_info
{
    uint32_t bits_per_pixel;    // framebuffer depth
    uint32_t xres_virtual;      // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path);

int main(int argc, const char *argv[])
{
    cv::Mat image1, image2, image3;
    cv::Size2f image_size;
    
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");

    // read image file (sample.bmp) from opencv libs.
    // https://docs.opencv.org/3.4.7/d4/da8/group__imgcodecs.html#ga288b8b3da0892bd651fce07b3bbd3a56
    // image = .......
    image1 = imread("/home/embedsky/lab4/01.png", cv::IMREAD_COLOR);
    image2 = imread("/home/embedsky/lab4/02.png", cv::IMREAD_COLOR);
    image3 = imread("/home/embedsky/lab4/03.png", cv::IMREAD_COLOR);

    // get image size of the image.
    // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a146f8e8dda07d1365a575ab83d9828d1
    // image_size = ......
    image_size = image1.size();

    // transfer color space from BGR to BGR565 (16-bit image) to fit the requirement of the LCD
    // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga397ae87e1288a81d2363b61574eb8cab
    // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga4e0972be5de079fed4e3a10e24ef5ef0
    cv::Mat framebuffer_compat1, framebuffer_compat2, framebuffer_compat3;
    cv::cvtColor(image1, framebuffer_compat1, cv::COLOR_BGR2BGR565);
    cv::cvtColor(image2, framebuffer_compat2, cv::COLOR_BGR2BGR565);
    cv::cvtColor(image3, framebuffer_compat3, cv::COLOR_BGR2BGR565);

    // output to framebufer row by row
    int framebuffer_width = fb_info.xres_virtual;
    int offset = 0, total_offset = 0, stop_point = 0;

    while(1){
        for (int y = 0; y < image_size.height; y++)
        {
            switch(total_offset/framebuffer_width){
                case 0:
                    ofs.seekp(y*framebuffer_width*2 + offset*2);
                    ofs.write(reinterpret_cast<char*>(framebuffer_compat1.ptr(y)),(image_size.width-offset)*2);
                    ofs.seekp(y*framebuffer_width*2);
                    ofs.write(reinterpret_cast<char*>(framebuffer_compat2.ptr(y))+(int)(image_size.width-offset)*2,offset*2);
                    break;
                case 1:
                    ofs.seekp(y*framebuffer_width*2 + offset*2);
                    ofs.write(reinterpret_cast<char*>(framebuffer_compat2.ptr(y)),(image_size.width-offset)*2);
                    ofs.seekp(y*framebuffer_width*2);
                    ofs.write(reinterpret_cast<char*>(framebuffer_compat3.ptr(y))+(int)(image_size.width-offset)*2,offset*2);
                    break;
                case 2:
                    ofs.seekp(y*framebuffer_width*2 + offset*2);
                    ofs.write(reinterpret_cast<char*>(framebuffer_compat3.ptr(y)),(image_size.width-offset)*2);
                    ofs.seekp(y*framebuffer_width*2);
                    ofs.write(reinterpret_cast<char*>(framebuffer_compat1.ptr(y))+(int)(image_size.width-offset)*2,offset*2);
                    break;
            }

        }
        offset = (offset+72)%(framebuffer_width);
        total_offset = (total_offset+72)%(framebuffer_width*3);
        stop_point += 72;
        usleep(200);
        if (stop_point >= 1280*6)
            break;
    }

    return 0;
}

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path)
{
    struct framebuffer_info fb_info;        // Used to return the required attrs.
    struct fb_var_screeninfo screen_info;   // Used to get attributes of the device from OS kernel.

    // open device with linux system call "open()"
    // https://man7.org/linux/man-pages/man2/open.2.html
    int fd = open(framebuffer_device_path, O_RDWR);

    // get attributes of the framebuffer device thorugh linux system call "ioctl()".
    // the command you would need is "FBIOGET_VSCREENINFO"
    // https://man7.org/linux/man-pages/man2/ioctl.2.html
    // https://www.kernel.org/doc/Documentation/fb/api.txt
    ioctl(fd, FBIOGET_VSCREENINFO, &screen_info);
    // put the required attributes in variable "fb_info" you found with "ioctl() and return it."
    fb_info.xres_virtual = screen_info.xres_virtual;      // 8
    fb_info.bits_per_pixel = screen_info.bits_per_pixel;    // 16

    return fb_info;
};