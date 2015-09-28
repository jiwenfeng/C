//$Id: lsocket.hpp 65945 2008-12-15 06:26:49Z tony $
//create by tony for lua socket api
#include "global.hpp"
#include "stdio.h"
#include "lstack.hpp"
#include <curl/curl.h>
#include <vector>

#ifndef __LUA_SOCKET_HPP__
#define __LUA_SOCKET_HPP__

#define CLIENT 0
#define SERVER 1

extern size_t gateWayDataLen;
extern const char* gateWayData;

typedef struct Head_t{
	unsigned int len;
	unsigned char zip;
	unsigned short pro;
}__attribute__((packed)) Head;


struct PtoNode{
	std::string data;
	unsigned char zip;
	unsigned short proid;
	PtoNode(const char* ptr, size_t len, unsigned char z, unsigned short p):data(ptr, len), zip(z), proid(p){}
};

bool is_socketfd(int socket_fd);
int checksocketfd(lua_State* L, int narg);

void luaopen_socket(lua_State* L);
void luaopen_httplib(lua_State* L);
void InitHttp(int Port, lua_State *L);
void luaopen_netlib(lua_State* L);

class TSocket
{
	public:
		TSocket(int _socket_fd):socket_fd(_socket_fd){}
		virtual ~TSocket(){
			if( is_socketfd(socket_fd) ) close(socket_fd);
		}
	protected:
		int socket_fd;
	private:
		TSocket(const TSocket&);//forbide
		TSocket& operator = (const TSocket&);//forbide
};

class TClientSocket:public TSocket
{
	public:
		TClientSocket(int _socket_fd, int type, int _read_ref, int _write_ref, int _error_ref);
		virtual ~TClientSocket();
		int write(const char* dataptr, size_t length);
		int on_error(short flag);
	//	int readcb(const char* dataptr, size_t length);
	//	int readcb(const char* dataptr, size_t length, size_t zip, size_t pro);
		int readcb(std::vector<PtoNode>& PtoList);
		int writecb();
		char* GetReadBuffer();
		int DelReadBufferData(unsigned int length);
		bool IsGateWayDataThrowed();
		void SetGateWayDataThrowed();
		
		unsigned int curlen;
		unsigned int max_len;
	private:
		struct bufferevent* bev;
		int read_ref;
		int write_ref;
		int error_ref;
		char *ReadBuffer;
		int type;
		
		// Is throwed gateway data
		int throwGW;
};

class TServerSocket:public TSocket
{
	public:
		TServerSocket(int _socket_fd, int _read_ref);
		virtual ~TServerSocket();
		void acceptcb();
	private:
		struct event* listen_ev;
		int read_ref;
};

#endif
