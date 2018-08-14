#include <string.h>
#include "Commands.h"
#include "SocketUtil.h"

BOOL HelpCommand(SOCKET* sck) {
	char buf[BUFSIZE];
	ZeroMemory(buf,BUFSIZE);
	
	for(int i = 0; strlen(buf) < BUFSIZE && i < N_COMMANDS; ++i) {
		strncat(buf, commands[i].command, strlen(commands[i].command));
		
		if(commands[i].needsArguments)
			strncat(buf, " [a]", 4);

		if(i < N_COMMANDS - 1)
			strncat(buf, ", ", 2);
	}

	WriteConnection(sck, buf);

	return TRUE;
}

BOOL HelloCommand(SOCKET* sck) {
	WriteConnection(sck, "i am still here dumbass...\n");

	return TRUE;
}

BOOL ReplyCommand(SOCKET* sck, PSTR str) {
	if(!strlen(str))
		return FALSE;

	WriteConnection(sck, str);

	return TRUE;
}
