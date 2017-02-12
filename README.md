[![Github All Releases](https://img.shields.io/github/downloads/ninharp/jq6500_tools/total.svg)]()
[![Twitter Follow](https://img.shields.io/twitter/follow/ninharp.svg?style=social&label=Follow)](https://twitter.com/ninharp)

# JQ6500 MP3 IC - Linux Toolset

## JQT6500 Flash Tool

Usage: jq6500flash [options]

-f device/file    File or SCSI Device with JQFS

-s address        Start Address to write to (Default: 0x40000)

-i file           Read out ISO from JQFS to file

-r dir            Read out JQFS to directory

-R file           Write file in Raw mode to device

-h                This help


## JQT6500 Command Tool

Usage: jq6500cmd [options]

-d | --device=device         Serial device to use

-b | --baud=baudrate         Serial baudrate (default: 9600)

-p | --play                  Sending Play Command

-I | --play-idx [idx]        Play Track [idx]

-s | --pause                 Sending Pause Command

-N | --next                  Skip to next Track

-P | --prev                  Skip to previous Track

-U | --vol-up                Volume up

-D | --vol-down              Volume down

-F | --next-dir              Skip to next folder

-G | --prev-dir              Skip to previous folder

-V | --fwversion             Get Firmware Version

-h | --help                  This help
