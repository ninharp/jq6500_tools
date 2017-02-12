/*

jq6500serial.h - JQ6500 Serial Protocol Definitions

Based on informations from JQ6500 Datasheet and 
Gogo:Tronics (http://sparks.gogo.co.nz/)

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

/* Private typedef -----------------------------------------------------------*/
typedef enum {
   EQ_NORMAL = 0,
   EQ_POP,
   EQ_ROCK,
   EQ_JAZZ,
   EQ_CLASSIC,
   EQ_BASS
} JQ6500_EQ_MODE;

typedef enum {
   LOOP_ALL = 0,
   LOOP_FOLDER,
   LOOP_ONE,
   LOOP_RAM,
   LOOP_ONE_STOP
} JQ6500_LOOP_MODE;
/* Private define ------------------------------------------------------------*/

/* JQ6500 Serial Protocol */
#define JQ6500_CMD_PREFIX        0x7E
#define JQ6500_CMD_TERM          0xEF

/* Control Commands
   Normal commands provide potential response of two ascii characters  “OK” and maybe  “ERROR”, but generally
   ignore responses to normal commands (it’s best to clear your serial buffer before and after issuing a normal
   command).
*/
#define JQ6500_CTRL_PLAY         0x0D     // Play. No Arguments
#define JQ6500_CTRL_PAUSE        0x0E     // Pause. No Arguments
#define JQ6500_CTRL_NEXT         0x01     // Next. No Arguments
#define JQ6500_CTRL_PREV         0x02     // Previous. No Arguments
#define JQ6500_CTRL_PLAY_IDX     0x03     // Play file by index number, 2 Arguments.  
                                          // The index number being the index in the FAT table, or upload order.
                                          // Argument 1 = high 8 bits of index number, Argument 2 = low 8 bits of index number.
#define JQ6500_CTRL_CHG_FOLDER   0x0F     // Change folder. 1 Argument.  Argument 1 = 0x01 for Next Folder, 0x00 for Previous Folder. 
#define JQ6500_CTRL_PLAY_NAME    0x12     // Play file by folder and name, 2 Arguments.  This applies to SD Card only where you have 
                                          // folders named 01 through 99, and files in those folders named 001.mp3 through 999.mp3.
                                          // Argument 1 = folder number, Argument 2 = file number.  Note that arguments are a single byte,
                                          // so effectively I think you can only access up to file 255.mp3 in any folder.
#define JQ6500_CTRL_VOL_UP       0x04     // Volume Up. No Arguments
#define JQ6500_CTRL_VOL_DOWN     0x05     // Volume Down. No Arguments
#define JQ6500_CTRL_SET_VOL      0x06     // Set Volume, 1 Argument.  Argument 1 = byte value from 0 to 30
#define JQ6500_CTRL_SET_EQ       0x07     // Set Equalizer Mode, 1 Argument.  Argument 1 = byte value 0/1/2/3/4/5 for Normal/Pop/Rock/Jazz/Classic/Bass
                                          // (actually  “Base” in the datasheet but I think they mean Bass)
#define JQ6500_CTRL_SET_LOOP     0x11     // Set Loop Mode, 1 Argument.  Argument 1 = byte value  0/1/2/3/4 for All/Folder/One/Ram/One_Stop
                                          // I don’t know what  “Ram” is, it’s not Random, it seems the same as  “One”.
#define JQ6500_CTRL_SET_SOURCE   0x09     // Set the source, 1 Argument.  Argument 1 = 0x01 for SDCard and 0x04 for the on board flash memory.
#define JQ6500_CTRL_SLEEP_MODE   0x0A     // Sleep Mode. No Arguments.  Supposedly a low power mode.
#define JQ6500_CTRL_RESET        0x0C     // Reset. No Arguments.  It’s advisable to wait 500mS or so after issuing this.

/* Query Commands
   None of the query commands have arguments. 
   Query commands return an unsigned integer as hexadecimal characters (ie if the response is the integer 1234, 
   then the response is the 4 ASCII characters  “04D2”, so yes, the commands are sent as raw bytes, and the 
   response is ASCII).
*/
#define JQ6500_QUERY_STATUS       0x42     // Get Status. Response integer (as hexadecimal ascii characters) 0/1/2 for Stopped/Playing/Paused.  
                                           // Note that  built in memory never  “Stops”, it only  “Pauses” after playing a track.  
                                           // And when playing you occasionally seem to get the odd  erroneous “Paused” response, it may be power issues,
                                           // but I sample this command several times to get a “consensus” of results!
#define JQ6500_QUERY_GET_VOL      0x43     // Get Volume.  Response integer (as hexadecimal ascii characters) from 0 to 30.
#define JQ6500_QUERY_GET_EQ       0x44     // Get Equalizer. Response integer (as hexadecimal ascii characters) from 0 to 5 (see set equalizer above for definitions).
#define JQ6500_QUERY_GET_LOOP     0x45     // Get Loop. Response integer (as hexadecimal ascii characters) from 0 to 4 (see set loop above for definitions).
#define JQ6500_QUERY_GET_VER      0x46     // Get Version.  Response appears to be an integer (as hexadecimal ascii characters).
#define JQ6500_QUERY_C_SD_FILES   0x47     // Count files on SD Card. Response integer (as hexadecimal ascii characters).
#define JQ6500_QUERY_C_INT_FILES  0x49     // Count files in on board flash memory.  Response integer (as hexadecimal ascii characters).
#define JQ6500_QUERY_C_SD_FOLDER  0x53     // Count folders on SD Card. Response integer (as hexadecimal ascii characters).
#define JQ6500_QUERY_SD_CURR_IDX  0x4B     // Get the index number (FAT table) of the current file on the SD Card.  Response integer (as hexadecimal ascii characters).
#define JQ6500_QUERY_IND_CURR_IDX  0x4D    // Get the index number MINUS ONE (!!) of the current file on the on board memory.  Response integer (as hexadecimal ascii characters).
#define JQ6500_QUERY_GET_CURR_POS  0x50    // Get the position in seconds of the current playing file.  Response integer (as hexadecimal ascii characters).
#define JQ6500_QUERY_GET_CURR_LEN  0x51    // Get the total length in seconds of the current playing file.  Response integer (as hexadecimal ascii characters).
#define JQ6500_QUERY_GET_CURR_NAME 0x52    // Get the name of the current file on the SD Card.  Response ASCII characters.  
                                           // Note that this will return a name even if the file is not playing, even if a file from the on board memory is playing, 
                                           // even if the SD Card has been removed… !  It’s also not really the file name, it lacks the file extenstion separator 
                                           // for a start (.), and is probably 8[nodot]3 max length.
