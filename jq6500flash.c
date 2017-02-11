/*

jq6500flash.c - JQ6500 Tool to read and write the JQ6500 internal flash memory

This is a tool to read and write the internal flash memory of the JQ6500 MP3 
Sound Chip. Originally it comes with a creepy Windows Software on Chinese which is
very spooky, but some reverse engeneering of the file storage system offers a simple
solution to make an own tool for that purposes.

Its currently in alpha stage and still quiet buggy.

Copyright (c) 2017 Michael Sauer <sauer@gmail.com> 
Some adaptions of the JQFS Functions 
Copyright (C) 2017 Reinhard Max <reinhard@m4x.de>

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
#include <errno.h>
#include <err.h>

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
    char *infile = NULL;
    char *outfile = NULL;
    char *outdir = NULL;
    char *endptr;
    int offset = BASE;
    int raw = 0;
    int read_iso = 0;
    int read_files = 0;

    //Specifying the expected options
    while ((option = getopt(argc, argv, "f:s:i:r:R:h")) != -1) {
        switch (option) {
             case 's' : errno = 0;
                        offset = strtol(optarg, &endptr, 0);
                        if (errno != 0 || *optarg == 0 || *endptr == 0) {
                            errx(1, "Invalid number in offset: '%s'", optarg);
                        }
                        break;
             case 'i' : outfile = optarg;
                        read_iso = 1;
                        break;
             case 'f' : infile = optarg; 
                 		break;
             case 'r' : outdir = optarg;
                        read_files = 1;
                 		break;
             case 'R' : outfile = optarg;
                        raw = 1;
                        break;
             case 'h' : print_usage();
                        exit(EXIT_FAILURE);
                        break;
             default: print_usage(); 
                      exit(EXIT_FAILURE);
        }
    }
    if (infile == NULL) {
        print_usage();
        exit(EXIT_FAILURE);
    } else {
        printf("Using Device/File: %s\n", infile);
        printf("Offset: 0x%x\n", offset);
        if (outdir != NULL)
            printf("Output Directory: %s\n", outdir);

        if (read_iso == 1 && raw == 0 && read_files == 0) {
            if (outfile == NULL) {
                printf("No outfile specified for ISO creation\n");
                exit(EXIT_FAILURE);
            }
            printf("Downloading ISO from '%s' to '%s'\n", infile, outfile);
            if (jq6500_read_iso_F(infile, outfile) == NO_ERROR)
            {
                printf("Done.\n");    
            } else {
                fprintf(stderr, "I/O Error\n\n");
                exit(EXIT_FAILURE);
            }
        } else if (read_iso == 0 && raw == 1 && read_files == 0) {
            printf("RAW Mode enabled!\n");
        } else if (read_iso == 0 && raw == 0 && read_files == 1) {
            printf("Read out JQFS to Directory...\n");
            jq6500_read_files_F(infile, outdir, offset);
        } else {
            print_usage();
            exit(EXIT_FAILURE);
        }
    	exit(EXIT_SUCCESS);
    }
}

/** 
	print_usage()

	Prints out the usage information on command line!
 */
void print_usage()
{
    printf("Usage: jq6500flash [options]\n\n"
        "-f device/file    File or SCSI Device with JQFS\n"
        "-s address        Start Address to write to (Default: 0x%x)\n"
        "-i file           Read out ISO from JQFS to file\n"
        "-r dir            Read out JQFS to directory\n"
        "-R file           Write file in Raw mode to device\n"
        "-h                This help\n\n", BASE);
}
