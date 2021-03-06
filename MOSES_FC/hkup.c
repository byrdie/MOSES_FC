/****************************************************************
*Author: Roy Smart						*
*1/21/14							*
*MOSES LAB							*
*								*
*hkup connects to a serial device and reads raw data from the	*
*serial device. hkup then strips the start byte, stop byte and	*
*parity bits from the data and encloses the data in the packet	*
*structure to add to the packet queue connected to control.c	*
****************************************************************/

#include "hkup.h"

void * hkupThread(void * arg){
    init_serial_connection();
    
    buildLookupTable();
    
    while(ts_alive){
	Packet new_packet;
        Packet * p = &new_packet;
        readPacket(fup, p);
        
        enqueue(&hkupQueue, p);
        
    }
    
    return;
}


void init_serial_connection(){
	/*Open serial device for reading*/
        fup = open(HKUP, O_RDONLY | O_NOCTTY);
        if (fup < 0)
        {
                perror(HKUP);
                exit(-1);
        }
        /*save current serial port settings*/
        tcgetattr(fup, &oldtio_up);

        /*clear struct for new port settings*/
        bzero(&newtio_up, sizeof(newtio_up));

        /*set flags for non-canonical serial connection*/
        newtio_up.c_cflag |= UPBAUD | CS8 | CSTOPB | HUPCL | CREAD | CLOCAL;
        newtio_up.c_cflag &= ~(PARENB | PARODD);
        newtio_up.c_iflag &= ~(IGNBRK | BRKINT | IGNPAR | PARMRK | INPCK | INLCR | IGNCR | ICRNL | IXON | IXOFF | IUCLC | IXANY | IMAXBEL);
        //newtio_up.c_iflag |= ISTRIP;
        newtio_up.c_oflag &= ~OPOST;
        newtio_up.c_lflag &= ~(ISIG | ICANON | XCASE | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN);

        /*set non-canonical attributes*/
        newtio_up.c_cc[VTIME] = 1;
        newtio_up.c_cc[VMIN] = 255;

        tcflush(fup, TCIFLUSH);
        tcsetattr(fup ,TCSANOW, &newtio_up);

}

void readPacket(int fd, Packet * p){
    int tempValid = TRUE;
    p->valid = TRUE;
    char temp;
    char * error = "";
    
    readData(fd, &temp, 1);
    while(temp != STARTBIT){
        error += temp;
        readData(fd, &temp, 1);
    }
    if(error != ""){
        printf("Bad Packet Data\n");
    }    
    tempValid = readData(fd, p->timeStamp, 6);
    p->valid = p->valid & tempValid;
    if(tempValid != TRUE) printf("Bad Timestamp\n");
        
    tempValid = readData(fd, p->type, 1);
    p->valid = p->valid & tempValid;
    if(tempValid != TRUE) printf("Bad type\n");
    
    tempValid = readData(fd, p->subtype, 3);
    p->valid = p->valid & tempValid;
    if(tempValid != TRUE) printf("Bad subtype\n");
    
    tempValid = readData(fd, p->dataLength, 2);
    p->valid = p->valid & tempValid;
    if(tempValid != TRUE) printf("Bad data length\n");
    p->dataSize = ahtoi(p->dataLength, 2);
    
    tempValid = readData(fd, p->data, p->dataSize);
    p->valid = p->valid & tempValid;
    if(tempValid != TRUE) printf("Bad data\n");
    
    readData(fd, p->checksum, 1);
    readData(fd, &temp, 1);
    
    while(temp != ENDBIT){
        readData(fd, &temp, 1);
    }
    tempValid = (p->checksum[0] == calcCheckSum(p));
    p->valid = p->valid & tempValid;
    if(tempValid != TRUE) printf("Bad checksum %d\n", p->valid);
    
    //return p;
}
/*readData returns an array if successful or 0 if an error occurred*/
int readData(int fd, char * data, int len){
    char temp;
    int result = TRUE;
    
    int rsz = read(fd, data, len);
    while(rsz < len){
        rsz += read(fd, data + rsz, len - rsz);
    }
    
    int i;
    for(i = 0; i < len; i++){
        temp = data[i];
        data[i] = decode(temp);
        
        if(temp != encode(data[i])){
            result = FALSE;
            printf("Bad packet Encoding");
        }
    }
    data[len] = '\0';
    return result;
}
