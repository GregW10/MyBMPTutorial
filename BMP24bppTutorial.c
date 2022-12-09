#include <stdio.h>
#include <stdlib.h>

/* this tutorial focuses entirely on creating 24 bit-per-pixel BMPs - other formats are slightly more complicated */

/* any comment prefaced with "CAN BE IGNORED" indicates that the subsequent code is not directly relevant to BMP I/O - if
 * you only wish to learn about how to write BMPs to files, please skip right past those sections */


/* CAN BE IGNORED */
typedef enum boolean {
	false, true
} bool;

/* CAN BE IGNORED - this is just a handy struct to store a pixel coordinate */
typedef struct point {
	unsigned int x;
	unsigned int y;
} pnt;

/* CAN BE IGNORED */
typedef struct dimension {
	unsigned int width;
	unsigned int height;
} dim;

/* the "pack()" pragma directive indicates to the compiler that it should NOT add padding bytes within structs - this
 * is pretty essential, as there would be extra bytes floating about our BMP that would corrupt the file */
#pragma pack(push, 1)

/* definition of a color struct - will be used in creating the pixel array of the BMP file */
typedef struct color {
	unsigned char b; // blue
	unsigned char g; // green
	unsigned char r; // red
} clr;			 // this order is necessary for the BMP format

/* this struct represents the file header of a BMP file - 14 bytes */
typedef struct bmp_file_header {
	unsigned char header_field[2]; // these are two characters to represent the type of BMP - set to 'B' and 'M'
	unsigned int file_size; // the total file size of the BMP - in bytes
	unsigned short reserved_1; // set to zero
	unsigned short reserved_2; // set to zero
	unsigned int array_offset; // the offset of the starting address of the pixel array (i.e., the byte at which it starts)
} file_header;

/* this struct represents the BITMAPINFOHEADER of a BMP file - 40 bytes */
typedef struct bmp_info_header {
	unsigned int header_size; // size of this header in bytes - 40
	unsigned int width; // width of the BMP - can be anything (up to the max. value of an unsigned int, obviously)
	unsigned int height; // height of the BMP
	unsigned short color_panes; // set to 1
	unsigned short bpp; // bits per pixel - most common format is 24 bits per pixel (so 3 bytes per pixel)
	unsigned int compression; // method used to compress the BMP - set to zero for no compressions
	unsigned int image_size; // raw size of the pixel array - can be set to zero for 24 bpp bitmaps
	unsigned int h_res; // horizontal resolution of the image - can be set to zero
	unsigned int v_res; // vertical resolution of the image - can be set to zero
	unsigned int num_clr_palette; // number of colours in the colour palette - can be zero
	unsigned int num_imp_clrs; // number of important colours - can be zero
} info_header;

/* see https://en.wikipedia.org/wiki/BMP_file_format for more in-depth explanations on the above */

/* this tells the compiler to revert back to its default setting for how structs are packed */
#pragma pack(pop)

/* CAN BE IGNORED - this is the function for filling the entire background of the BMP with a given colour */
int fill_background(struct color **arr, struct color col, dim dims) {
	if (!arr || !*arr)
		return 1;
	clr *row = NULL;
	for (unsigned int y = 0; y < dims.height; ++y, ++arr) {
		row = *arr;
		for (unsigned int x = 0; x < dims.width; ++x, ++row) {
			*row = col;
		}
	}
	return 0;
}

/* CAN BE IGNORED */
bool draw_square(pnt origin, unsigned int side_len, struct color **arr, struct color col, dim image_dims) {
	if (!arr || !*arr)
		return false;
	if (origin.x >= image_dims.width || origin.y >= image_dims.height)
		return false;
	pnt top = {origin.x + side_len > image_dims.width ? image_dims.width : origin.x + side_len,
		   origin.y + side_len > image_dims.height ? image_dims.height : origin.y + side_len};
	arr += origin.y;
	clr *row = NULL;
	unsigned int x; // avoids reassigning counter within loop
	for (unsigned int y = origin.y; y < top.y; ++y, ++arr) {
		row = *arr;
		row += origin.x;
		for (x = origin.x; x < top.x; ++x, ++row) {
			*row = col;
		}
	}
	return true;
}

/* CAN BE IGNORED */
bool draw_center_square(struct color **arr, struct color col, unsigned int side_len, dim image_dims) {
	if (!arr || !*arr)
		return true;
	return draw_square((pnt) {(image_dims.width - side_len)/2, (image_dims.height - side_len)/2},
			   side_len, arr, col, image_dims);
}

const unsigned int width = 2000; // change these as you wish
const unsigned int height = 1400;

const char *bmp_path = "MyFirstBMP.bmp"; // change as you wish

clr bg_clr = {255, 0, 255}; // a dark-pinky colour - change BGR values as you wish
clr sqr_clr = {0, 255, 0}; // colour of the square we will draw - change as you wish
unsigned int sqr_side_len = width/4; // side length of the square to draw - change as you wish

