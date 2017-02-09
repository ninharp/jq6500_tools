/*

jq6500fs.c - JQ6500 Library to read and write internal flash memory

Copyright (c) 2017 Michael Sauer <sauer@gmail.com>

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>

#include <jq6500fs.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
JQ6500_ERR_t jq6500_read_iso(char *infile, char *outfile) {
#ifdef DEGUG
	printf("Reading ISO from '%s' to '%s'\n", infile, outfile);
#endif
	FILE *ptr_infile;
	FILE *ptr_outfile;

	unsigned long fileLen;
	char *temp_buffer;

	ptr_infile = fopen(infile, "r");
	ptr_outfile = fopen(outfile,"w+");
	if (!ptr_infile && !ptr_outfile)
	{
		if (!ptr_infile)
			fprintf(stderr, "Unable to open file '%s'!\n", infile);
		if (!ptr_outfile)
			fprintf(stderr, "Unable to open file '%s'!\n", outfile);
		return(FILE_OPEN_ERROR);
	}

	//Get file length
	fseek(ptr_outfile, 0, SEEK_END);
	fileLen = ftell(ptr_infile);
	fseek(ptr_outfile, 0, SEEK_SET);

	//Allocate memory
	temp_buffer = (char *)malloc(fileLen+1);
	//temp_buffer = (char *)malloc((256*1024)+1);
	if (!temp_buffer)
	{
		fprintf(stderr, "Memory error!");
        fclose(ptr_outfile);
		return(MEMORY_ERROR);
	}

	//Read file contents into buffer
	fread(temp_buffer, (256*1024), 1, ptr_infile);

	fwrite(temp_buffer, (256*1024), 1, ptr_outfile);
	//fclose(ptr_infile);
	//fclose(ptr_outfile);

	/*for (int c = 0; c < 256; c++)
	{
		fread(temp_buffer[c], sizeof(temp_buffer[c]), 1, ptr_infile);
		//fwrite(temp_buffer, sizeof(temp_buffer), 1, ptr_outfile);
		printf(".");
		//printf("%d of 1024 Blocks read!\n", c);
	}
	printf("\n");
	fclose(ptr_infile);
	for (int c = 0; c < 256; c++)
	{
		fwrite(temp_buffer[c], sizeof(temp_buffer[c]), 1, ptr_outfile);
		printf(".");
		//printf("%d of 1024 Blocks read!\n", c);
	}
	*/
	return(NO_ERROR);
}