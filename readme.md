
# Project Title

A brief description of what this project does and who it's for

# Assignment 1: Computer Graphics

## Project Overview

This project is part of the Computer Graphics course and involves implementing image filters using C++ and OpenGL. The goal is to process an 8-bit .jpg image and perform various image processing tasks using the provided graphics engine.

## Compiling the Engine

1. Clone the engine using the link provided on the course site.
2. Open CMake GUI and set the project folder and build directory (e.g., "build" in the same folder).
3. Press "Configure" and then "Generate." Ensure there are no errors.
4. Open your compiler and compile the Game project (e.g., set it as the startup project in Visual Studio).

## Implemented Image Processing Tasks

### 1. Read Image to 1D Array

Read the input image to a one-dimensional array of unsigned bytes.

### 2. Viewport Configuration

Open a window of size 512x512 pixels and divide it into four squares using different viewports.

### 3. Grayscale Display

The upper-left square displays the image in grayscale without considering the color table.

### 4. Edge Detection

The upper-right square shows the "edges" image using Sobel operators for edge detection.

### 5. Halftone Pattern

The bottom-left square displays a halftone pattern based on a specified algorithm.

### 6. Floyd-Steinberg Algorithm

The bottom-right square implements the Floyd-Steinberg Algorithm to reduce the image from 256 intensity grayscale values to 16 intensity grayscale values.

### 7. Text Files

Generate three text files (img4.txt, img5.txt, and img6.txt) for subsections 4-6. Each file contains pixel values (0-1 for black and white, and 0-15 for grayscale).

## Code Changes

The following files and functions were modified or added:
- **game.cpp**: Implemented image processing algorithms and handling of different squares.
- **main.cpp**: Setup for the Computer Graphics project, including initialization and rendering.
- **scene.cpp**: Functions related to scene manipulation and picking.

## How to Compile and Run

1. Clone the repository.
2. Follow the compilation instructions mentioned above.
3. Run the compiled executable.



