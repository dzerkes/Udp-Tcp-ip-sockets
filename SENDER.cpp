// UDP_Communication_Framework.cpp : Defines the entry point for the console application.
//
#pragma comment(lib, "ws2_32.lib")
#include "stdafx.h"
#include <winsock2.h>
#include "ws2tcpip.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <zlib.h>
#include <string>
#include <bitset>
#include <cstring>


#define TARGET_IP	"192.168.55.209"

#define BUFFERS_LEN 1024

#define SENDER
//#define RECEIVER

#ifdef SENDER
#define TARGET_PORT 5555
#define LOCAL_PORT 8888
#endif  SENDER

#ifdef RECEIVER
#define TARGET_PORT 8888
#define LOCAL_PORT 5555
#endif // RECEIVER


void InitWinsock()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

//**********************************************************************

unsigned short crc16(const unsigned char* data_p, unsigned char length){
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}



int main()
{
	SOCKET socketS;

	InitWinsock();

	struct sockaddr_in local;
	struct sockaddr_in from;

	int fromlen = sizeof(from);
	local.sin_family = AF_INET;
	local.sin_port = htons(LOCAL_PORT);
	local.sin_addr.s_addr = INADDR_ANY;


	socketS = socket(AF_INET, SOCK_DGRAM, 0);
	if (bind(socketS, (sockaddr*)&local, sizeof(local)) != 0) {
		printf("Binding error!\n");
		getchar(); //wait for press Enter
		return 1;
	}
	//**********************************************************************
	char buffer_rx[BUFFERS_LEN];
	char buffer_tx[BUFFERS_LEN]="text to read";

#ifdef SENDER

	sockaddr_in addrDest;
	addrDest.sin_family = AF_INET;
	addrDest.sin_port = htons(TARGET_PORT);
	InetPton(AF_INET, _T(TARGET_IP), &addrDest.sin_addr.s_addr);


	strncpy(buffer_tx, "Hello world payload!\n", BUFFERS_LEN); //put some data to buffer
	printf("Sending packet.\n");

	FILE *fileptr;
	fileptr = fopen("penguin.jpg", "rb");
	int pos;


	fseek(fileptr, 0L, SEEK_END);

	int lengthfile = ftell(fileptr);
	rewind(fileptr);
	int numberofpackets = lengthfile / BUFFERS_LEN - 1;
	//int i = 1;
	int ctr = 0;
	int ack_recv =1;
	int frame_kind = 1; // 1-seq , 0 - ack
	int ack=0;
  unsigned short crc_tx;
	char stop[BUFFERS_LEN] = "stop";

	//printf("start");
	while (!feof(fileptr)) {

		if(ack_recv ==1){


			buffer_tx[0] = (char)ctr;
			buffer_tx[1] = (char) frame_kind;
			buffer_tx[2]= (char)ack;
			//pos = ftell(fileptr);
			//fseek(fileptr, pos - 1, SEEK_SET);
			fread(buffer_tx + 4, sizeof(buffer_tx) - 4, 1, fileptr);
			crc_tx = crc16(buffer_tx+4, sizeof(buffer_tx)-4 );
			buffer_tx[3] = (char) crc_tx;
			//printf("%d \n",ctr);
			printf("%s", buffer_tx);
			sendto(socketS, buffer_tx, sizeof(buffer_tx), 0, (sockaddr*)&addrDest, sizeof(addrDest));
		}
	 int f_recv_size = recvfrom(socketS, buffer_rx, sizeof(buffer_rx), 0, (sockaddr*)&from, &fromlen);
	 if (f_recv_size > 0 && (int)buffer_rx[0]==0 && (int)buffer_tx[2]==ctr+1) {

		 ack_recv =1;
		 printf("[+]ack received\n");

	 }else{
		 ack_recv =0;
		 printf("[-]ack not received\n");
	 }
	 if(ack_recv==0){
		 //maybe didnt reach maybe data corruption from crc
		 	sendto(socketS, buffer_tx, sizeof(buffer_tx), 0, (sockaddr*)&addrDest, sizeof(addrDest));
			ctr--; // since its same packet and later im going to increase it i decrease it so we are counting properly
	 }




		ctr++;

	}

	sendto(socketS, stop, sizeof(stop), 0, (sockaddr*)&addrDest, sizeof(addrDest));

	// after stop i can send md5 sum

	fclose(fileptr);
	closesocket(socketS);

#endif // SENDER

#ifdef RECEIVER

	strncpy(buffer_rx, "No data received.\n", BUFFERS_LEN);
	printf("Waiting for datagram ...\n");
	if (recvfrom(socketS, buffer_rx, sizeof(buffer_rx), 0, (sockaddr*)&from, &fromlen) == SOCKET_ERROR) {
		printf("Socket error!\n");
		getchar();
		return 1;
	}
	else
		printf("Datagram: %s", buffer_rx);

	closesocket(socketS);
#endif
	//**********************************************************************

	getchar(); //wait for press Enter
	return 0;
}
