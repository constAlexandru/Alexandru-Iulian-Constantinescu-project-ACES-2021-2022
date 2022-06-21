/*************************************************************
Filename: code.c

Description: Source file for the edge enhancer code for the 
             SDPT & PAO project

Author: Alexandru-Iulian Constantinescu
**************************************************************/
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <png.h>

unsigned short width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers = NULL;

void read_png_file(char *filename) {
  FILE *fp = fopen(filename, "rb");

  // allocate and initialize a  png_struct  structure for reading a PNG file
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) abort(); // from stdlib.h

  // allocate and initialize a  png_info  structure
  png_infop info = png_create_info_struct(png);
  if(!info) abort();

  // in case of unexpected errors
  if(setjmp(png_jmpbuf(png))) abort();

  // initialize input/output for the PNG file
  png_init_io(png, fp);

  // read the PNG image information
  png_read_info(png, info);

  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);

  if(bit_depth == 16)
      // strip 16 bit PNG file to 8 bit depth
      png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(png);
    
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand_gray_1_2_4_to_8(png);  
    
  

  // determine if chunk data is valid
  if(png_get_valid(png, info, PNG_INFO_tRNS))
      // tRNS chunks are expanded to alpha channels
      png_set_tRNS_to_alpha(png);

  // this color_type doesn't have an alpha channel --> fill it with 0xff.
  if(color_type == PNG_COLOR_TYPE_RGB ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    
   if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png);  
    

  // update png_info structure
  png_read_update_info(png, info);

  if (row_pointers) abort();
  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }

  png_read_image(png, row_pointers);

  fclose(fp);
  png_destroy_read_struct(&png, &info, NULL);
}



void write_png_file(char *filename) {
  FILE *fp = fopen(filename, "wb");
  if(!fp) abort();

  // allocate and initialize a  png_struct  structure for writing a PNG file  
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  // allocate and initialize a  png_info  structure  
  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  // in case of unexpected errors
  if (setjmp(png_jmpbuf(png))) abort();

  // initialize input/output for the PNG file
  png_init_io(png, fp);

  // Output is 8bit depth, RGBA format
  png_set_IHDR(
    png,
    info,
    width, height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);

  if (!row_pointers) abort();
  png_write_image(png, row_pointers);
  // write the end of a PNG file
  png_write_end(png, NULL);

  for(int y = 0; y < height; y++) {
    free(row_pointers[y]);
  }
  free(row_pointers);

  fclose(fp);

  png_destroy_write_struct(&png, &info);
}



void process_png_file() {
  unsigned char temp[height][width][3];
  short int temp2[height][width][3];
  
  clock_t start = clock();
  # pragma omp parallel for schedule(dynamic, 1)
  for(unsigned short y = 0; y < height; y++) {
      png_bytep row = row_pointers[y];
      for(unsigned short x = 0; x < width; x++) {
          png_bytep px = &(row[x * 4]);
          temp[y][x][0] = px[0];
          temp[y][x][1] = px[1];
          temp[y][x][2] = px[2];
          //printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);
    }
  }
    clock_t end = clock();
    float dif1 = difftime(end, start)/CLOCKS_PER_SEC;
    printf ("First for took %f seconds to run.\n", dif1);
    
   short int kernel[3][3] = {
       {0 , -1, 0},
       {-1, 5, -1},
       {0, -1, 0}
   };
    
    
    start = clock();
    # pragma omp parallel for 
    for(unsigned short i=1; i<height-1; ++i)
        for(unsigned short j=1; j<width-1; ++j)
            for(unsigned char k=0; k<3; ++k)
        {   
            temp2[i][j][k] = temp[i-1][j-1][k] * kernel[0][0] +
                            temp[i-1][j][k] * kernel[0][1] +
                            temp[i-1][j+1][k] * kernel[0][2] +
                            temp[i][j-1][k] * kernel[1][0] +
                            temp[i][j][k] * kernel[1][1] +
                            temp[i][j+1][k] * kernel[1][2] +
                            temp[i+1][j-1][k] * kernel[2][0] +
                            temp[i+1][j][k] * kernel[2][1] +
                            temp[i+1][j+1][k] * kernel[2][2];
             temp2[i][j][k] = temp2[i][j][k] > 255 ? 255:temp2[i][j][k];
             temp2[i][j][k] = temp2[i][j][k] < 0 ? 0 : temp2[i][j][k];
        }
    end = clock();
    float dif2 = difftime(end, start)/CLOCKS_PER_SEC;
    printf ("Second for took %f seconds to run.\n", dif2);
    
    start = clock();
    # pragma omp parallel for schedule(dynamic, 1)
    for(unsigned short y = 0; y < height; y++) {
        png_bytep row = row_pointers[y];
        for(unsigned short x = 0; x < width; x++) {
            png_bytep px = &(row[x * 4]);
            px[0] = (char) temp2[y][x][0];
            px[1] = (char) temp2[y][x][1];
            px[2] = (char) temp2[y][x][2];
        }
    }
    end = clock();
    float dif3 = difftime(end, start)/CLOCKS_PER_SEC;
    printf ("Third for took %f seconds to run.\n", dif3);
    printf ("Total time processing image: %f seconds\n", dif1 + dif2 + dif3);
}



int main(int argc, char** argv){
    // gcc code.c -lpng -fopenmp -O3 -o prog
    
    char* filename = "image_cat.png";
    //char * filename = argv[1];
    //char* filename = "image_dog.png";
    //char* filename = "image_lena.png";
    
    char* result_filename = "image_cat.png";
    //char * result_filename = argv[1];
    //char* result_filename = "image_dog.png";
    //char* result_filename = "image_lena.png";

    read_png_file(filename);
    process_png_file();
    write_png_file(result_filename);
    
    printf("Done processing image\n");
    return 0;
}
