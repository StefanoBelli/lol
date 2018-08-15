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
char* GetDirectoryContent(HANDLE heapHandle, LPCSTR directory);

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

static inline char* GetDoubleQuoteDelimString(PSTR strin, DWORD* endpos, SIZE_T lenstr) {
	char *strBeginning = NULL;

	for (SIZE_T i = 0; i < lenstr; ++i) {
		if (strin[i] == '"') {
			if (!strBeginning)
				strBeginning = strin + i; //first occ
			else {
				if (i > 0 && strin[i - 1] != '\\') { //if it is not a \"
					*endpos = i + 1;
					break;
				}
			}
		}
	}

	return strBeginning;
}

// \\*
// reserve 2 more bytes for str
// char path[12]; [ZEROED]
// Example:
//   [c:\windows]\*
//   12 bytes
#define PutAnyWildcardAtString(str) \
	strncat(str, "\\*", 2)

static inline char* GetNextStringToken(PSTR strin, DWORD* quotedStringEndPos) {
	static char* firstToken = NULL;

	//reset internal state
	if (strin == NULL) {
		firstToken = NULL;
		return NULL;
	}

	if (firstToken == NULL)
		firstToken = strtok(strin, " ");
	else
		firstToken = strtok(NULL, " ");

	if (firstToken == NULL)
		return NULL;

	*quotedStringEndPos = 0;
	char* string = GetDoubleQuoteDelimString(firstToken, quotedStringEndPos, strlen(firstToken));

	return string ? string : firstToken;
}