int main() {
	/* CAN BE IGNORED */
	dim image_dims = {width, height};
	
	/* Padding is an important part of BMP generation. Pixel data in BMPs are stored as rows (i.e., the data for a single row is
	 * contiguous in memory), and each row must end on a 4-byte boundary. I.e., the starting address of the first byte in the
	 * first row must be a multiple of 4. Thus, if the width of the BMP is not a multiple of 4, a certain number of padding
	 * bytes will have to be added to the end of each row to ensure it ends at a 4-byte boundary. This is calculated using the
	 * width and height. For a 24 bpp or 3 byte-per-pixel BMP, multiply the width times 3, take the remainder of this divided by 4,
	 * if this is zero, take the padding as zero, if not, then take 4 minus the remainder of this number as the number of bytes of 
	 * padding needed. */
	const unsigned char rem = (width*3) % 4;
	const int padding = rem ? 4 - rem : 0; // thus, if the width is a multiple of 4, padding will be zero
	
	/* declare a file_header variable and initialise all its values to zero */
	file_header fh = {0};
	
	/* set the header field to "BM" */
	fh.header_field[0] = 'B';
	fh.header_field[1] = 'M';

	/* set the file size - each row requires padding, so must multiply height times padding to get total padding bytes */
	fh.file_size = sizeof(file_header) + sizeof(info_header) + height*(width*3 + padding);
	//			= 14			= 40			= variable
	
	/* set the reserved fields to zero */
	fh.reserved_1 = 0;
	fh.reserved_2 = 0;
	
	/* set the pixel array offset - will always be 54 for a 24 bpp BMP */
	fh.array_offset = sizeof(file_header) + sizeof(info_header); // = 54
	
	/* now we declare an info_header variable and initialise all values to zero */
	info_header ih = {0};
	
	/* set the total size of the BITMAPINFOHEADER (this is its formal name) */
	ih.header_size = sizeof(info_header); // = 40
	
	/* set the BMP width and height */
	ih.width = width;
	ih.height = height;
	
	/* set the number of colour panes (always 1) */
	ih.color_panes = 1;
	
	/* set the number of bits per pixel - 24 for this tutorial */
	ih.bpp = 24;
	
	/* for a 24 bpp BMP, all the following fields can be set to zero - here, there is no need to explicitly do this
	 * since the struct was initialised to zero, but it is here for clarity */
	ih.compression = 0;
	ih.image_size = 0;
	ih.h_res = 0;
	ih.v_res = 0;
	ih.num_clr_palette = 0;
	ih.num_imp_clrs = 0;
	
	/* Now, for the purposes of this tutorial, we could immediately open a file and start writing to it at this point,
	 * but, for all intents and purposes, when you create a BMP file, you want to depict something useful on it. Thus,
	 * we will allocate a 2D array of "clr" structs (see top of file), to be able to "draw" stuff on the BMP by
	 * setting certain pixels of it to different colours. Since padding bytes are not included within this 2D array,
	 * those will have to be written when outputting the BMP. We start by allocating the memory for the array: */
	clr **array = malloc(height*sizeof(clr*));
	/* we create a double pointer, since this pointer is now pointing to an array of pointers (and each pointer will
	 * point to a row on the image) */
	
	/* check if there was an error allocating the memory */
	if (!array) {
		fprintf(stderr, "Array of pointers memory allocation error.\n");
		return 1;
	}
	
	/* declare and define a copy of the above pointer to use below */
	clr **cpy = array;
	
	/* now, we allocate the memory for each row for each pointer in the above pointer array */
	for (unsigned int y = 0; y < height; ++y, ++cpy) { // cpy pointer is incremented in each iteration (to point to next row)
		/* each row-pointer is allocated the memory for each row (width times 3 since there are 3 bytes per pixel) */
		*cpy = malloc(width*sizeof(clr));
		if (!*cpy) { // check for memory allocation error
			fprintf(stderr, "Row memory allocation error (for row = %i).\n", y);
			return 1;
		}
	}
	
	/* CAN BE IGNORED - We are now at a point where we can start drawing on the BMP. The first thing we will do is set the
	 * background of the BMP to a certain colour: */
	fill_background(array, bg_clr, image_dims);
	
	/* ... and now we will draw a square in the middle of the image: */
	draw_center_square(array, sqr_clr, sqr_side_len, image_dims);
	
	/* we are now in a position to write our image */
	
	/* get file pointer to bmp file we will be writing - must be obtained in "write bytes" (wb) mode since we will be writing
	 * binary data only */
	FILE *fp = fopen(bmp_path, "wb");
	
	/* here we check if the fopen() function failed (if it fails, fp == NULL) */
	if (!fp) {
		fprintf(stderr, "Error opening file: %s\n", bmp_path);
		return 1;
	}
	
	/* here we will write the file header and info header to the BMP file */
	fwrite(&fh, sizeof(file_header), 1, fp);
	fwrite(&ih, sizeof(info_header), 1, fp);
	
	/* here we declare and initialise the array of characters for padding (only 3 chars since that is the max. number that would
	 * be written) - a certain number of the zeroes will be written, depending on the width (padding variable stores this) */
	unsigned char pad[3] = {0, 0, 0};
	
	/* now we will write each row individually to the image - since padding needs to be inserted after each row, and the
	 * pixel array data is not contiguous - and then free the memory for each row */
	cpy = array;
	for (unsigned int y = 0; y < height; ++y, ++cpy) {
		/* the below line reads: write the cpy array - with an element size of "sizeof(color)" and number of elements
		 * equal to width - to the file represented by the file pointer "fp" */
		fwrite(*cpy, sizeof(clr), width, fp);
		fwrite(pad, sizeof(unsigned char), padding, fp); // this does nothing if padding == 0
		/* here we free the memory for the row */
		free(*cpy);
	}
	/* here we free the original array of pointers */
	free(array);
	
	/* I/O for a BMP would be faster if all the data were stored in a single pointer (i.e., contiguously) - but this makes accessing
	 * each pixel slightly more complicated, especially since padding bytes are included in the single contiguous array - this
	 * approach would be better, however, for reading and writing many BMP files at once, and for faster access to pixels */
	
	/* here we print the total size of the BMP file written */
	printf("File size: %zu\n", ftell(fp)); // the position of the file pointer at the end is equal to the file size
	
	/* close the handle to the file (flushes any data waiting to be written) */
	fclose(fp);
	
	return 0;
}
