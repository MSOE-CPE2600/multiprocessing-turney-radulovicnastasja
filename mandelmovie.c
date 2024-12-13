/******************************************************************************************************************
* @file mandelmovie.c
* @brief Multiprocessing and multithreading for Mandelbrot movie generation
*
* Course: CPE2600
* Section: 111
* Assignment: CPE Lab 12
*
* Description: This program generates multiple frames of the Mandelbrot set using multiprocessing and multithreading.
* Date: 12/06/2024
*
* Compile Instructions:
*   To compile this file, use:
*   - gcc -o mandelmovie mandelmovie.c -lpthread
******************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

// Function prototypes
void help();

int main(int argc, char* argv[]) {
    int process_count = 1;  // Default number of processes
    int thread_count = 1;   // Default number of threads
    int frames = 50;        // Number of frames to generate
    char c;

    // Parse command-line arguments
    while ((c = getopt(argc, argv, "p:t:h")) != -1) {
        switch (c) {
            case 'p':
                process_count = atoi(optarg);
                if (process_count < 1) {
                    fprintf(stderr, "Error: Process count must be at least 1.\n");
                    exit(1);
                }
                break;
            case 't':
                thread_count = atoi(optarg);
                if (thread_count < 1 || thread_count > 20) {
                    fprintf(stderr, "Error: Thread count must be between 1 and 20.\n");
                    exit(1);
                }
                break;
            case 'h':
                help();
                exit(0);
            default:
                fprintf(stderr, "Invalid option. Use -h for help.\n");
                exit(1);
        }
    }

    printf("Creating Mandelbrot movie with %d processes and %d threads.\n", process_count, thread_count);

    int active_processes = 0;
    double xcenter = -0.088;  // X center of the Mandelbrot set
    double ycenter = 0.654;   // Y center of the Mandelbrot set
    double scale = 0.01;      // Starting scale

    for (int i = 0; i < frames; i++) {
        if (active_processes >= process_count) {
            wait(NULL);
            active_processes--;
        }

        pid_t pid = fork();
        if (pid == 0) {  // Child process
            char outfile[256];
            snprintf(outfile, sizeof(outfile), "mandel%03d.jpg", i);

            char x_arg[64], y_arg[64], scale_arg[64], max_arg[64], width_arg[64], height_arg[64], thread_arg[64], outfile_arg[512];
            snprintf(x_arg, sizeof(x_arg), "-x%lf", xcenter);
            snprintf(y_arg, sizeof(y_arg), "-y%lf", ycenter);
            snprintf(scale_arg, sizeof(scale_arg), "-s%lf", scale);
            snprintf(max_arg, sizeof(max_arg), "-m1000");
            snprintf(width_arg, sizeof(width_arg), "-W1000");
            snprintf(height_arg, sizeof(height_arg), "-H1000");
            snprintf(thread_arg, sizeof(thread_arg), "-t%d", thread_count);
            snprintf(outfile_arg, sizeof(outfile_arg), "-o%s", outfile);

            execl("./mandel", "mandel", x_arg, y_arg, scale_arg, max_arg, width_arg, height_arg, thread_arg, outfile_arg, (char*)NULL);

            // If execl fails
            perror("execl failed");
            exit(1);
        } else if (pid > 0) {  // Parent process
            active_processes++;
            scale *= 0.95;  // Reduce scale to zoom in
        } else {
            perror("fork failed");
            exit(1);
        }
    }

    while (active_processes > 0) {
        wait(NULL);
        active_processes--;
    }

    printf("All frames generated. Creating movie.\n");

    // Stitch frames into a movie using ffmpeg
    // Stitch frames into a movie using ffmpeg
    if (system("ffmpeg -i mandel%03d.jpg mandelmovie.mpg") != 0) {
        fprintf(stderr, "Failed to create movie using ffmpeg.\n");
        exit(1);
    }

    printf("Movie created: mandelmovie.mpg\n");
    return 0;
}

// Show help message
void help() {
    printf("Usage: mandelmovie [options]\n");
    printf("Options:\n");
    printf("  -p <process_count> Number of child processes to use (default=1)\n");
    printf("  -t <thread_count>  Number of threads to use per process (default=1, max=20)\n");
    printf("  -h                 Show this help message\n");
}