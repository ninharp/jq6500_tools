/*

jq6500fs.c - JQ6500 Library to read and write internal flash memory

Copyright (c) 2017 Michael Sauer <sauer.uetersen@gmail.com>

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
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <scsi/sg.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <jq6500fs.h>
/* Private define ------------------------------------------------------------*/
#define ERASE 0xfbd8
#define WRITE 0xfbd9
#define BLKSZ 0x1000
#define TIMEOUT 30000
/* Private typedef -----------------------------------------------------------*/
struct jqcmd {
    uint16_t j_cmd;
    uint32_t j_off;
    uint32_t j_foo;
    uint32_t j_len;
    uint16_t j_bar;
} __attribute__((packed));
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
off_t _filesize(char *file);
off_t _isdevice(char *file);
void _w32le(uint8_t **loc, uint32_t val);
/* Private functions ---------------------------------------------------------*/
off_t _filesize(char *file)
{
    int result;
    struct stat buf;
    result = stat(file, &buf);
    if (result != 0) {
	err(1, "cannot stat %s", file);
    }
    return buf.st_size;
}

off_t _isdevice(char *file)
{
    int result;
    struct stat buf;
    result = stat(file, &buf);
    if (result != 0) {
	err(1, "cannot stat %s", file);
    }
    return (buf.st_rdev != 0);
}

void _w32le(uint8_t **loc, uint32_t val)
{
    memcpy(*loc, &htole32(val), 4);
    (*loc) += 4;
}
/* Public functions ----------------------------------------------------------*/
JQ6500_ERR_t jq6500_read_iso_F(char *infile, char *outfile)
{
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
	fseek(ptr_infile, 0, SEEK_END);
	fileLen = ftell(ptr_infile);
	fseek(ptr_infile, 0, SEEK_SET);

	//Allocate memory
	temp_buffer = (char *)malloc(fileLen+1);
	if (!temp_buffer)
	{
		fprintf(stderr, "Memory allocation Error!\n");
        fclose(ptr_infile);
		return(MEMORY_ERROR);
	}

	//Read file contents into buffer (first 256kib)
	fread(temp_buffer, 256, 1000, ptr_infile);
	//Write buffer contents into file
	int ss = fwrite(temp_buffer, 256, 1000, ptr_outfile);
	fclose(ptr_infile);
	fclose(ptr_outfile);

	if (ss != 0) {
		printf("%d Bytes\n", 256*(int)ss);
		return(NO_ERROR);
	} else {
		fprintf(stderr, "Input/Output Error!\n");
		return(IO_ERROR);
	}
	
	return(UNKNOWN_ERROR);
}

JQ6500_ERR_t jq6500_read_files_F(char *infile, char *outdir, int offset)
{
	FILE *ptr_infile;

	unsigned long fileLen;
	uint8_t *temp_buffer;

	ptr_infile = fopen(infile, "r");
	if (!ptr_infile)
	{
		if (!ptr_infile)
			fprintf(stderr, "Unable to open file '%s'!\n", infile);
		return(FILE_OPEN_ERROR);
	}

	//Get file length
	fseek(ptr_infile, 0, SEEK_END);
	fileLen = ftell(ptr_infile);
	fseek(ptr_infile, 0, SEEK_SET);
	printf("File Length: %0.2f MB\n", (float)fileLen/1024/1024);

	//Allocate memory
	temp_buffer = (uint8_t *)malloc(fileLen+1);
	if (!temp_buffer)
	{
		fprintf(stderr, "Memory allocation Error!\n");
        fclose(ptr_infile);
		return(MEMORY_ERROR);
	}

	fread(temp_buffer, fileLen, 1, ptr_infile);
	fclose(ptr_infile);

	if (jq6500_read_jqfs(temp_buffer, offset, fileLen, outdir) == NO_ERROR) {
		printf("Done.\n");
	}

	return(NO_ERROR);
}

uint32_t _c32(uint8_t *buf, int offset)
{
	uint32_t t1 = buf[offset+3]<<24;
	uint32_t t2 = buf[offset+2]<<16;
	uint32_t t3 = buf[offset+1]<<8;
	uint32_t t4 = buf[offset];
	return t1+t2+t3+t4;
}

