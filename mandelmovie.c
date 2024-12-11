/*
 * Lab 12 Assignment
 * Nastasja Radulovic
 * CPE 2600 121
 * Date: 12/11/24
 *
 * Description:
 * This program generates a sequence of Mandelbrot set images by 
 * creating and managing child processes and threads. It uses command-line options 
 * to specify the number of processes and threads and dynamically scales the images 
 * for creating a movie effect. Each frame is saved as a separate JPEG file. 
 * The program can be extended to combine the frames into a movie using FFmpeg.
 *
 * Usage:
 * Compile using the provided Makefile:
 *   $ make
 *    gcc -o mandelmovie mandelmovie.c -lpthread
 * Run with the following format:
 *   $ ./mandelmovie -n <# of processes> -t <# of threads>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define TOTAL_IMAGES 50 // Total number of images to generate

void *generate_images(void *arg);
void show_help();

// Structure to hold thread arguments
typedef struct {
    int thread_id;
    int num_threads;
    double scale;
    double scale_step;
} thread_data_t;

int main(int argc, char *argv[]) {
    int opt;
    int num_processes = 0;
    int num_threads = 0;
    time_t start_time, end_time;

    // Record the start time
    time(&start_time);

    // Check for arguments
    if (argc != 5) {
        fprintf(stderr, "Usage: %s -n <# of processes> <# of threads>\n", argv[0]);
        return 1;
    }

    // Parse command-line options using getopt
    while ((opt = getopt(argc, argv, "n:t:h")) != -1) {
        switch (opt) {
            case 'n':
                num_processes = atoi(optarg);
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'h':
                show_help();
                return 0;
            default:
                fprintf(stderr, "Usage: %s -n <# of processes> <# of threads>\n", argv[0]);
                return 1;
        }
    }

    // Validate input
    if (num_processes <= 0 || num_threads <= 0) {
        fprintf(stderr, "Both the number of processes and threads must be positive integers.\n");
        return 1;
    }

    // Initial scale and scale step for creating the zoom effect
    double scale = 6.0;
    double scale_step = 0.1;

    // Create the specified number of child processes
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            printf("Child process %d pid: %d\n", i + 1, getpid());

            // Create threads within each child process
            pthread_t threads[num_threads];
            thread_data_t thread_data[num_threads];

            for (int j = 0; j < num_threads; j++) {
                thread_data[j].thread_id = j;
                thread_data[j].num_threads = num_threads;
                thread_data[j].scale = scale;
                thread_data[j].scale_step = scale_step;

                if (pthread_create(&threads[j], NULL, generate_images, &thread_data[j]) != 0) {
                    fprintf(stderr, "Error creating thread %d\n", j);
                    exit(1);
                }
            }

            // Wait for all threads to finish
            for (int j = 0; j < num_threads; j++) {
                pthread_join(threads[j], NULL);
            }

            exit(0); // Child process exits after completing its work
        } else if (pid < 0) {
            // Fork failed
            fprintf(stderr, "Fork failed\n");
            return 1;
        }
    }

    // Parent process: wait for all child processes to finish
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    // Record the end time
    time(&end_time);

    // Calculate and display the time taken
    double elapsed_time = difftime(end_time, start_time);
    printf("Time: %.2f seconds\n", elapsed_time);

    return 0;
}

// Thread function to generate images
void *generate_images(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int thread_id = data->thread_id;
    int num_threads = data->num_threads;
    double scale = data->scale;
    double scale_step = data->scale_step;

    for (int i = thread_id; i < TOTAL_IMAGES; i += num_threads) {
        double current_scale = scale - i * scale_step; // Calculate the current scale for the image
        char command[256];

        // Construct the command to generate the image
        snprintf(command, sizeof(command), "./mandel -s %f -o mandel%03d.jpg", current_scale, i + 1);
        int ret = system(command); // Execute the command

        if (ret != 0) {
            // If the command fails, print an error message and exit
            fprintf(stderr, "Command failed with return code %d\n", ret);
            pthread_exit(NULL);
        }
    }

    pthread_exit(NULL);
}

// Show help message
void show_help() {
    printf("Usage: mandelmovie [options]\n");
    printf("Options:\n");
    printf("  -n <# of processes>  Number of child processes to use\n");
    printf("  -t <# of threads>    Number of threads per process to use\n");
    printf("  -h                  Show this help message\n");
}
