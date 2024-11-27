/*
 * Lab 11 Assignment
 * Nastasja Radulovic
 * CPE 2600 121
 * Date: 11/26/24
 *
 * Description:
 * This program generates a sequence of Mandelbrot set images by 
 * creating and managing child processes. It uses command-line options 
 * to specify the number of processes and dynamically scales the images 
 * for creating a movie effect. Each frame is saved as a separate JPEG file. 
 * The program can be extended to combine the frames into a movie using FFmpeg.
 *
 * Usage:
 * Compile using the provided Makefile:
 *   $ make
 * Run with the following format:
 *   $ ./mandelmovie -n <# of processes>
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>

#define totalImage 50

int main(int argc, char *argv[]) {
    int opt;
    int num_children = 0;
    time_t start_time, end_time;

    // Record the start time
    time(&start_time);

    //check for arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s -n <# of children>\n", argv[0]);
        return 1;
    }

    // Parse command line options using getopt
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                num_children = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -n <# of children>\n", argv[0]);
                return 1;
        }
    }

    // Check if the number of children is specified and valid
    if (num_children <= 0) {
        fprintf(stderr, "Number of children must be a positive integer.\n");
        return 1;
    }

    // Initial scale and scale step for creating the moving video effect
    double scale = 6.0;
    double scale_step = 0.1;

    pid_t pid;

    // Create the specified number of child processes
    for (int i = 0; i < num_children; i++) {
        pid = fork(); // Fork a new process
        if (pid == 0) {
            // Child process
            printf("Child process %d pid: %d\n", i + 1, getpid());
            // Each child process handles a subset of the total images
            for (int j = i; j < totalImage; j += num_children) {
                double current_scale = scale - j * scale_step; // Calculate the current scale for the image
                char command[256];
                // Construct the command to generate the image
                snprintf(command, sizeof(command), "./mandel -s %f -o mandel%d.jpg", current_scale, j + 1);
                int ret = system(command); // Execute the command
                if (ret != 0) {
                    // If the command fails, print an error message and exit
                    fprintf(stderr, "Command failed with return code %d\n", ret);
                    exit(1);
                }
            }
            exit(0); // Child process exits after completing its work
        } else if (pid < 0) {
            // Fork failed
            fprintf(stderr, "Fork failed\n");
            return 1;
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_children; i++) {
        wait(NULL);
    }

    // Record the end time
    time(&end_time);

    // Calc & display the time
    double elapsed_time = difftime(end_time, start_time);
    printf("Time: %.2f seconds\n", elapsed_time);

    return 0;
}