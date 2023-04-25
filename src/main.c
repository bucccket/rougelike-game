#include <MiniFB.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "filemgr.h"
#include "png_img.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

static unsigned int g_buffer_high[WINDOW_WIDTH * WINDOW_HEIGHT];
static unsigned int resolution_scaling = 0; // 0 = native, 1 = 1/2 scale, 2=1/4 scale ...

void pretty_square(unsigned int* p, int dimenW, int dimenH)
{
	memset(p, 127, dimenW * dimenH << 2);

	struct PngImage* imageData = (struct PngImage*)calloc(1, sizeof(struct PngImage));

	switch (openPngFile("img/ralsei.png", imageData)) {
	case PNG_IMG_OK:
		printf("%s: Opened PNG file.\n", __func__);
		printf("%s: w %d, h %d, color type %d, bit_depth %d\n", __func__, imageData->width, imageData->height, imageData->color_type, imageData->bit_depth);
		uch red, green, blue;
		readpng_get_bgcolor(imageData, &red, &green, &blue);
		double* display_exponent = (double*)calloc(1, sizeof(double));
		getGamma(display_exponent);
		uch* imageBytes;
		imageBytes = readpng_get_image(imageData, *display_exponent);
		if (!imageBytes) {
			printf("Cannot read image data!\n");
		}

		printf("window: (%d,%d) image: (%d,%d)\n", dimenW, dimenH, imageData->width, imageData->height);

		int pad = WINDOW_WIDTH - imageData->width;

		// copy image per row using memcpy and pad offset
		// add pad to image struct
		// add pchannels and prowbytes to image struct
		for (long i = 0; i < (imageData->width * imageData->height); i++) {
			p[i + pad * (i / imageData->width)] = MFB_ARGB(0xFF, imageBytes[(i * 3)], imageBytes[(i * 3) + 1], imageBytes[(i * 3) + 2]);
		}

		free(display_exponent);

		readpng_cleanup(imageData);
		free(imageData);
		free(imageBytes);
		break;
	case E_PNG_IMG_NULLPTR:
		printf("error! FILE* pointer invalid!\n");
		break;
	case E_PNG_IMG_NO_SUCH_FILE:
		printf("error! File cannot be found/read!\n");
		break;
	case E_PNG_IMG_OUT_OF_MEMORY:
		printf("error! OutOfMemory!\n");
		break;
	default:
		printf("fatal error! Unkown file loading error!\n");
	}
}

int main(void)
{
	readpng_version_info();

	pretty_square(g_buffer_high, WINDOW_WIDTH, WINDOW_HEIGHT);

	struct mfb_window* window_high = mfb_open("HighRes", WINDOW_WIDTH >> resolution_scaling, WINDOW_HEIGHT >> resolution_scaling);
	do {
		if (mfb_update_ex(window_high, g_buffer_high, WINDOW_WIDTH, WINDOW_HEIGHT) != STATE_OK) {
			mfb_close(window_high);
			window_high = NULL;
			break;
		}
	} while (mfb_wait_sync(window_high));

	return 0;
}
