#ifndef _NETWORK_H_
#define _NETWORK_H_

int ConnectToServer(char *ip, int port);

int SendMsg(char *buff, int len);

int RecvMsg(char *buff, int len);

int CloseConnect();

#endif
