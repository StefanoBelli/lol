#include <Winsock2.h>
#include <WS2tcpip.h>
#include "SocketUtil.h"

SOCKET NewTcpSocket() {
	return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

BOOL EstablishConnection(SOCKET* sock, PCSTR addr, SHORT port) {
	SOCKADDR_IN cAddr;
	ZeroMemory(&cAddr, sizeof(SOCKADDR_IN));

	cAddr.sin_family = AF_INET;
	inet_pton(cAddr.sin_family, addr, &cAddr.sin_addr.s_addr);
	cAddr.sin_port = htons(port);

	return connect(*sock, (SOCKADDR*) &cAddr, sizeof(cAddr)) != SOCKET_ERROR; 
}

void CloseConnection(SOCKET* sock) {
	closesocket(*sock);
}

void WriteConnection(SOCKET* sock, PCSTR str) {
	send(*sock, str, (int) strlen(str), 0);
}

BOOL ReadConnection(SOCKET* sock, PSTR dst, int len) {
	ZeroMemory(dst, len);
	return recv(*sock, dst, len, 0) > 0;
}

void TransmitFileConnection(SOCKET* sock, HANDLE file) {
	DWORD readBytes = 0;
	char buffer[512] = { 0 };

	while(ReadFile(file, buffer, 512, &readBytes, NULL) 
			&& readBytes > 0)
		WriteConnectionSize(sock, buffer, readBytes);
}

void WriteConnectionSize(SOCKET* sock, PCSTR str, int len) {
	send(*sock, str, len, 0);
}

