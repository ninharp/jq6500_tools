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
void create_command(uint8_t *out, char command, uint8_t argc, uint8_t arg1, uint8_t arg2);
void print_usage();
long get_baudrate(long baudrate);
char* get_eq_name(uint8_t idx);
char* get_loop_name(uint8_t idx);
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
   struct sigaction saio;
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
   //uint8_t command[8] = "";
   uint8_t *command;
   
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
       {"play-idx",  required_argument,   0, 'I'},
       {"pause",     no_argument,         0, 's'},
       {"next",      no_argument,         0, 'N'},
       {"prev",      no_argument,         0, 'P'},
       {"vol-up",    no_argument,         0, 'U'},
       {"vol-down",  no_argument,         0, 'D'},
       {"fwversion", no_argument,         0, 'V'},
       {"next-dir",  no_argument,         0, 'F'},
       {"prev-dir",  no_argument,         0, 'G'},
       {"help",      no_argument,         0, 'h'},
       {"baud",      required_argument,   0, 'b'},
       {"device",    required_argument,   0, 'd'},
       {"get-eq",    no_argument,         0, 'E'},
       {"get-loop",  no_argument,         0, 'l'},
       {"set-loop",  required_argument,   0, 'L'},
       {"get-volume",no_argument,         0, '1'},
       {"set-volume",required_argument,   0, '2'},
       {"set-eq",    required_argument,   0, '3'},
       {0, 0, 0, 0}
   };

   uint16_t idx = 0; // Index Play helping variable
   command = (uint8_t*)malloc(7*sizeof(uint8_t));
   //Specifying the expected options
   //while ((option = getopt(argc, argv, "d:b::psNP+-h")) != -1) {
   while ((option = getopt_long (argc, argv, "d:b::pI:sNPUDVlL123Eh", long_options, &option_index)) != -1) {
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
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'I' :  cmd = JQ6500_CTRL_PLAY_IDX;
                     idx = atoi(optarg);
                     if (idx > 0 && idx < 65536) {
                        send_command = 1;
                        if (idx > 255) {
                           create_command(command, cmd, 2, (uint8_t)(idx>>8), (uint8_t)(idx & 0xFF));
                        } else {
                           create_command(command, cmd, 2, 0, (uint8_t)idx);
                        }
                        break;
                     } else {
                        fprintf(stderr, "Error: Play index out of range! (1-65535)\n");
                        exit(EXIT_FAILURE);
                     }
         case 's' :  send_command = 1;
                     cmd = JQ6500_CTRL_PAUSE;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'N' :  send_command = 1;
                     cmd = JQ6500_CTRL_NEXT;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'P' :  send_command = 1;
                     cmd = JQ6500_CTRL_PREV;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'U' :  send_command = 1;
                     cmd = JQ6500_CTRL_VOL_UP;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'D' :  send_command = 1;
                     cmd = JQ6500_CTRL_VOL_DOWN;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'V' :  send_command = 1;
                     cmd = JQ6500_QUERY_GET_VER;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'l' :  send_command = 1;
                     cmd = JQ6500_QUERY_GET_LOOP;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'L' :  send_command = 1;
                     cmd = JQ6500_CTRL_SET_LOOP;
                     idx = atoi(optarg);
                     if (idx >= 0 && idx <= 5) {
                        create_command(command, cmd, 1, idx, 0);
                        break;
                     } else {
                        fprintf(stderr, "Error: Loop out of range! (0-5)\n");
                        exit(EXIT_FAILURE);
                     }
         case '1' :  send_command = 1;
                     cmd = JQ6500_QUERY_GET_VOL;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case '2' :  send_command = 1;
                     cmd = JQ6500_CTRL_SET_VOL;
                     idx = atoi(optarg);
                     if (idx >= 0 && idx <= 30) {
                        create_command(command, cmd, 1, idx, 0);
                        break;
                     } else {
                        fprintf(stderr, "Error: Volume out of range! (0-30)\n");
                        exit(EXIT_FAILURE);
                     }
         case '3' :  send_command = 1;
                     cmd = JQ6500_CTRL_SET_EQ;
                     idx = atoi(optarg);
                     if (idx >= 0 && idx <= 6) {
                        create_command(command, cmd, 1, idx, 0);
                        break;
                     } else {
                        fprintf(stderr, "Error: EQ mode out of range! (0-6)\n");
                        exit(EXIT_FAILURE);
                     }
         case 'E' :  send_command = 1;
                     cmd = JQ6500_QUERY_GET_EQ;
                     create_command(command, cmd, 0, 0, 0);
                     break;
         case 'F' :  send_command = 1;
                     cmd = JQ6500_CTRL_CHG_FOLDER;
                     create_command(command, cmd, 1, JQ6500_NEXT_FOLDER, 0);
                     break;
         case 'G' :  send_command = 1;
                     cmd = JQ6500_CTRL_CHG_FOLDER;
                     create_command(command, cmd, 1, JQ6500_PREV_FOLDER, 0);
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
      fprintf(stderr, "Error: Unknown Baudrate: %ld\n\n", baudrate);
      exit(EXIT_FAILURE);
   }

   if (send_command == 0) {
      fprintf(stderr, "Error: No Command specified!\n\n");
      exit(EXIT_FAILURE);  
   }

   printf("Using device: %s\n", devicename);
   printf("Baudrate: %ld baud\n\n", baudrate);

   if (verbose_flag)
      printf("Command String: ");
   for (int i = 0; i < 8; i++) {
      if ((uint8_t)command[i] == JQ6500_CMD_TERM) {
         command_len = i+1;
         if (verbose_flag)
            printf("0x%02X\n", (uint8_t)command[i]);
         break;
      } else {
         if (verbose_flag)
            printf("0x%02X ", (uint8_t)command[i]);
      }
   }
   if (verbose_flag) printf("\n");

   switch (cmd) {
      case JQ6500_CTRL_PLAY:        printf("Sending Command 'Play'\n"); break;
      case JQ6500_CTRL_PLAY_IDX:    printf("Sending Command 'Play Index' with Index %d\n", idx); break;
      case JQ6500_CTRL_PAUSE:       printf("Sending Command 'Pause'\n"); break;
      case JQ6500_CTRL_PREV:        printf("Sending Command 'Previous Track'\n"); break;
      case JQ6500_CTRL_NEXT:        printf("Sending Command 'Next Track'\n"); break;
      case JQ6500_CTRL_VOL_UP:      printf("Sending Command 'Volume Up'\n"); break;
      case JQ6500_CTRL_VOL_DOWN:    printf("Sending Command 'Volume Down'\n"); break;
      case JQ6500_CTRL_SET_VOL:     printf("Set Volume %d\n", idx); break;
      case JQ6500_CTRL_SET_EQ:      printf("Set EQ Mode %s\n", get_eq_name(idx)); break;
      case JQ6500_CTRL_SET_LOOP:    printf("Set Loop Mode %s\n", get_loop_name(idx)); break;
      case JQ6500_QUERY_GET_VER:    printf("Querying Version Information\n"); break;
      case JQ6500_QUERY_GET_LOOP:   printf("Querying Loop Mode\n"); break;
      case JQ6500_QUERY_GET_EQ:     printf("Querying EQ Mode\n"); break;
      case JQ6500_QUERY_GET_VOL:    printf("Querying Current Volume\n"); break;
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
   newtio.c_cflag = get_baudrate(baudrate) | CREAD;
   newtio.c_cflag &= ~CLOCAL;
   newtio.c_iflag = IGNPAR | IGNBRK;
   newtio.c_oflag = 0;
   newtio.c_lflag = 0;
   newtio.c_cc[VMIN] = 2;
   newtio.c_cc[VTIME] = 1;
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
         } else if (cmd == JQ6500_QUERY_GET_VOL && c == 4) {
            printf("Current Volume: %d\n", (int)strtol(response, NULL, 16));
            STOP = true;
         } else if (cmd == JQ6500_QUERY_GET_EQ && c == 4) {
            printf("Current EQ Mode: %s\n", get_eq_name((int)strtol(response, NULL, 16)));
            STOP = true;
         } else if (cmd == JQ6500_QUERY_GET_LOOP && c == 4) {
            printf("Current Loop Mode: %s\n", get_loop_name((int)strtol(response, NULL, 16)));
            STOP = true;
         } else if (cmd == JQ6500_CTRL_SET_EQ) {
            STOP = true;
         }
         wait_flag = true; // wait for new input
      }

   }  //while stop==FALSE

   printf("\n");

   /* restore the old port settings */
   tcsetattr(fd, TCSANOW, &oldtio);
   return(EXIT_SUCCESS);
}

