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
#include <getopt.h>

#include <jq6500serial.h>

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define MAX_CMD_CHARS 16
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FILE * input;
int status;
char Key;

volatile bool STOP = false;
static int verbose_flag;
int wait_flag = true;
/* Private function prototypes -----------------------------------------------*/
char* create_command(char command, int argc, int arg1, int arg2);
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
   int fd, res;
   struct termios oldtio, newtio;
   struct sigaction saio;               //definition of signal action
   char buf[255];
   char response[16];
   int option = 0;
   int option_index = 0;
   char *devicename = NULL;
   long baudrate = 9600;
   char *endptr;
   int send_command = 0;
   int command_len = 0;
   char cmd = 0;
   char command[6] = "";
   
   printf("JQT6500 Command Tool " VERSION " - (c) by Michael Sauer\n\n");

   if (argc == 1) {
      print_usage();
      exit(EXIT_FAILURE);   
   }

   static struct option long_options[] =
   {
       {"verbose",   no_argument,         &verbose_flag, 1},
       {"brief",     no_argument,         &verbose_flag, 0},
       {"play",      no_argument,         0, 'p'},
       {"pause",     no_argument,         0, 's'},
       {"next",      no_argument,         0, 'N'},
       {"prev",      no_argument,         0, 'P'},
       {"vol-up",    no_argument,         0, 'U'},
       {"vol-down",  no_argument,         0, 'D'},
       {"fwversion", no_argument,         0, 'V'},
       {"next-dir",  no_argument,         0, 'F'},
       {"prev-dir",  no_argument,         0, 'G'},
       {"baud",      required_argument,   0, 'b'},
       {"device",    required_argument,   0, 'd'},
       {0, 0, 0, 0}
   };

   //Specifying the expected options
   //while ((option = getopt(argc, argv, "d:b::psNP+-h")) != -1) {
   while ((option = getopt_long (argc, argv, "d:b::psNPUDVh", long_options, &option_index)) != -1) {
      switch (option) {
         case 'd' :  devicename = optarg;
                     break;
         case 'b' :  baudrate = strtol(optarg, &endptr, 0);
                     /*if (errno != 0 || *optarg == 0 || *endptr == 0) {
                         errx(1, "Invalid number in baudrate: '%s'", optarg);
                     }*/
                     break;
         case 'p' :  send_command = 1;
                     cmd = JQ6500_CTRL_PLAY;
                     strcpy(command, create_command(cmd, 0, 0, 0));
                     break;
         case 's' :  send_command = 1;
                     cmd = JQ6500_CTRL_PAUSE;
                     strcpy(command, create_command(cmd, 0, 0, 0));
                     break;
         case 'N' :  send_command = 1;
                     cmd = JQ6500_CTRL_NEXT;
                     strcpy(command, create_command(cmd, 0, 0, 0));
                     break;
         case 'P' :  send_command = 1;
                     cmd = JQ6500_CTRL_PREV;
                     strcpy(command, create_command(cmd, 0, 0, 0));
                     break;
         case 'U' :  send_command = 1;
                     cmd = JQ6500_CTRL_VOL_UP;
                     strcpy(command, create_command(cmd, 0, 0, 0));
                     break;
         case 'D' :  send_command = 1;
                     cmd = JQ6500_CTRL_VOL_DOWN;
                     strcpy(command, create_command(cmd, 0, 0, 0));
                     break;
         case 'V' :  send_command = 1;
                     cmd = JQ6500_QUERY_GET_VER;
                     strcpy(command, create_command(cmd, 0, 0, 0));
                     break;
         case 'F' :  send_command = 1;
                     cmd = JQ6500_CTRL_CHG_FOLDER;
                     strcpy(command, create_command(cmd, 1, JQ6500_NEXT_FOLDER, 0));
                     break;
         case 'G' :  send_command = 1;
                     cmd = JQ6500_CTRL_CHG_FOLDER;
                     strcpy(command, create_command(cmd, 1, JQ6500_PREV_FOLDER, 0));
                     break;
         case 'h' :  print_usage();
                     exit(EXIT_FAILURE);
      }
   }

   if (devicename == NULL) {
      print_usage();
      exit(EXIT_FAILURE);   
   }

   if (get_baudrate(baudrate) == 0) {
      printf("Error: Unknown Baudrate: %ld\n\n", baudrate);
      exit(EXIT_FAILURE);
   }

   if (send_command == 0) {
      printf("Error: No Command specified!\n\n");
      exit(EXIT_FAILURE);  
   }

   printf("Using device: %s\n", devicename);
   printf("Baudrate: %ld baud\n\n", baudrate);

   if (verbose_flag)
      printf("Command String: ");
   for (int i = 0; i < 6; i++) {
      if (command[i] == 0x00) {
         command_len = i;
         if (verbose_flag)
            printf("\n");
         break;
      } else {
         if (verbose_flag)
            printf("0x%02X ", (uint8_t)command[i]);
      }
   }

   switch (cmd) {
      case JQ6500_CTRL_PLAY: printf("Sending Command 'Play'\n"); break;
      case JQ6500_CTRL_PAUSE: printf("Sending Command 'Pause'\n"); break;
      case JQ6500_CTRL_PREV: printf("Sending Command 'Previous Track'\n"); break;
      case JQ6500_CTRL_NEXT: printf("Sending Command 'Next Track'\n"); break;
      case JQ6500_CTRL_VOL_UP: printf("Sending Command 'Volume Up'\n"); break;
      case JQ6500_CTRL_VOL_DOWN: printf("Sending Command 'Volume Down'\n"); break;
      case JQ6500_QUERY_GET_VER: printf("Querying Version Information\n"); break;
   }

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
   memset(response, 0, 16);
   write(fd, command, command_len);
   
   int STOP = false;
   int c = 0;

   while (STOP == false)
   {
      // after receiving SIGIO, wait_flag = FALSE, input is available and can be read
      if (wait_flag == true)  //if input is available
      {
         res = read(fd, buf, 1);
         if (res == 1) {
            response[c++] = buf[0];
            if (verbose_flag) printf("%d 0x%02X\n", c, buf[0]);
         }

         if (strcmp(response, "OK") == 0) {
            printf("OK received!\n");
            memset(response, 0, 16);
            STOP = true;
         }
         if (strcmp(response, "ERROR") == 0) {
            printf("Error received!\n");
            memset(response, 0, 16);
            STOP = true;
         }
         if (strcmp(response, "STOP") == 0) {
            printf("Stop received!\n");
            memset(response, 0, 16);
            STOP = true;
         }

         if (cmd == JQ6500_QUERY_GET_VER && c == 4) {
            printf("Version %s\n", response);
            STOP = true;
         }
         wait_flag = true; // wait for new input
      }

   }  //while stop==FALSE

   /* restore the old port settings */
   tcsetattr(fd, TCSANOW, &oldtio);
   return(EXIT_SUCCESS);
}

