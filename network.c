#include <Winsock2.h>

static SOCKET s;

int ConnectToServer(char *ip, int port)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) 
	{
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	int ret = connect(s, (const struct sockaddr *)&addr, sizeof(addr));
	if (ret != 0) 
	{
		return -2;
	}

	return 0;
}

int SendMsg(char *buff, int len)
{
	return send(s, buff, len, 0);
}

int RecvMsg(char *buff, int len)
{
	return recv(s, buff, len, 0);
}

int CloseConnect()
{
	closesocket(s);
}
