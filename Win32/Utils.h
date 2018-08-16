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

//
// strin: target input string
// endptr: output end pointer for the current token
// length: length of the input string
//
// returns: the initial position for the delimited string
//
static inline char* GetDoubleQuoteDelimString(PSTR strin, char** endptr, SIZE_T length) {
	char *strBeginning = NULL;

	for (SIZE_T i = 0; i < length; ++i) {
		if (strin[i] == '"') {
			if (!strBeginning)
				strBeginning = strin + i; //first occ
			else {
				if (i > 0 && strin[i - 1] != '\\') { //if it is not a \"
					*endptr = strin + i + 1;
					break;
				}
			}
		}
	}

	return strBeginning;
}

#define __Internal_TokenizerCondRst(c,pb) \
	c = 0; \
	pb = NULL

//
// strin: target input string
// endptr: output end pointer for the current token
// length: length of the input string
// 
// returns the initial position of the token
//
// internal state resets for the following conditions:
//   * strin is set to NULL
//   * end of the string is reached
//
static inline char* GetNextStringToken(PSTR strin, char** endptr, SIZE_T length) {
	static SIZE_T counter = 0;
	static char* beginningptr = NULL;

	//reset internal state explicitly
	if (strin == NULL) {
		__Internal_TokenizerCondRst(counter, beginningptr);
		*endptr = NULL;
		return NULL;
	}

	//reset internal state, reached the end of the string
	if (length == counter) {
		__Internal_TokenizerCondRst(counter, beginningptr);
		return NULL;
	}

	if (beginningptr == NULL)
		beginningptr = strin;

	char* tmpptr = beginningptr;

	for (; counter < length && *beginningptr++ != ' '; ++counter) {}

	*endptr = beginningptr;

	return tmpptr;
}

#undef __Internal_TokenizerCondRst