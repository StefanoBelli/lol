#pragma once

#include <Windows.h>
#include <ctype.h>

typedef struct __s_SpawnedProcessInfo {
	PROCESS_INFORMATION info;
	HANDLE stdOut;
	HANDLE stdIn;
	HANDLE stdErr;
	BOOL isOk;
} SPAWNED_PROCESS_INFO;

static inline SPAWNED_PROCESS_INFO SpawnNewProcess(LPSTR commandLine) {
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

static inline LPSTR ErrorString(int errn) {
	LPSTR target;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errn, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), //US-ENG 
		(LPSTR) &target, 0, NULL);

	return target;
}

static inline BOOL IsNumber(PSTR str, SIZE_T lenstr) {
	for(SIZE_T i = 0; i < lenstr; ++i)
		if(!isdigit(str[i]))
			return FALSE;

	return TRUE;
}

static inline char* GetCommandTimeout(PSTR command, DWORD* dwTimeout) {
	char* integerToken = strtok(command, " ");
	char* endptr = NULL;
	SIZE_T integerTokenLen = strlen(integerToken);

	if((*dwTimeout = strtol(integerToken, &endptr, 10)) == 0){
		if(!IsNumber(integerToken, integerTokenLen))
			return NULL;
		else
			*dwTimeout = INFINITE;
	}

	return command + integerTokenLen + 1;
}
