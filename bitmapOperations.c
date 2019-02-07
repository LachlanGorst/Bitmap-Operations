// *****************************************************************************************************************
//  File name   :   bitmapOperations.c
//  Version     :   1.0
//  Description :   This program will perform different operations on images saved as bitmaps. (extension .bmp)
//					Primarily, the program will perform the following operations on images:
//						1.Save a copy of an image - Load and save a copy of an image
//						2.Remove channel - Remove either the red, blue or green components of an image
//						3.Quantize - Reduce the number of colours in an image
//						4.Invert - Invert all the colours in an image
//						5.Flip Horizontally - Flip the image horizontally
//  Author      :   Lachlan Gorst
//  Contact     :   lachlangorst@outlook.com
//  Compiler    :   GNU GCC
//  Last Update :   03-June-2018
//  Note        :   ...
//
// (c) Lachlan Gorst 2018 - All Rights Reserved
// *****************************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_NAME_SIZE 100

#define FAIL 1
#define SUCCESS 0
#define RED 1
#define GREEN 2
#define BLUE 3

//#define DEBUG 1

struct Pixel 
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

struct RGB_Image
{
	char file_name[MAX_FILE_NAME_SIZE];
	int height;
	int width;
	int size;
	struct Pixel** pixels;
};

void save_image_copy();
void remove_image_channel();
void invert_image_colours();
void quantize_image();
void flip_horizontal_image();

int load_image(struct RGB_Image* image_ptr);
int save_image(struct RGB_Image image);

void free_RGB_pixels(struct RGB_Image image);

void invert_pixels(struct Pixel** pixels, int height, int width);
void flip_horizontal_pixels(struct Pixel** pixels, int height, int width);
void quantize_pixels(struct Pixel** pixels, int height, int width, int quantization_level);
void remove_red_pixels(struct Pixel** pixels, int height, int width);
void remove_green_pixels(struct Pixel** pixels, int height, int width);
void remove_blue_pixels(struct Pixel** pixels, int height, int width);

int main()
{
	int choice; 
	do
	{
		printf("MENU\n");
		printf("1 . Save Copy of Image\n");
		printf("2 . Remove Image Channel\n");
		printf("3 . Invert Image Colours\n");
		printf("4 . Quantize Image\n");
		printf("5 . Flip Image Horizontally\n");
		scanf("%d",&choice);
		if(choice==1)
		{
			save_image_copy();
		}
		else if(choice==2)
		{
			remove_image_channel();
		}
		else if(choice==3)
		{
			invert_image_colours();
		}
		else if(choice==4)
		{
			quantize_image();
		}
		else if(choice==5)
		{
			flip_horizontal_image();
		}
	}while(choice!=-1);
	return(0);
}

/*
Name:
    load_image
Description:
    Will load image data into the RGB_Image struct pointed to. This data will be loaded from a user selected image file.
Note:
    Using the shorthand ( -> ) to derefernce pointers.
    Image file will be saved as extension ( .bmp ).
    SEEK_CUR = macro for moving byte position in file, starting from current position.
Parameters:
    struct RGB_Image* image_ptr		struct that will have image data loaded into, pass a pointer to a struct RGB_Image
Returns:
    FAIL if load fails
    SUCCESS if successful execution
*/
int load_image(struct RGB_Image* image_ptr)
{
	printf("Enter the file name of the image to load: ");
	scanf("%s", image_ptr->file_name);
	char file_name_cpy[MAX_FILE_NAME_SIZE];
	strcpy(file_name_cpy, image_ptr->file_name);
	strcat(file_name_cpy, ".bmp");
	FILE* fp = fopen(file_name_cpy, "r");
	if(fp==NULL)
	{
		printf("File can not be opened\n");
		return FAIL;
	}
	fseek(fp,2,SEEK_CUR); 					//We don't need first two bytes of the file. 
	fread(&(image_ptr->size),4,1,fp); 		//Read the size of the file. Read next 4 bytes (size of int) . 1, since not an array.
	fseek(fp,12,SEEK_CUR); 					//skip 12 bytes. 
	fread(&(image_ptr->width),4,1,fp);
	fread(&(image_ptr->height),4,1,fp);
	fseek(fp,28,SEEK_CUR);

	//Dynamically allocate memory for height of array
	image_ptr->pixels = (struct Pixel**) malloc(sizeof(struct Pixel*) * (image_ptr->height));
	
	int i;
	for(i=0; i<image_ptr->height; i++)
	{	
		//Dynamically allocate memory for width of array - per each height cell
		image_ptr->pixels[i] = (struct Pixel*) malloc(sizeof(struct Pixel) * (image_ptr->width));
	}
	int j;
	//File color bytes organised as bgr not rgb.
	for (i=0; i<image_ptr->height; i++)
	{
		for(j=0; j<image_ptr->width; j++)
		{
			fread(&(image_ptr->pixels[i][j].blue),1,1,fp);
			fread(&(image_ptr->pixels[i][j].green),1,1,fp);
			fread(&(image_ptr->pixels[i][j].red),1,1,fp);
		}
	}
	fclose(fp);
	printf("Image Loaded\n\n");
	return SUCCESS;
}

