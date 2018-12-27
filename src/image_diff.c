/*
 * use libjpeg to read an write jpeg format file.
 * 
 * usage:
 * gcc -o jpeg_sample jpeg_sample.c -ljpeg
 * ./jpeg_sample
 */

#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <string.h>

unsigned char *raw_image_1 = NULL;
unsigned char *raw_image_2 = NULL;
unsigned char *diff_image = NULL;

int width = 640;
int height = 480;
int bytes_per_pixel = 3;   // or 1 for GRACYSCALE images
int color_space = JCS_RGB; // or JCS_GRAYSCALE for grayscale images
/*
*/

int read_jpeg_file( char *filename, int image_id )
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];
	
	FILE *infile = fopen( filename, "rb" );
	unsigned long location = 0;
	int i = 0;
	
	if ( !infile )
	{
		printf("Error opening jpeg file %s\n!", filename );
		return -1;
	}
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &cinfo );
	jpeg_stdio_src( &cinfo, infile );
	jpeg_read_header( &cinfo, TRUE );
	jpeg_start_decompress( &cinfo );

	if(image_id == 1) {
		raw_image_1 = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	}
	if(image_id == 2) {
		raw_image_2 = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
		diff_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	}

	row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );

	while( cinfo.output_scanline < cinfo.image_height )
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
		for( i=0; i<cinfo.image_width*cinfo.num_components;i++) {

			if(image_id == 1) {
				raw_image_1[location] = row_pointer[0][i];
			}
			if(image_id == 2) {
				raw_image_2[location] = row_pointer[0][i]/2;

				// diff_image[location] = abs(raw_image_2[location] - raw_image_1[location]);
				// location++;
				// if( (i+1) % 3 == 0) {
				// 	// $diff_avg = (abs($r1 - $r2) + abs($g1 - $g2) + abs($b1 - $b2)) / 3;
				// }
			}
			location++;
		}
	}

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	free( row_pointer[0] );
	fclose( infile );

	return 1;
}

int read_jpeg_file2( char *filename1, char *filename2, int sensitiviy, int cluster_square_size, float cluster_threshold_factor )
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];
	
	FILE *infile1 = fopen( filename1, "rb" );
	FILE *infile2 = fopen( filename2, "rb" );
	unsigned long location1 = 0;
	unsigned long location2 = 0;
	
	if ( !infile1 )
	{
		printf("Error opening jpeg file %s\n!", filename1 );
		return -1;
	}
	if ( !infile2 )
	{
		printf("Error opening jpeg file %s\n!", filename2 );
		return -1;
	}

	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &cinfo );
	jpeg_stdio_src( &cinfo, infile1 );
	jpeg_read_header( &cinfo, TRUE );
	jpeg_start_decompress( &cinfo );

	raw_image_1 = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );

	while( cinfo.output_scanline < cinfo.image_height )
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
		for( int i=0; i<cinfo.image_width*cinfo.num_components;i++) {
			raw_image_1[location1] = row_pointer[0][i];
			location1++;
		}
	}

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	free( row_pointer[0] );
	fclose( infile1 );




	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &cinfo );
	jpeg_stdio_src( &cinfo, infile2 );
	jpeg_read_header( &cinfo, TRUE );
	jpeg_start_decompress( &cinfo );

	raw_image_2 = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	diff_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	width = cinfo.output_width;
	height = cinfo.output_height;
	
	row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );

	while( cinfo.output_scanline < cinfo.image_height )
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
		for( int i=0; i<cinfo.image_width*cinfo.num_components;i++) {
			raw_image_2[location2] = row_pointer[0][i];
			diff_image[location2] = abs(row_pointer[0][i] - raw_image_1[location2]);
			location2++;
		}
	}

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	free( row_pointer[0] );
	fclose( infile2 );


	// Make diff image green if different
	for(int i = 0; i < cinfo.image_width * cinfo.image_height * cinfo.num_components; i += cinfo.num_components ) {
		int diff_avg = (diff_image[i] + diff_image[i+1] + diff_image[i+2]) / 3;
		if( diff_avg > sensitiviy ) {
			diff_image[i] = 0;
			diff_image[i+1] = 255;
			diff_image[i+2] = 0;
		} else {
			diff_image[i] = 0;
			diff_image[i+1] = 0;
			diff_image[i+2] = 0;
		}
	}


	int cluster_hit_counter = 0;

	for (int x = 0; x < cinfo.image_width - cluster_square_size; x += cluster_square_size/2) {
		for (int y = 0; y < cinfo.image_height - cluster_square_size; y += cluster_square_size/2) {

			// Avoid the upper left corner due to the time counter there
			if(x > 240 || y > 20) {

			    	int diff_pixels_count = 0;

			    	// loop the cluster square
					for (int square_x = 0; square_x <= cluster_square_size; square_x++) {
					    for (int square_y = 0; square_y <= cluster_square_size; square_y++) {

					    	if(diff_image[(x+square_x + (y+square_y)*cinfo.image_width)*3 + 1] == 255) {
					    		diff_pixels_count++;
					    	}
					        
						}
					}


					// are there enough number of pixels with diff in this square to consider it a "hit" ?
					if(diff_pixels_count > cluster_square_size * cluster_square_size * cluster_threshold_factor) {

						cluster_hit_counter++;

						// draw a square around the cluster
						for (int square_x = 0; square_x <= cluster_square_size; square_x++) {
						    for (int square_y = 0; square_y <= cluster_square_size; square_y++) {

						    	if(square_x == 0 || square_x == cluster_square_size) {
							    	raw_image_2[(x+square_x + (y+square_y)*cinfo.image_width)*3 + 0] = 255;
							    	raw_image_2[(x+square_x + (y+square_y)*cinfo.image_width)*3 + 1] = 0;
							    	raw_image_2[(x+square_x + (y+square_y)*cinfo.image_width)*3 + 2] = 0;
						    	}

						    	if(square_y == 0 || square_y == cluster_square_size) {
							    	raw_image_2[(x+square_x + (y+square_y)*cinfo.image_width)*3 + 0] = 255;
							    	raw_image_2[(x+square_x + (y+square_y)*cinfo.image_width)*3 + 1] = 0;
							    	raw_image_2[(x+square_x + (y+square_y)*cinfo.image_width)*3 + 2] = 0;
						    	}
						        
							}
						}
					}

			}


		}
	}



	return cluster_hit_counter;
}