JQ6500_ERR_t jq6500_read_jqfs(uint8_t *buf, int offset, int len, char *outdir)
{
	/*
	for (int i = 0; i < len; i++) {
		printf("0x%02x ", (uint8_t)buf[i]);
		if ((i % 16) == 0)
			printf("\n0x%06x ", i+1);
	}
	*/
	int *dir_offsets;
	int *file_offsets;
	int *file_sizes;
	int dir = 0;
	int count_dirs = _c32(buf, offset);
	dir_offsets = (int*)malloc(count_dirs*sizeof(uint32_t));
	printf("Count Directories: %d\n", count_dirs);
	offset += 4;
	for (int c = 0; c < count_dirs; c++) {
		int backup_offset = offset;
		dir_offsets[dir] = _c32(buf, offset);
		printf("Offset %d: 0x%06x\n", dir, dir_offsets[dir]);
		dir++;
		int count_files = buf[_c32(buf, offset)];
		file_offsets = (int*)malloc(count_files*sizeof(uint32_t));
		file_sizes = (int*)malloc(count_files*sizeof(uint32_t));
		printf("Offset %d Count Files: %d\n", dir, count_files);
		for (int cf = 0; cf < count_files; cf++) {
			file_offsets[cf] = _c32(buf, offset);
			printf("Offset %d (0x%06x) File %d Offset: 0x%06x\n", dir, dir_offsets[dir-1], cf+1, file_offsets[cf]);
			offset += 4;
			file_sizes[cf] = _c32(buf, offset);
			printf("Offset %d (0x%06x) File %d Size: %d bytes\n", dir, dir_offsets[dir-1], cf+1, file_sizes[cf]);
			FILE *ptr_outfile;
			char file[256];
			uint8_t *temp_buffer;
			temp_buffer = (uint8_t*)malloc(file_sizes[cf]*sizeof(uint8_t));
			for (int i = 0; i < file_sizes[cf]; i++) {
				temp_buffer[i] = buf[file_offsets[cf]+i];
			}
			sprintf(file, "%s/out_%d.mp3", outdir, cf+1);
			printf("Writing %s\n", file);
			ptr_outfile = fopen(file, "w+");
			fwrite(temp_buffer, file_sizes[cf], 1, ptr_outfile);
			fclose(ptr_outfile);
		}
		offset = backup_offset;
		offset += 4;
	}
	return(NO_ERROR);
}

/* jq6500 mkjqfs - Program the SPI flash of a JQ6500 MP3 player module.
 * Original Function Copyright (C) 2017 Reinhard Max <reinhard@m4x.de>
 */
int jq6500_make_jqfs(uint8_t *buf, int count, char **files, int offset, bool force)
{
    int size, i, len;
    uint8_t *dir, *data;
    memset(buf, 0xff, FLASHSIZE);

    dir = buf;
    /* 2 for main dir, 1 for length of subdir, 2 for each file */
    size = (2 + 1 + 2 * count) * sizeof(uint32_t);
    data = dir + size;

    _w32le(&dir, 1);		/* Number of Subdirs */
    _w32le(&dir, offset + 8);	/* Offset of first subdir */
    _w32le(&dir, count);		/* Number of files in subdir 1 */

    for (i = 0; i < count; i++) {
		int fd;
		char *fname = files[i];
		len = _filesize(fname);
		size += len;
		if (!force && size >= MAXSIZE) {
		    errx(1, "length %d exceeds maximum of %d\n", size, MAXSIZE);
		}
		fd = open(fname, O_RDONLY);
		if (fd < 0) 
		    err(1, "cannot open %s", fname);
		if (read(fd, data, len) != len) 
		    err(1, "cannot read %s", fname);
		close(fd);
		_w32le(&dir, offset + data - buf); /* File offset */
		_w32le(&dir, len);                 /* File length */
		data += len;
    }
    return size;
}

/* jq6500 jqwrite - Program the SPI flash of a JQ6500 MP3 player module.
 * Original Function Copyright (C) 2017 Reinhard Max <reinhard@m4x.de>
 */
void jq6500_write_buf_D(char *device, uint8_t *buf, int offset, int size)
{
    int dev, i;

    dev = open(device, O_RDWR);
    if (dev < 0)
	err(1, "cannot open device %s", device);

    fprintf(stderr, "Uploading %d bytes...\n", size);

    for (i = 0; i < size; i += BLKSZ) {
	
		struct jqcmd cmd;
		struct sg_io_hdr hdr;
		//
		memset(&cmd, 0, sizeof(cmd));
		cmd.j_cmd = htobe16(ERASE);
		cmd.j_off = htobe32(offset + i);

		memset(&hdr, 0, sizeof(hdr));
		hdr.interface_id = 'S';
		hdr.timeout = TIMEOUT;
		hdr.cmdp = (unsigned char *) &cmd;
		hdr.cmd_len = sizeof(cmd);
		hdr.dxfer_direction = SG_DXFER_NONE;

		if (ioctl(dev, SG_IO, &hdr) < 0) {
		     fprintf(stderr, "\n");
		     err(1, "erase failed at offset %x", offset + i);
		}

		memset(&cmd, 0, sizeof(cmd));
		cmd.j_cmd = htobe16(WRITE);
		cmd.j_off = htobe32(offset + i);
		cmd.j_len = htobe32(BLKSZ);

		memset(&hdr, 0, sizeof(hdr));
		hdr.interface_id = 'S';
		hdr.timeout = TIMEOUT;
		hdr.cmdp = (unsigned char *) &cmd;
		hdr.cmd_len = sizeof(cmd);
		hdr.dxfer_direction = SG_DXFER_TO_DEV;
		hdr.dxferp = buf + i;
		hdr.dxfer_len = BLKSZ;
		
		if (ioctl(dev, SG_IO, &hdr) < 0) {
		    fprintf(stderr, "\n");
		    err(1, "\nwrite failed at offset %x", offset + i);
		}
    }
    //
    fprintf(stderr, "\n");
    close(dev);
}


