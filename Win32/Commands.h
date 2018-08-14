#pragma once

#include <Windows.h>

#define N_COMMANDS 7

typedef BOOL (*CommandProc)(SOCKET*, PSTR);
typedef struct _s_Command {
	char command[50];
	CommandProc proc;
	BOOL needsArguments;
} Command;

Command commands[N_COMMANDS];

BOOL HelpCommand(SOCKET*);
BOOL HelloCommand(SOCKET*);
BOOL ReplyCommand(SOCKET*, PSTR);
BOOL NtRaiseHardErrorCommand(SOCKET*);
BOOL ChangeDirectoryCommand(SOCKET*, PSTR);
BOOL GetCwdCommand(SOCKET*);
BOOL CmdCommand(SOCKET*, PSTR);