char* create_command(char command, int argc, int arg1, int arg2)
{
   
   char *ret = (char*)malloc(argc*sizeof(char)+1);
   if (argc == 0)
      sprintf(ret, "%c%c%c%c", JQ6500_CMD_PREFIX, 2, command, JQ6500_CMD_TERM);
   else if (argc == 1)
      sprintf(ret, "%c%c%c%c%c", JQ6500_CMD_PREFIX, 3, command, arg1, JQ6500_CMD_TERM);
      else if (argc == 2)
      sprintf(ret, "%c%c%c%c%c%c", JQ6500_CMD_PREFIX, 4, command, arg1, arg2, JQ6500_CMD_TERM);
   else 
      return NULL;
   return ret;
}

void print_usage()
{
   printf("Usage: jq6500cmd [options]\n\n"
        "\t-d | --device=device         Serial device to use\n"
        "\t-b | --baud=baudrate         Serial baudrate (default: 9600)\n"
        "\t-p | --play                  Sending Play Command\n"
        "\t-s | --pause                 Sending Pause Command\n"
        "\t-N | --next                  Skip to next Track\n"
        "\t-P | --prev                  Skip to previous Track\n"
        "\t-U | --vol-up                Volume Up\n"
        "\t-D | --vol-down              Volume Down\n"
        "\t-h | --help                  This help\n\n");
}

long get_baudrate(long baudrate)
{
   long BAUD = 0;
   switch (baudrate)
      {
         case 115200: BAUD = B115200; break;
         case 38400:  BAUD = B38400;  break;
         case 19200:  BAUD  = B19200; break;
         case 9600:   BAUD  = B9600;  break;
         case 4800:   BAUD  = B4800;  break;
         case 2400:   BAUD  = B2400;  break;
      }  //end of switch baudrate
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

