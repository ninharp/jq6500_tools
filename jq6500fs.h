/*

jq6500fs.h - JQ6500 Library to read and write internal flash memory

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

#ifndef JQ6500FS_H_
#define JQ6500FS_H_

/* Includes -------------------------------------------------------------------*/
#include <stdio.h>
/* Exported define ------------------------------------------------------------*/
#define DEBUG
/* Exported typedef -----------------------------------------------------------*/
typedef enum {
	NO_ERROR = 0,
	READ_ERROR,
	MEMORY_ERROR,
	FILE_OPEN_ERROR,
	UNKNOWN_ERROR
} JQ6500_ERR_t;

/* Exported macro -------------------------------------------------------------*/
/* Exported variables ---------------------------------------------------------*/
/* Exported function prototypes -----------------------------------------------*/
JQ6500_ERR_t jq6500_read_iso(char *filename, char *infile);

#endif // JQ6500FS_H_