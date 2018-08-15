#include "SocketUtil.h"
#include "Utils.h"

SPAWNED_PROCESS_INFO SpawnNewProcess(LPSTR commandLine) {
	SPAWNED_PROCESS_INFO process;

	PROCESS_INFORMATION procInfo;
	STARTUPINFO startupInfo;

	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&process, sizeof(SPAWNED_PROCESS_INFO));

	char system32[MAX_PATH];
	ZeroMemory(system32, MAX_PATH);

	const char* systemDrive = getenv("SystemDrive");
	*system32 = *systemDrive;
	*(system32 + 1) = *(systemDrive + 1);
	memcpy(system32 + 2, "\\Windows\\System32", sizeof("\\Windows\\System32"));

	char fullCommand[BUFSIZE + 11];
	ZeroMemory(fullCommand, BUFSIZE + 11);
	memcpy(fullCommand, "cmd.exe /c ", 11);
	memcpy(fullCommand + 11, commandLine, strlen(commandLine));

	if(!CreateProcessA(NULL, fullCommand,
				NULL, NULL, FALSE, CREATE_NEW_CONSOLE, 
				NULL, system32, &startupInfo, &procInfo))
		return process;

	process.info = procInfo;
	process.stdOut = startupInfo.hStdOutput;
	process.stdIn = startupInfo.hStdInput;
	process.stdErr = startupInfo.hStdError;
	process.isOk = TRUE;

	return process;
}


