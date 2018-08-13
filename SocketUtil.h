#pragma once

#include <Windows.h>

SOCKET NewTcpSocket();
BOOL EstablishConnection(SOCKET*, PCSTR, SHORT);
void CloseConnection(SOCKET*);

void WriteConnection(SOCKET*, PCSTR);
BOOL ReadConnection(SOCKET*, PSTR, int);

