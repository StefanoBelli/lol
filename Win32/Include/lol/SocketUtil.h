#pragma once

#include <Windows.h>

#define MAX_BUF_IN_OUT 256
#define BUFSIZE (MAX_BUF_IN_OUT + 1)

SOCKET NewTcpSocket();
BOOL EstablishConnection(SOCKET*, PCSTR, SHORT);
void CloseConnection(SOCKET*);

void WriteConnection(SOCKET*, PCSTR);
void WriteConnectionSize(SOCKET*, PCSTR, int);
BOOL ReadConnection(SOCKET*, PSTR, int);

