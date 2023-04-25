#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>
#include <zlib.h>

#define PNG_IMG_OK 0

#define E_PNG_IMG_NULLPTR 1
#define E_PNG_IMG_NO_SUCH_FILE 2
#define E_PNG_IMG_BAD_SIGNATURE 3
#define E_PNG_IMG_OUT_OF_MEMORY 4
#define E_PNG_IMG_LIBRARY_ERROR 5

typedef unsigned char uch;
typedef unsigned long ulg;
typedef unsigned short ush;

struct ColorRGB {
	uch red;
	uch green;
	uch blue;
};

struct PngImage {
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width;
	png_uint_32 height;
	int bit_depth;
	int color_type;
	double gamma;
	struct ColorRGB background;
	int* channels;
	ulg* rowbytes;
	uch* imageBytes;
};

void readpng_version_info(void);

int openPngFile(const char* fname, struct PngImage* imageData);

int readpng_get_bgcolor(struct PngImage* imageData, uch* red, uch* green, uch* blue);

int getGamma(double* display_exponent);

uch* readpng_get_image(struct PngImage* imageData, double display_exponent);

void readpng_cleanup(struct PngImage* imageData);
