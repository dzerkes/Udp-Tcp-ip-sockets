// UDP_Communication_Framework.cpp : Defines the entry point for the console application.
//
#pragma comment(lib, "ws2_32.lib")
#include "stdafx.h"
#include <winsock2.h>
#include "ws2tcpip.h"
#include <zlib.h>
#include <string>
#include <iostream>
#include <bitset>
#include <cstring>

#define TARGET_IP "147.32.221.8"
#define BUFFERS_LEN 1024
//#define SENDER
#define RECEIVER
//#ifdef SENDER
#define TARGET_PORT 5555
#define LOCAL_PORT 8888
//#endif // SENDER
#ifdef RECEIVER
#define TARGET_PORT 8888
#define LOCAL_PORT 5555
#endif  RECEIVER

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
	char buffer_tx[BUFFERS_LEN];
#ifdef SENDER
	sockaddr_in addrDest;
	addrDest.sin_family = AF_INET;
	addrDest.sin_port = htons(TARGET_PORT);
	InetPton(AF_INET, _T(TARGET_IP), &addrDest.sin_addr.s_addr);

	strncpy(buffer_tx, "Hello world payload!\n", BUFFERS_LEN); //put some data to buffer
	printf("Sending packet.\n");
	FILE *fileptr;
	fileptr = fopen("file1234.txt", "rb");
	int pos;

	fseek(fileptr, 0L, SEEK_END);
	int lengthfile = ftell(fileptr);
	int numberofpackets = lengthfile / BUFFERS_LEN;
	int i = 1;
	int ctr = 0;
	while (fread(buffer_tx, sizeof(buffer_tx), 1, fileptr) == 1) {
		buffer_tx[0] = ctr;
		pos = ftell(fileptr);
		fseek(fileptr, pos - 1, SEEK_SET);
		fread(buffer_tx + i, sizeof(buffer_tx) - 1, 1, fileptr);
		sendto(socketS, buffer_tx, strlen(buffer_tx), 0, (sockaddr*)&addrDest, sizeof(addrDest));
		ctr++;
	}




	fclose(fileptr);
	closesocket(socketS);
#endif // SENDER
	int posinfile;
	FILE * f;
	f = fopen("imagesent.jpg", "wb");
#ifdef RECEIVER
	strncpy(buffer_rx, "No data received.\n", BUFFERS_LEN);
	printf("Waiting for datagram ...\n");
 int ctr=0;
  int frame_kind = 0; // 1-seq , 0 - ack
 int ack=0;
 int tmp_zero=0;
   unsigned short crc_rx;
	while (true) {

		if (recvfrom(socketS, buffer_rx, sizeof(buffer_rx), 0, (sockaddr*)&from, &fromlen) == SOCKET_ERROR) {
			printf("Socket error!\n");
			getchar();
			return 1;
		}
		else{
			if (buffer_rx[0] == 's' && buffer_rx[1] == 't' && buffer_rx[2] == 'o' && buffer_rx[3] == 'p' ){
				break; // stop packet sent
			}
			if ((int)buffer_rx[0]==ctr && (int)buffer_rx[1]==1)
			{
					  printf("[+]Data received\n");

						crc_rx =  crc16(buffer_rx+4, sizeof(buffer_rx)-4 );
						if(crc_rx == (unsigned short)buffer_rx[3]){

									buffer_tx[0] = char(tmp_zero);
								  buffer_tx[1] = char(frame_kind);
									buffer_tx[2] = char(ctr+1);
									sendto(socketS, buffer_tx, sizeof(buffer_tx), 0, (sockaddr*)&addrDest, sizeof(addrDest));


									posinfile = (int) buffer_rx[0];
									fseek(f, posinfile * 1020, SEEK_SET);
									fwrite(buffer_rx + 4, 1, sizeof(buffer_rx) - 4, f);
									printf("Datagram: %s", buffer_rx);
									  printf("[+]ack sent\n");
						}else{
								  printf("Corrupted Data \n");
										ctr--;
						}
			}
			else {
					  printf("[-]Data not received\n");
							ctr--;


			}
		}
			ctr++;
	}

  // receive after stop packet the md5 sum and compare it with the one here

	closesocket(socketS);
	fclose(f);
#endif
	//**********************************************************************
	getchar(); //wait for press Enter
	return 0;
}
