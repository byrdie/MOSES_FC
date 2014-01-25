#include "system.h"
#include <termios.h>
#include "packetType.h"
#include "lockingQueue.h"
#include <signal.h>

#define UPBAUD B1200
#define HKUP "/dev/hkup"
#define _POSIX_SOURCE 1 /*POSIX compliant source*/


extern volatile sig_atomic_t ts_alive;

pid_t parent_pid;
extern LockingQueue hkupQueue;

int fup, res;	//variables for reading serial data
char buf[255];	//serial port buffer
char packet_array[270];		//final packetr array
struct termios oldtio_up, newtio_up; //structures for old device settings and new 

//extern LockingQueue hkupQueue;

/*Functions*/
void init_serial_connection();	//initialize attributs of serial connection