/*
Name:
    save_image
Description:
    This function takes image data and saves it to a .bmp file.
Note:
    .
Parameters:
    struct RGB_Image image		struct that will have image data to be saved, pass a struct RGB_Image
Returns:
    FAIL if file can't be opened
    SUCCESS if successful execution    
*/
int save_image(struct RGB_Image image)
{
	char file_name_cpy[MAX_FILE_NAME_SIZE];
	strcpy(file_name_cpy, image.file_name);
	strcat(file_name_cpy, ".bmp");
	FILE* fp = fopen(file_name_cpy, "w");
	if(fp==NULL)
	{
		printf("File can not be saved\n");
		return FAIL;
	}
	int image_datasize = image.size-40;
	unsigned char bmp_header[] = {
									0x42, 0x4D,
									image.size, image.size>>8, image.size>>16, image.size>>24,
									0x00, 0x00, 0x00, 0x00, 
									0x36, 0x00, 0x00, 0x00, 
									0x28, 0x00, 0x00, 0x00, 
									image.width, image.width>>8, image.width>>16, image.width>>24,
									image.height, image.height>>8, image.height>>16, image.height>>24,
									0x01, 0x00, 0x18, 0x00, 
									0x00, 0x00, 0x00, 0x00, 
									image_datasize, image_datasize>>8, image_datasize>>16, image_datasize>>24, 
									0x00, 0x00, 0x00, 0x00, 
									0x00, 0x00, 0x00, 0x00, 
									0x00, 0x00, 0x00, 0x00, 
									0x00, 0x00, 0x00, 0x00, 
								  };
	fwrite(bmp_header, 1, 54, fp);
	int i,j;
	for (i=0; i<image.height; i++)
	{
		for(j=0; j<image.width; j++)
		{
			fwrite(&(image.pixels[i][j].blue),1,1,fp);
			fwrite(&(image.pixels[i][j].green),1,1,fp);
			fwrite(&(image.pixels[i][j].red),1,1,fp);
		}
	}
	fclose(fp);
	printf("Image Saved\n\n");
	return SUCCESS;
}

/*
Name:
    free_RGB_pixels
Description:
    This function frees the dynamic memory allocated from when the image was loaded.
Note:
    .
Parameters:
    struct RGB_Image image		struct that will have dynamically memory that needs to be freed, pass a struct RGB_Image
Returns:
    .
*/
void free_RGB_pixels(struct RGB_Image image)
{
	#ifdef DEBUG
		printf("inside free rgb\n");
	#endif
	int i;
	for(i=0; i<image.height; i++)
	{
		free(image.pixels[i]);
	}
	free(image.pixels);
}

/*
Name:
    save_image_copy
Description:
    This function loads an image and then saves the same image.
Note:
    .
Parameters:
    .
Returns:
    .
*/
void save_image_copy()
{
	struct RGB_Image image;
	if(!load_image(&image))
	{
		printf("Image Copied\n\n");
		strcat(image.file_name, "_copy");
		save_image(image);
		free_RGB_pixels(image);
	}
}

