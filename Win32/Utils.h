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

SPAWNED_PROCESS_INFO SpawnNewProcess(LPSTR commandLine);

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
