#include <xtensa/tie/portqueue6o.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include "datatype.h"
#include "jdatatype.h"
#include "config.h"
#include "prototype.h"
#include "encoder.h"

//int main (void){
int main(int argc, char *argv[]){

	INT8 input_file_name [150], output_file_name [150];
	INT8 *input_file_name_ptr, *output_file_name_ptr;
	UINT32 image_size;
	UINT8 *input , *output ;
	FILE *fp, *fpt;

	UINT8 *output_ptr;
	UINT32 quality_factor, image_format, image_width, image_height;


	fp = fopen ("param.txt", "r");
	printf("File Open @ %d \n",clock());
	while (fscanf (fp, "%s", input_file_name) != EOF)
	{
		input_file_name_ptr = input_file_name;
		output_file_name_ptr = output_file_name;

		//while ((*output_file_name_ptr++ = *input_file_name_ptr++) != '.');

		// *output_file_name_ptr++ = 'j';
		// *output_file_name_ptr++ = 'p';
		// *output_file_name_ptr++ = 'g';
		// *output_file_name_ptr++ = '\0';
		
		while(*input_file_name_ptr != '.'){
			SEND6(*input_file_name_ptr);
			input_file_name_ptr++;
		}
		SEND6('\0');

		fscanf (fp, "%d", &quality_factor);
		fscanf (fp, "%d", &image_format);
		fscanf (fp, "%d", &image_width);
		fscanf (fp, "%d", &image_height);

		if (image_format == FOUR_ZERO_ZERO)
			image_size = image_width * image_height;
		else if (image_format == FOUR_TWO_ZERO)
			image_size = (image_width * image_height * 3) >> 1;
		else if (image_format == FOUR_TWO_TWO)
			image_size = image_width * image_height * 2;
		else
			image_size = image_width * image_height * 3;

		input=(UINT8 *)jemalloc(2500000*sizeof(UINT8));
		fpt = fopen (input_file_name, "rb");
		fread (input, 1, image_size, fpt);
		fclose (fpt);
		//output=(UINT8 *)jemalloc(250000*sizeof(UINT8*));
		//output_ptr = output;

		output_ptr = encode_image (input, output_ptr, quality_factor, image_format, image_width, image_height);
 
		//fpt = fopen (output_file_name, "wb");
		//fwrite (output, 1, output_ptr - output, fpt);
		//fclose (fpt);
		free (input);
		//free (output);
		//fprintf(stderr,"Waiting now\n");
		//xt_iss_profile_disable();
		//while(!ISDONE());
		//xt_iss_profile_enable();
	}

	SEND6(0);

	fclose (fp);
	
	//while(1);
	printf("\n Done ! \n");
	return 0;
}

void* jemalloc(size_t size)
{
	return (malloc(size));
}
