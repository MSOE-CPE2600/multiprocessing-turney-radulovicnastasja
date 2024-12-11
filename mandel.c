/// 
// Lab 12 Assignment 
// Nastasja Radulovic
// CPE 2600 121
// Date: 12/11/24
//
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <pthread.h>

typedef struct {
    int width;
    int height;
    unsigned char *data;  // Pointer to image data (e.g., pixel array)
} image_t;


// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image(imgRawImage *img, double x_min, double x_max, double y_min, double y_max, int max, int num_threads);
static void show_help();

typedef struct {
    imgRawImage *img;
    double xmin, xmax, ymin, ymax;
    int max;
    int thread_id;
    int total_threads;
} ThreadData;

// Compute a portion of the image
void* compute_image_region(void* args) {
    ThreadData* targs = (ThreadData*)args;
    int region_height = targs->img->height / targs->total_threads;
    int start_y = targs->thread_id * region_height;
    int end_y = (targs->thread_id == targs->total_threads - 1) ? targs->img->height : start_y + region_height;

    for (int j = start_y; j < end_y; j++) {
        for (int i = 0; i < targs->img->width; i++) {
            double x = targs->xmin + i * (targs->xmax - targs->xmin) / targs->img->width;
            double y = targs->ymin + j * (targs->ymax - targs->ymin) / targs->img->height;
            int iters = iterations_at_point(x, y, targs->max);
            setPixelCOLOR(targs->img, i, j, iteration_to_color(iters, targs->max));
        }
    }

    return NULL;
}

int main( int argc, char *argv[] )
{
	char c;

	// Default config values is no args given
	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; 
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;

	// For each command line argument given,
	// override the appropriate configuration value.

	int num_threads = 1; // Default to 1 thread

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Validate thread count
    if (num_threads < 1 || num_threads > 20) {
        fprintf(stderr, "Error: Thread count must be between 1 and 20.\n");
        return 1;
    }

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s\n",xcenter,ycenter,xscale,yscale,max,outfile);

	// Create a raw image of the appropriate size.
	imgRawImage* img = initRawImage(image_width,image_height);

	// Fill with black
	setImageCOLOR(img,0);

	// Compute the Mandelbrot image
	compute_image(img, xcenter - xscale / 2, xcenter + xscale / 2, ycenter - yscale / 2, ycenter + yscale / 2, max);

	// Save the image in file.
	storeJpegImageFile(img,outfile);

	// Free malloc
	freeRawImage(img);

	return 0;
}

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int thread_count) {
    pthread_t threads[num_threads];
    ThreadArgs targs[num_threads];

    for (int t = 0; t < num_threads; t++) {
        targs[t] = (ThreadArgs){img, xmin, xmax, ymin, ymax, max, t, num_threads};
        pthread_create(&threads[t], NULL, compute_image_region, &targs[t]);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
}

/*
Return num iterations at x,y to max
*/
int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}

// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}