/*
Name:
    remove_image_channel
Description:
    This function loads an image, asks the user which channel they would like to remove,
	removes the channel and then saves the image.
Note:
    Uses macros:
    	RED = 1
    	GREEN = 2
    	BLUE = 3
Parameters:
    .
Returns:
    .
*/
void remove_image_channel()
{
	struct RGB_Image image;
	if(!load_image(&image))
	{
		int choice = 1;
		do	
		{	
			if(choice<1 || choice>3)
			{
				printf("NO\n");
			}
			printf("Enter the channel to remove:\n");
			printf("1.Red\n");
			printf("2.Green\n");
			printf("3.Blue\n");
			scanf("%d", &choice);
		}while(choice<1 || choice>3);
		if(choice==RED)
		{
			remove_red_pixels(image.pixels, image.height, image.width);
			strcat(image.file_name, "_red_channel_removed");
			printf("red channel removed\n");
		}
		else if(choice==GREEN)
		{
			remove_green_pixels(image.pixels, image.height, image.width);
			strcat(image.file_name, "_green_channel_removed");
			printf("green channel removed\n");
		}
		else if(choice==BLUE)
		{
			remove_blue_pixels(image.pixels, image.height, image.width);
			strcat(image.file_name, "_blue_channel_removed");
			printf("blue channel removed\n");
		}
		save_image(image);
		free_RGB_pixels(image);
	}
}

/*
Name:
    invert_image_colours
Description:
    This function loads an image, inverts the image and then saves the image. 
    Inverting inverts all the bits in each colour.
Note:
    .
Parameters:
    .
Returns:
    .
*/
void invert_image_colours()
{
	#ifdef DEBUG	
		printf("invert_image_colours\n");
	#endif
	struct RGB_Image image;
	if(!load_image(&image))
	{
		invert_pixels(image.pixels, image.height, image.width);
		strcat(image.file_name, "_inverted");
		printf("Image Inverted\n");
		save_image(image);
		free_RGB_pixels(image);
	}
}

/*
Name:
    quantize_image
Description:
    This function loads an image, asks the user for the level of quantization, quantizes the image and then saves the
	image.
	Quantization reduces the number of colours in an image and is a type of lossy compression.
Note:
    .
Parameters:
    .
Returns:
    .
*/
void quantize_image()
{
	#ifdef DEBUG	
		printf("quantize_image\n");
	#endif
	struct RGB_Image image;
	if(!load_image(&image))
	{
		int choice = 1;
		do	
		{	
			if(choice<0 || choice>7)
			{
				printf("NO\n");
			}
			printf("Enter the quantization level (0 to 7): ");
			scanf("%d", &choice);
		}while(choice<0 || choice>7);
		quantize_pixels(image.pixels, image.height, image.width, choice);
		switch (choice) 
		{
			case 0:
				strcat(image.file_name, "_quantize_0");
				break;
			case 1:
				strcat(image.file_name, "_quantize_1");
				break;
			case 2:
				strcat(image.file_name, "_quantize_2");
				break;
			case 3:
				strcat(image.file_name, "_quantize_3");
				break;
			case 4:
				strcat(image.file_name, "_quantize_4");
				break;
			case 5:
				strcat(image.file_name, "_quantize_5");
				break;
			case 6:
				strcat(image.file_name, "_quantize_6");
				break;
			case 7:
				strcat(image.file_name, "_quantize_7");
				break;
		}
		printf("Image quantized by a level of %d\n", choice);
		save_image(image);
		free_RGB_pixels(image);
	}
}

/*
Name:
    flip_horizontal_image
Description:
    This function loads an image, flips the image horizontally and then saves the image.
Note:
    .
Parameters:
    .
Returns:
    .
*/
void flip_horizontal_image()
{
	#ifdef DEBUG	
		printf("flip_horizontal_image\n");
	#endif
	struct RGB_Image image;
	if(!load_image(&image))
	{
		flip_horizontal_pixels(image.pixels, image.height, image.width);
		strcat(image.file_name, "_flipped_horizontally");
		printf("Image Flipped Horizontally\n");
		save_image(image);
		free_RGB_pixels(image);
	}
}

/*
Name:
    invert_pixels
Description:
    This function inverts each colour for each pixel in the image. 
Note:
    .
Parameters:
    struct Pixel** pixels      two dimensional array of struct Pixels, pass as pointer to a pointer of struct Pixel
    int height                 height of the two dimensional Pixel array
    int width                  width of the two dimensional Pixel array
Returns:
    .
*/
void invert_pixels(struct Pixel** pixels, int height, int width)
{
	#ifdef DEBUG	
		printf("invert_pixels\n");
	#endif
	int i, j;
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			pixels[i][j].red = pixels[i][j].red ^ 0xFF;
			pixels[i][j].green = pixels[i][j].green ^ 0xFF;
			pixels[i][j].blue = pixels[i][j].blue ^ 0xFF;
		}
	}
}

