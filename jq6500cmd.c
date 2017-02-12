/*

jq6500cmd.c - JQ6500 Tool to send commands to the JQ6500 serial interface

This tool sends out the commands to the JQ6500 chip

Its currently in alpha stage and still quiet buggy.

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
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MODEMDEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define MAX_CMD_CHARS 16
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FILE * input;
int status;
char Key;

volatile bool STOP = false;
int wait_flag = true;
/* Private function prototypes -----------------------------------------------*/
void print_usage();
long get_baudrate(long baudrate);
void signal_handler_IO(int status);
/* Public functions ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*
Main Routine
*/
int main (int argc, char *argv[])
{
   int fd;
   struct termios oldtio, newtio;
   struct sigaction saio;               //definition of signal action
   char buf[255];
   int option = 0;
   char *devicename = NULL;
   long baudrate = 9600;
   char *endptr;
   
   printf("JQT6500 Command Tool " VERSION " - (c) by Michael Sauer\n\n");

   if (argc == 1) {
      print_usage();
      exit(EXIT_FAILURE);   
   }

   //Specifying the expected options
   while ((option = getopt(argc, argv, "d:b::h")) != -1) {
      switch (option) {
         case 'd' :  errno = 0;
                     devicename = optarg;
                     break;
         case 'b' :  errno = 0;
                     baudrate = strtol(optarg, &endptr, 0);
                     /*if (errno != 0 || *optarg == 0 || *endptr == 0) {
                         errx(1, "Invalid number in baudrate: '%s'", optarg);
                     }*/
                     break;
         case 'h' :  print_usage();
                     exit(EXIT_FAILURE);
                     break;
         default: print_usage(); 
                  exit(EXIT_FAILURE);
      }
   }

   if (devicename == NULL) {
      print_usage();
      exit(EXIT_FAILURE);   
   }

   if (get_baudrate(baudrate) == 0) {
      printf("Error: Unknown Baudrate: %ld\n\n", baudrate);
      print_usage();
      exit(EXIT_FAILURE);
   }

   printf("Using device: %s\n", devicename);
   printf("Baudrate: %ld baud\n", baudrate);

   /*
    Open modem device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */
   //open the device(com port) to be non-blocking (read will return immediately)
   fd = open(devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);
   if (fd < 0)
   {
      perror(devicename);
      exit(EXIT_FAILURE);
   }

   //install the serial handler before making the device asynchronous
   saio.sa_handler = signal_handler_IO;
   sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
   saio.sa_flags = 0;
   saio.sa_restorer = NULL;
   sigaction(SIGIO, &saio, NULL);

   // allow the process to receive SIGIO
   fcntl(fd, F_SETOWN, getpid());
   // Make the file descriptor asynchronous (the manual page says only
   // O_APPEND and O_NONBLOCK, will work with F_SETFL...)
   fcntl(fd, F_SETFL, FASYNC);

   tcgetattr(fd, &oldtio); // save current port settings
   // set new port settings for canonical input processing
   newtio.c_cflag = get_baudrate(baudrate) | CS8 | CLOCAL | CREAD;
   newtio.c_iflag = IGNPAR;
   newtio.c_oflag = 0;
   newtio.c_lflag = 0;       //ICANON;
   newtio.c_cc[VMIN] = 1;
   newtio.c_cc[VTIME] = 0;
   tcflush(fd, TCIFLUSH);
   tcsetattr(fd, TCSANOW, &newtio);


   memset(buf, 0, 255);
   //write(fd, "halt\r", 5);
   
   /*
   //write(fd, "head\r\n", 6);
   int lines = 0;
   // STop here for testing purposes
   //STOP = false;
   while (STOP == false)
   {
      // after receiving SIGIO, wait_flag = FALSE, input is available and can be read
      if (wait_flag == true)  //if input is available
      {
         res = read(fd, buf, 255);
         //buf[res] = 0;
         printf("%s\r\n", buf);
         //STOP == TRUE;
         if (res==1) STOP=true; // stop loop if only a CR was input
         wait_flag = true; // wait for new input
         if (res > 0)
            lines++;
         if (lines >= 1)
         {
            printf("\r\n");
            exit(0);
         }
      }  //end if wait flag == FALSE

   }  //while stop==FALSE
      //
   */
   /* restore the old port settings */
   tcsetattr(fd, TCSANOW, &oldtio);
   return(EXIT_SUCCESS);
}

void print_usage()
{
   printf("Usage: jq6500cmd [options]\n\n"
        "-d device         Serial device to use\n"
        "-b baudrate       Serial baudrate (default: 9600)\n"
        "-h                This help\n\n");
}

long get_baudrate(long baudrate)
{
   long BAUD = 0;
   switch (baudrate)
      {
         case 115200:
            BAUD = B115200;
            break;
         case 38400:
            BAUD = B38400;
            break;
         case 19200:
            BAUD  = B19200;
            break;
         case 9600:
            BAUD  = B9600;
            break;
         case 4800:
            BAUD  = B4800;
            break;
         case 2400:
            BAUD  = B2400;
            break;
         case 1800:
            BAUD  = B1800;
            break;
         case 1200:
            BAUD  = B1200;
            break;
         case 600:
            BAUD  = B600;
            break;
         case 300:
            BAUD  = B300;
            break;
         case 200:
            BAUD  = B200;
            break;
         case 150:
            BAUD  = B150;
            break;
         case 134:
            BAUD  = B134;
            break;
         case 110:
            BAUD  = B110;
            break;
         case 75:
            BAUD  = B75;
            break;
         case 50:
            BAUD  = B50;
            break;
      }  //end of switch baud_rate
   return BAUD;
}

/***************************************************************************
* signal handler. sets wait_flag to FALSE, to indicate above loop that     *
* characters have been received.                                           *
***************************************************************************/

void signal_handler_IO (int status)
{
   //printf("received SIGIO signal.\n");
   wait_flag = false;
}

