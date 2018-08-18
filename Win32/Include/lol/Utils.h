#pragma once

#include <Windows.h>

typedef struct __s_SpawnedProcessInfo {
	PROCESS_INFORMATION info;
	HANDLE stdOut;
	HANDLE stdIn;
	HANDLE stdErr;
	BOOL isOk;
} SPAWNED_PROCESS_INFO;

SPAWNED_PROCESS_INFO SpawnNewProcess(LPSTR commandLine);
char* GetDirectoryContent(HANDLE heapHandle, LPCSTR directory);
char* GetSystemProcessSnapshot(HANDLE heapHandle);
char* GetNextStringToken(PSTR strin, char** endptr, SIZE_T length);

#define ErrorString(errn, target)\
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |	\
					FORMAT_MESSAGE_FROM_SYSTEM | \
					FORMAT_MESSAGE_IGNORE_INSERTS, \
					NULL, errn, \
					MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), \
					target, 0, NULL)

static inline char* GetCommandTimeout(PSTR command, DWORD* dwTimeout) {
	if (*command == '0') {
		*dwTimeout = INFINITE;
		return command + 2;
	}

	char* endptr = NULL;
	char* integerToken = strtok(command, " ");

	if (!(*dwTimeout = strtol(integerToken, &endptr, 10)))
		return NULL;
	
	return command + strlen(integerToken) + 1;
}

static inline char* GetDoubleQuoteDelimString(PSTR strin, char** endptr, SIZE_T length) {
	char *strBeginning = NULL;

	for (SIZE_T i = 0; i < length; ++i) {
		if (strin[i] == '"') {
			if (!strBeginning)
				strBeginning = strin + i; //first occ
			else if (i > 0 && strin[i - 1] != '\\') { //if it is not a \"
				*endptr = strin + i + 1;
				break;
			}
		}
	}

	return strBeginning;
}