int write_jpeg_file( char *filename )
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	JSAMPROW row_pointer[1];
	FILE *outfile = fopen( filename, "wb" );
	
	if ( !outfile )
	{
		printf("Error opening output jpeg file %s\n!", filename );
		return -1;
	}
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);


	cinfo.image_width = width;	
	cinfo.image_height = height;
	cinfo.input_components = bytes_per_pixel;
	cinfo.in_color_space = color_space;

	jpeg_set_defaults( &cinfo );

	jpeg_start_compress( &cinfo, TRUE );

	while( cinfo.next_scanline < cinfo.image_height )
	{
		row_pointer[0] = &raw_image_2[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines( &cinfo, row_pointer, 1 );
	}

	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
	fclose( outfile );

	return 1;
}




// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}




int main(int argc, char* argv[]) {
	char *infilename1 = "";
	char *infilename2 = "";
	char *outfilename = "";

	int sensitiviy = 20;
	int cluster_square_size = 12;
	float cluster_threshold_factor = 0.5;

	int verbose = 0;

	for (int arg = 0; arg < argc; ++arg)
    {
    	if(strcmp(argv[arg], "-f1") == 0) {
			infilename1 = argv[arg+1];
    	}
    	if(strcmp(argv[arg], "-f2") == 0) {
			infilename2 = argv[arg+1];
    	}
    	if(strcmp(argv[arg], "-v") == 0) {
    		verbose = 1;
			printf("verbose mode started\n");
    	}
    	if(strcmp(argv[arg], "-s") == 0) {
    		sensitiviy = atoi(argv[arg+1]);
    	}
    	if(strcmp(argv[arg], "-c") == 0) {
    		cluster_square_size = atoi(argv[arg+1]);
    	}
    	if(strcmp(argv[arg], "-t") == 0) {
    		cluster_threshold_factor = atof(argv[arg+1]);
    	}
    }

	if(verbose == 1) {
		printf("infilename1: %s\n", infilename1);
		printf("infilename2: %s\n", infilename2);
		printf("sensitiviy: %d\n", sensitiviy);
		printf("cluster_square_size: %d\n", cluster_square_size);
		printf("cluster_threshold_factor: %f\n", cluster_threshold_factor);
	}

	outfilename = str_replace(infilename2, ".jpg", "_diff.jpg");

	int cluster_hit_counter = read_jpeg_file2( infilename1, infilename2, sensitiviy, cluster_square_size, cluster_threshold_factor );
	if( cluster_hit_counter < 0 ) {
		printf("{\"error\":\"failed to compare images\"}");
		return -1;
	}

	if(cluster_hit_counter > 0) {
		if( write_jpeg_file( outfilename ) < 0 ) {
			printf("{\"error\":\"failed to write difference image\"}");
			return -1;
		}
	}

	if(cluster_hit_counter > 0) {
		printf("{\"cluster_hit_counter\":%d, \"difference_image\":\"%s\"}", cluster_hit_counter, outfilename);
	} else {
		printf("{\"cluster_hit_counter\":%d}", cluster_hit_counter);
	}

	return 0;
}

