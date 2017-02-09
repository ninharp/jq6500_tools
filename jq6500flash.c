/*

jq6500flash.c - JQ6500 Tool to read and write the JQ6500 internal flash memory

This is a tool to read and write the internal flash memory of the JQ6500 MP3 
Sound Chip. Originally it comes with a creepy Windows Software on Chinese which is
very spooky, but some reverse engeneering of the file storage system offers a simple
solution to make an own tool for that purposes.

Its currently in alpha stage and still quiet buggy.

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
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>

#include <jq6500fs.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void print_usage();
/* Public functions ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*
Main Routine
*/
int main(int argc, char *argv[])
{
	printf("JQT6500 Flash Tool " VERSION " - (c) by Michael Sauer\n\n");

	int option = 0;
    int type = -1;
    char *infile = NULL;
    char *outfile = NULL;
    char *direction = NULL;

    //Specifying the expected options
    while ((option = getopt(argc, argv, "f:o:t:d:")) != -1) {
        switch (option) {
             case 't' : type = atoi(optarg);
                 		break;
             case 'f' : infile = optarg; 
                 		break;
             case 'o' : outfile = optarg;
                 		break;
             case 'd' : direction = optarg;
                 		break;
             default: print_usage(); 
                 exit(EXIT_FAILURE);
        }
    }
    if (infile == NULL || outfile == NULL || type == -1 || direction == NULL) {
        goto ERROR_USAGE;
    } else {
    	if (direction[0] == 'r') {
    		printf("Downloading %s from '%s' to '%s'\n", type==0 ? "ISO File" : "MP3 File", infile, outfile);
    		if (jq6500_read_iso(infile, outfile) != NO_ERROR)
    			goto ERROR_READING;
    	} else if (direction[0] == 'w') {
    		printf("Uploading %s '%s' to '%s'\n", type==0 ? "ISO File" : "MP3 File", infile, outfile);
    	} else {
    		goto ERROR_USAGE;
    	}
    	exit(EXIT_SUCCESS);
    }

ERROR_READING:
	fprintf(stderr, "I/O Error\n\n");
	exit(EXIT_FAILURE);

ERROR_USAGE:
	print_usage();
	exit(EXIT_FAILURE);
}

/** 
	print_usage()

	Prints out the usage information on command line!
 */
void print_usage()
{
    printf("Usage: jq6500flash -t [type] -d [r/w] -f [file] -o [file]\n");
}
