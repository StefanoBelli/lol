#include <stdio.h>
#include "RemoteCommandExecutor.h"
#include "SocketUtil.h"

#define DST "192.168.2.103"
#define PORT 12345L
#define MAX_BUF_IN_OUT 256
#define BUFSIZE (MAX_BUF_IN_OUT + 1)

#define SendInitialInfo(wsad) { \
	char buf[BUFSIZE]; \
	ZeroMemory(buf, BUFSIZE); \
	snprintf(buf, MAX_BUF_IN_OUT, "WSA implementation\n" \
			"Version: %d.%d\n" \
			"Max. sockets: %d\n" \
			"VendorInfo: %s\n" \
			"InternalStatus: %s\n" \
			"^^^Happy Hunting^^^", \
			LOBYTE(wsad.wVersion), HIBYTE(wsad.wVersion), \
			wsad.iMaxSockets, wsad.lpVendorInfo, wsad.szSystemStatus); \
	WriteConnection(&tcpSocket, buf); \
}

#define N_COMMANDS 2

typedef void(*CommandProc)();

static SOCKET tcpSocket = SOCKET_ERROR;
static char *commandsString[] = { "help\n", "hello\n" };

void HelpCommand() {
	char buf[BUFSIZE];
	ZeroMemory(buf,BUFSIZE);

	for(int i = 0; i < N_COMMANDS && strlen(buf) < BUFSIZE; i++)
		strncat(buf, commandsString[i], strlen(commandsString[i]));
	
	WriteConnection(&tcpSocket, buf);
}

void HelloCommand() {
	WriteConnection(&tcpSocket, "i am still here dumbass...\n");
}

static CommandProc commandProc[2] = { HelpCommand, HelloCommand };

void CommandParser(PSTR);

void StartCommandExecutorConnection(void) {
	WSADATA internalSocketData;
	if(WSAStartup(MAKEWORD(2,2), &internalSocketData))
		return;

	while(1) { //top-level
		tcpSocket = NewTcpSocket();
		while(!EstablishConnection(&tcpSocket, DST, PORT)) {}
		
		//connection established
		SendInitialInfo(internalSocketData);
		
		//start the command parser
		//it will run til TCP connection is ESTABLISHED
		char commandBuffer[BUFSIZE];
		while(ReadConnection(&tcpSocket, commandBuffer, MAX_BUF_IN_OUT))
			CommandParser(commandBuffer);

		//disconnected or error occoured, close socket and get back
		CloseConnection(&tcpSocket);
	}
}

void StopCommandExecutor(void) {
	CloseConnection(&tcpSocket);
	WSACleanup();
}

static void CommandParser(PSTR buffer) {
	BOOL isValidCommand = FALSE;

	for(int i = 0;i < N_COMMANDS; i++) {
		if(!strcmp(buffer,commandsString[i])) {
			commandProc[i]();
			isValidCommand = TRUE;
		}
	}

	if(!isValidCommand)
		WriteConnection(&tcpSocket, "Uh?\nInvalid command.\n");
}