/*
Name:
    flip_horizontal_pixels
Description:
    This function reorders the pixels such that the pixels become flipped horizontally.
Note:
    .
Parameters:
    struct Pixel** pixels      two dimensional array of struct Pixels, pass as pointer to a pointer of struct Pixel
    int height                 height of the two dimensional Pixel array
    int width                  width of the two dimensional Pixel array
Returns:
    .
*/
void flip_horizontal_pixels(struct Pixel** pixels, int height, int width)
{
	#ifdef DEBUG	
		printf("flip_horizontal_pixels\n");
	#endif

	//Create reversed Pixel 2D array.
	struct Pixel** pixelsReversed = (struct Pixel**) malloc(sizeof(struct Pixel*) * height);
	int i;
	for(i=0; i<height; i++)
	{	
		//Dynamically allocate memory for width of array - per each height cell.
		pixelsReversed[i] = (struct Pixel*) malloc(sizeof(struct Pixel) * width);
	}

	//Following will create reversed array.
	int j, reversedWidth;
	for(i=0; i<height; i++)
	{	
		reversedWidth = 0;
		for(j=width-1; j>=0; j--)
		{
			pixelsReversed[i][reversedWidth].blue = pixels[i][j].blue;
			pixelsReversed[i][reversedWidth].green = pixels[i][j].green;
			pixelsReversed[i][reversedWidth].red = pixels[i][j].red;
			reversedWidth++;
		}
	}

	//Following will assign reversed values to original
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			pixels[i][j].blue = pixelsReversed[i][j].blue;
			pixels[i][j].green = pixelsReversed[i][j].green;
			pixels[i][j].red = pixelsReversed[i][j].red;
		}
	}
}

/*
Name:
    quantize_pixels
Description:
    This function clears the quantization level number of bits (changes them to 0) from the rightmost bit for each
	colour.
Note:
    .
Parameters:
    struct Pixel** pixels      two dimensional array of struct Pixels, pass as pointer to a pointer of struct Pixel
    int height                 height of the two dimensional Pixel array
    int width                  width of the two dimensional Pixel array
Returns:
    .
*/
void quantize_pixels(struct Pixel** pixels, int height, int width, int quantization_level)
{
	#ifdef DEBUG	
		printf("quantize_pixels\n");
	#endif
	int i, j, k;
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			for(k=0; k<quantization_level; k++)
			{
				pixels[i][j].red &= ~(1 << k);
				pixels[i][j].green &= ~(1 << k);
				pixels[i][j].blue &= ~(1 << k);
			}
		}
	}
}

/*
Name:
    remove_red_pixels
Description:
    Iterates through all pixels and set red to 0.
Note:
    .
Parameters:
    struct Pixel** pixels      two dimensional array of struct Pixels, pass as pointer to a pointer of struct Pixel
    int height                 height of the two dimensional Pixel array
    int width                  width of the two dimensional Pixel array
Returns:
    .
*/
void remove_red_pixels(struct Pixel** pixels, int height, int width)
{
	#ifdef DEBUG	
		printf("remove_red_pixels\n");
	#endif
	int i, j;
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			pixels[i][j].red = 0;
		}
	}
}

/*
Name:
    remove_green_pixels
Description:
    Iterates through all pixels and set green to 0.
Note:
    .
Parameters:
    struct Pixel** pixels      two dimensional array of struct Pixels, pass as pointer to a pointer of struct Pixel
    int height                 height of the two dimensional Pixel array
    int width                  width of the two dimensional Pixel array
Returns:
    .
*/
void remove_green_pixels(struct Pixel** pixels, int height, int width)
{
	#ifdef DEBUG	
		printf("remove_green_pixels\n");
	#endif
	int i, j;
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			pixels[i][j].green = 0;
		}
	}
}

/*
Name:
    remove_blue_pixels
Description:
    Iterates through all pixels and set blue to 0.
Note:
    .
Parameters:
    struct Pixel** pixels      two dimensional array of struct Pixels, pass as pointer to a pointer of struct Pixel
    int height                 height of the two dimensional Pixel array
    int width                  width of the two dimensional Pixel array
Returns:
    .
*/
void remove_blue_pixels(struct Pixel** pixels, int height, int width)
{
	#ifdef DEBUG	
		printf("remove_blue_pixels\n");
	#endif
	int i, j;
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			pixels[i][j].blue = 0;
		}
	}
}












