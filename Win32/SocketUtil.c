#include <Winsock2.h>
#include "SocketUtil.h"

SOCKET NewTcpSocket() {
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

BOOL EstablishConnection(SOCKET* sock, PCSTR addr, SHORT port) {
	SOCKADDR_IN cAddr;
	ZeroMemory(&cAddr, sizeof(SOCKADDR_IN));

	cAddr.sin_family = AF_INET;
	cAddr.sin_addr.s_addr = inet_addr(addr);
	cAddr.sin_port = htons(port);

	return connect(*sock, (SOCKADDR*) &cAddr, sizeof(cAddr)) != SOCKET_ERROR; 
}

void CloseConnection(SOCKET* sock) {
	closesocket(*sock);
}

void WriteConnection(SOCKET* sock, PCSTR str) {
	send(*sock, str, strlen(str), 0);
}

BOOL ReadConnection(SOCKET* sock, PSTR dst, int len) {
	ZeroMemory(dst, len);
	return recv(*sock, dst, len, 0) > 0;
}

