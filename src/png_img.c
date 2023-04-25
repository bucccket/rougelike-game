#include "png_img.h"

void readpng_version_info(void)
{
	printf("Compiled with libpng %s; using libpng %s.\n", PNG_LIBPNG_VER_STRING, png_libpng_ver);
	printf("Compiled with zlib %s; using zlib %s.\n", ZLIB_VERSION, zlib_version);
}

int openPngFile(const char* fname, struct PngImage* imageData)
{
	FILE* fp = fopen(fname, "rb");

	if (!fp) {
		return E_PNG_IMG_NO_SUCH_FILE;
	}

	uch sig[8];

	fread(sig, 1, 8, fp);
	if (!png_check_sig(sig, 8))
		return E_PNG_IMG_BAD_SIGNATURE;

	imageData->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!imageData->png_ptr)
		return E_PNG_IMG_OUT_OF_MEMORY;

	imageData->info_ptr = png_create_info_struct(imageData->png_ptr);
	if (!imageData->info_ptr) {
		png_destroy_read_struct(&imageData->png_ptr, NULL, NULL);
		return E_PNG_IMG_OUT_OF_MEMORY;
	}

	if (setjmp(png_jmpbuf(imageData->png_ptr))) {
		png_destroy_read_struct(&imageData->png_ptr, &imageData->info_ptr, NULL);
		return E_PNG_IMG_LIBRARY_ERROR;
	}

	png_init_io(imageData->png_ptr, fp);
	png_set_sig_bytes(imageData->png_ptr, 8);
	png_read_info(imageData->png_ptr, imageData->info_ptr);

	png_get_IHDR(imageData->png_ptr,
	    imageData->info_ptr,
	    &imageData->width,
	    &imageData->height,
	    &imageData->bit_depth,
	    &imageData->color_type,
	    NULL, NULL, NULL);

	return PNG_IMG_OK;
}

int readpng_get_bgcolor(struct PngImage* imageData, uch* red, uch* green, uch* blue)
{

	if (setjmp(png_jmpbuf(imageData->png_ptr))) {
		png_destroy_read_struct(&imageData->png_ptr, &imageData->info_ptr, NULL);
		return E_PNG_IMG_LIBRARY_ERROR;
	}

	if (!png_get_valid(imageData->png_ptr, imageData->info_ptr, PNG_INFO_bKGD))
		return E_PNG_IMG_LIBRARY_ERROR;

	png_color_16p pBackground;
	png_get_bKGD(imageData->png_ptr, imageData->info_ptr, &pBackground);

	if (imageData->bit_depth == 16) {
		*red = pBackground->red >> 8;
		*green = pBackground->green >> 8;
		*blue = pBackground->blue >> 8;
	} else if (imageData->color_type == PNG_COLOR_TYPE_GRAY && imageData->bit_depth < 8) {
		if (imageData->bit_depth == 1)
			*red = *green = *blue = pBackground->gray ? 255 : 0;
		else if (imageData->bit_depth == 2) /* i.e., max value is 3 */
			*red = *green = *blue = (255 / 3) * pBackground->gray;
		else /* bit_depth == 4 */ /* i.e., max value is 15 */
			*red = *green = *blue = (255 / 15) * pBackground->gray;
	} else {
		*red = pBackground->red;
		*green = pBackground->green;
		*blue = pBackground->blue;
	}

	printf("read background done.\n");

	return PNG_IMG_OK;
}

int getGamma(double* display_exponent)
{

	double LUT_exponent;
	double CRT_exponent = 2.2;
	double default_display_exponent;

	char* p;

	LUT_exponent = 1.0; /* assume no LUT:  most PCs */

	default_display_exponent = LUT_exponent * CRT_exponent;
	if ((p = getenv("SCREEN_GAMMA")) != NULL)
		*display_exponent = atof(p);
	else
		*display_exponent = default_display_exponent;

	return PNG_IMG_OK;
}

uch* readpng_get_image(struct PngImage* imageData, double display_exponent)
{

	if (setjmp(png_jmpbuf(imageData->png_ptr))) {
		png_destroy_read_struct(&imageData->png_ptr, &imageData->info_ptr, NULL);
		return NULL;
	}

	if (!imageData->channels) {
		imageData->channels = (int*)calloc(1, sizeof(int));
	}
	if (!imageData->rowbytes) {
		imageData->rowbytes = (ulg*)calloc(1, sizeof(ulg));
	}

	if (imageData->color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(imageData->png_ptr);
	if (imageData->color_type == PNG_COLOR_TYPE_GRAY && imageData->bit_depth < 8)
		png_set_expand(imageData->png_ptr);
	if (png_get_valid(imageData->png_ptr, imageData->info_ptr, PNG_INFO_tRNS))
		png_set_expand(imageData->png_ptr);

	if (imageData->bit_depth == 16)
		png_set_strip_16(imageData->png_ptr);
	if (imageData->color_type == PNG_COLOR_TYPE_GRAY || imageData->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(imageData->png_ptr);

	if (png_get_gAMA(imageData->png_ptr, imageData->info_ptr, &imageData->gamma))
		png_set_gamma(imageData->png_ptr, display_exponent, imageData->gamma);

	png_uint_32 i, rowbytes;
	png_bytep row_pointers[imageData->height];

	png_read_update_info(imageData->png_ptr, imageData->info_ptr);

	*imageData->rowbytes = rowbytes = png_get_rowbytes(imageData->png_ptr, imageData->info_ptr);
	*imageData->channels = (int)png_get_channels(imageData->png_ptr, imageData->info_ptr);

	uch* image_bytes;

	if ((image_bytes = (uch*)malloc(rowbytes * imageData->height)) == NULL) {
		png_destroy_read_struct(&imageData->png_ptr, &imageData->info_ptr, NULL);
		return NULL;
	}

	for (i = 0; i < imageData->height; ++i)
		row_pointers[i] = image_bytes + i * rowbytes;

	png_read_image(imageData->png_ptr, row_pointers);

	png_read_end(imageData->png_ptr, NULL);

	return image_bytes;
}

void readpng_cleanup(struct PngImage* imageData)
{
	free(imageData->channels);
	free(imageData->rowbytes);
	free(imageData->imageBytes);
	if (imageData->png_ptr && imageData->info_ptr) {
		png_destroy_read_struct(&imageData->png_ptr, &imageData->info_ptr, NULL);
		imageData->png_ptr = NULL;
		imageData->info_ptr = NULL;
	}
}
