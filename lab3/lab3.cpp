#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/fb.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <pthread.h>

struct framebuffer_info
{
    uint32_t bits_per_pixel;    // depth of framebuffer
    uint32_t xres_virtual;      // how many pixel in a row in virtual screen
    uint32_t yres_virtual;
};

struct framebuffer_info get_framebuffer_info ( const char *framebuffer_device_path );

int print_flag = 0;
pthread_mutex_t mutex;

void *input_key(void *args){

	while(1){
		char input;
		input = getchar();

		if(input == 'c'){
			pthread_mutex_lock(&mutex);
			print_flag = 1;
			pthread_mutex_unlock(&mutex);
		}
	}

}

int main ( int argc, const char *argv[] )
{

    // variable to store the frame get from video stream
    cv::Mat frame;
    cv::Size2f frame_size;
    cv::Size2f first_frame_size;

    // open video stream device
    cv::VideoCapture camera (2);

    // get info of the framebuffer
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");

    // open the framebuffer device
    std::ofstream ofs("/dev/fb0");

    // check if video stream device is opened success or not
    if( !camera.isOpened() )
    {
         std::cerr << "Could not open video device." << std::endl;
         return 1;
    }

    // set propety of the frame
    bool first_frame = camera.read(frame);
    if(first_frame == false) return 0;
    first_frame_size = frame.size();

    
    camera.set(CV_CAP_PROP_FRAME_WIDTH, first_frame_size.width);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT,first_frame_size.height);

    //printf("frame size : \n");  
    //printf("height : %d, width : %d\n", first_frame_size.height, first_frame_size.width);

    //printf("frame buffer size :\n");
    //printf("height : %d, width : %d\n", fb_info.xres_virtual, fb_info.yres_virtual);
   
	int counter = 0;
    char filename[80];

	pthread_t tid;
	pthread_create(&tid, NULL, input_key, NULL);

    cv::VideoWriter writer("Video.avi", CV_FOURCC('M', 'J', 'P', 'G'), 25.0, cv::Size(640, 480));

    //int timer = 200;
    //while (timer)
    while(1)
    {
        
        bool ret = camera.read(frame);
        frame_size = frame.size();

		pthread_mutex_lock(&mutex);
		if(print_flag == 1){
        	sprintf(filename, "/run/media/mmcblk1p2/image_%d.png", counter);
        	cv::imwrite(filename, frame);
        	counter++;
		}
		print_flag = 0;
        pthread_mutex_unlock(&mutex);

        cvtColor(frame, frame, cv::COLOR_BGR2BGR565);
        
        for ( int y = 0; y < frame_size.height; y++ )
        {
            int pos = (fb_info.xres_virtual * y * fb_info.bits_per_pixel) / 8 + (fb_info.xres_virtual - frame_size.width)/2 * fb_info.bits_per_pixel / 8;
            ofs.seekp(pos);
            ofs.write((const char *)frame.ptr(y),(frame_size.width * fb_info.bits_per_pixel) / 8);
        }

        //timer--;

    }
    camera.release ();
    return 0;
}

struct framebuffer_info get_framebuffer_info ( const char *framebuffer_device_path )
{

    // get attributes of the framebuffer device thorugh linux system call "ioctl()"

    // put the required attributes in variable "fb_info" you found with "ioctl() and return it."

    struct framebuffer_info fb_info;  // Used to return the required attrs.
    struct fb_var_screeninfo screen_info;  // Used to get attributes of the device from OS kernel.
    int fbfd = -1;
    fbfd = open(framebuffer_device_path, O_RDWR);
    if(!fbfd){
    	printf("cannot not open framebuffer device ? \n");
    	exit(-1);
    }
    ioctl(fbfd, FBIOGET_VSCREENINFO, &screen_info);
    fb_info.xres_virtual = screen_info.xres_virtual;
    fb_info.yres_virtual = screen_info.yres_virtual;
    fb_info.bits_per_pixel = screen_info.bits_per_pixel;

    return fb_info;
};
