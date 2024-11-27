# System Programming Lab 11 Multiprocessing

**Author:** Nastasja Radulovic  
**Course:** CPE 2600 Section 121  
**Date:** 11/26/24

---

## Overview

This project demonstrates parallel programming techniques using **C** and **Linux processes** to generate Mandelbrot set images and create a movie. The program uses **multi-processing** to speed up the computation of fractal images by dividing the workload across multiple child processes.

---

## Features

- **Mandelbrot Image Generation**:
  - Generates a series of images (`mandelX.jpg`) representing zoomed views of the Mandelbrot set.
  - Supports configurable zoom levels and frame counts.

- **Multi-Processing**:
  - Distributes image generation tasks across multiple child processes for faster computation.

- **Movie Creation**:
  - Combines the generated images into a video using `ffmpeg`.

---

## Files

- **`mandel.c`**: Generates individual Mandelbrot images based on scale and output filename.
- **`mandelmovie.c`**: Manages the creation of multiple child processes and orchestrates the generation of all images.
- **`Makefile`**: Automates the compilation of the programs.
- **`README.md`**: Documentation for the project.
- **`output_files/`**: Directory containing the generated Mandelbrot images.

---

## Compilation and Execution

### Compile the Programs
Use the provided `Makefile` to compile the project:
```bash
make

