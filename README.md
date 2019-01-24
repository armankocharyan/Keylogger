# Keylogger

README:

Creator: Arman Kocharyan

This project is a simple keylogger made for a linux system (tested on ubuntu 16.04)

To COMPILE:
1) go to the directory of the file from a linux terminal
2) type and enter "make" 

To INSTALL the module:
1) go to the directory of the file from a linux terminal after compiling
2) type and enter "sudo insmod keylogger.ko" 

To READ the logged keys:
1) open a linux terminal
2) type and enter "sudo cat /sys/kernel/debug/keylogger/keys" 

To REMOVE the linux module:
1) open a linux terminal
2) type and enter "sudo rmmod keylogger" 

To CLEAN the files:
1) go to the directory of the "keylogger.c" file from a linux terminal
2) type and enter "make clean" 


Files in the folder:
keylogger.c
Makefile

