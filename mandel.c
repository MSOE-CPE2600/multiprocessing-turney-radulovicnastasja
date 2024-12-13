/******************************************************************************************************************
* @file mandel.c
* @brief Mandelbrot generation with multithreading
*
* Course: CPE2600
* Section: 111
* Assignment: CPE Lab 12
*
* Description: This program generates a Mandelbrot image and supports multithreading.
* Date: 12/06/2024
*
* Compile Instructions:
*   To compile this file, use:
*   - gcc -o mandel mandel.c jpegrw.c -lpthread -ljpeg
******************************************************************************************************************/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"

// Function prototypes
static int iteration_to_color(int i, int max);
static int iterations_at_point(double x, double y, int max);
static void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int thread_count);
static void show_help();

// Thread arguments structure
typedef struct {
    imgRawImage* img;
    double xmin, xmax, ymin, ymax;
    int max;
    int thread_id;
    int total_threads;
} ThreadArgs;

// Compute a portion of the image
void* compute_image_region(void* args) {
    ThreadArgs* targs = (ThreadArgs*)args;
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

// Main function
int main(int argc, char* argv[]) {
    char c;
    const char* outfile = "mandel.jpg";
    double xcenter = 0, ycenter = 0, xscale = 4, yscale = 0;
    int image_width = 1000, image_height = 1000, max = 1000, thread_count = 1;

    // Parse command-line arguments
    while ((c = getopt(argc, argv, "x:y:s:W:H:m:o:t:h")) != -1) {
        switch (c) {
            case 'x': xcenter = atof(optarg); break;
            case 'y': ycenter = atof(optarg); break;
            case 's': xscale = atof(optarg); break;
            case 'W': image_width = atoi(optarg); break;
            case 'H': image_height = atoi(optarg); break;
            case 'm': max = atoi(optarg); break;
            case 'o': outfile = optarg; break;
            case 't': thread_count = atoi(optarg); break;
            case 'h': show_help(); exit(1);
            default:
                fprintf(stderr, "Unknown option: %c\n", c);
                show_help();
                return 1;
        }
    }

    // Validate thread count
    if (thread_count < 1 || thread_count > 20) {
        fprintf(stderr, "Error: Thread count must be between 1 and 20.\n");
        return 1;
    }

    // Calculate yscale based on xscale and image dimensions
    yscale = xscale / image_width * image_height;

    // Display the configuration
    printf("mandel: x=%lf y=%lf xscale=%lf yscale=%lf max=%d threads=%d outfile=%s\n",
           xcenter, ycenter, xscale, yscale, max, thread_count, outfile);

    // Create and initialize the image
    imgRawImage* img = initRawImage(image_width, image_height);
    setImageCOLOR(img, 0); // Fill with black

    // Compute the Mandelbrot image
    compute_image(img, xcenter - xscale / 2, xcenter + xscale / 2,
                  ycenter - yscale / 2, ycenter + yscale / 2, max, thread_count);

    // Save the image to the specified file
    storeJpegImageFile(img, outfile);

    // Free allocated memory
    freeRawImage(img);

    return 0;
}

// Compute an entire Mandelbrot image using multithreading
void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int thread_count) {
    pthread_t threads[thread_count];
    ThreadArgs targs[thread_count];

    for (int t = 0; t < thread_count; t++) {
        targs[t] = (ThreadArgs){img, xmin, xmax, ymin, ymax, max, t, thread_count};
        pthread_create(&threads[t], NULL, compute_image_region, &targs[t]);
    }

    for (int t = 0; t < thread_count; t++) {
        pthread_join(threads[t], NULL);
    }
}

// Determine the number of iterations at a point in Mandelbrot space
int iterations_at_point(double x, double y, int max) {
    double x0 = x, y0 = y;
    int iter = 0;

    while ((x * x + y * y <= 4) && iter < max) {
        double xt = x * x - y * y + x0;
        double yt = 2 * x * y + y0;
        x = xt;
        y = yt;
        iter++;
    }

    return iter;
}

// Convert iteration number to color
int iteration_to_color(int iters, int max) {
    return 0xFFFFFF * iters / max;
}

// Show help message
void show_help() {
    printf("Usage: mandel [options]\n");
    printf("Options:\n");
    printf("  -m <max>    Maximum number of iterations per point (default=1000)\n");
    printf("  -x <coord>  X coordinate of image center (default=0)\n");
    printf("  -y <coord>  Y coordinate of image center (default=0)\n");
    printf("  -s <scale>  Scale of the image in Mandelbrot coordinates (X-axis, default=4)\n");
    printf("  -W <pixels> Width of the image in pixels (default=1000)\n");
    printf("  -H <pixels> Height of the image in pixels (default=1000)\n");
    printf("  -o <file>   Output file name (default=mandel.jpg)\n");
    printf("  -t <threads> Number of threads to use (default=1, max=20)\n");
    printf("  -h          Show this help message\n");
}