void create_command(uint8_t *out, char command, uint8_t argc, uint8_t arg1, uint8_t arg2)
{
   
   int len = 0;
   if (argc == 0)
      len = 2;
   else if (argc == 1)
      len = 3;
   else if (argc == 2)
      len = 4;
   else 
      return;

   out[0] = (uint8_t)JQ6500_CMD_PREFIX;
   out[1] = (uint8_t)len;
   out[2] = (uint8_t)command;
   if (argc == 0) {
      out[3] = (uint8_t)JQ6500_CMD_TERM;
   } else if (argc == 1) {
      out[3] = (uint8_t)arg1;
      out[4] = (uint8_t)JQ6500_CMD_TERM;
   } else if (argc == 2) {
      out[3] = (uint8_t)arg1;
      out[4] = (uint8_t)arg2;
      out[5] = (uint8_t)JQ6500_CMD_TERM;
   }
}

void print_usage()
{
   printf("Usage: jq6500cmd [options]\n\n"
        "\t-d | --device=device         Serial device to use\n"
        "\t-b | --baud=baudrate         Serial baudrate (default: 9600)\n"
        "\t-p | --play                  Sending Play Command\n"
        "\t-I | --play-idx [idx]        Play Track [idx]\n"
        "\t-s | --pause                 Sending Pause Command\n"
        "\t-N | --next                  Skip to next Track\n"
        "\t-P | --prev                  Skip to previous Track\n"
        "\t-U | --vol-up                Volume up\n"
        "\t-D | --vol-down              Volume down\n"
        "\t-F | --next-dir              Skip to next folder\n"
        "\t-G | --prev-dir              Skip to previous folder\n"
        "\t-V | --fwversion             Get Firmware Version\n"
        "\t-h | --help                  This help\n\n");
}

char* get_eq_name(uint8_t idx)
{
   switch (idx) {
      case EQ_NORMAL:   return "Normal";
      case EQ_POP:      return "Pop";
      case EQ_ROCK:     return "Rock";
      case EQ_JAZZ:     return "Jazz";
      case EQ_CLASSIC:  return "Classic";
      case EQ_BASS:     return "Bass";
      default:          return "Error";
   }
}

char* get_loop_name(uint8_t idx)
{
   switch (idx) {
      case LOOP_ALL:       return "All";
      case LOOP_FOLDER:    return "Folder";
      case LOOP_ONE:       return "One";
      case LOOP_RAM:       return "RAM";
      case LOOP_ONE_STOP:  return "One Stop";
      default:             return "Error";
   }
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
   //fprintf(stderr, "Error: Received SIGIO signal.\n");
   wait_flag = false;
}

