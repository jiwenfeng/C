//$Id: lsocket.cpp 106777 2010-01-12 13:39:32Z tony@NETEASE.COM $
//create by tony Wed 15 Oct 2008 11:20:45 CST for lua socket api
#include <map>
#include <evhttp.h>
#include <evdns.h>
#include "netlib.hpp"
#include "global.hpp" 
#include "luacall.hpp"

#define RESOLV "/etc/resolv.conf"
#define CURL_DATA_LEN 1024 * 5
#define USER_DEFINED_ERR_CODE 10001


void socket_setnonblocking(int socket_fd)
{
	int flags = fcntl(socket_fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(socket_fd, F_SETFL, flags);
}

static void socket_accept_hook(int socket_fd, short event, void* arg)
{
	TServerSocket* socket_obj = (TServerSocket*)arg;
	socket_obj -> acceptcb();
}
TServerSocket::TServerSocket(int _socket_fd, int _read_ref):TSocket(_socket_fd)
{
	read_ref = _read_ref;
	listen_ev = new struct event;
	event_set(listen_ev, socket_fd, EV_READ|EV_PERSIST, socket_accept_hook, (void*)this);
	if ( event_add(listen_ev, NULL) != 0 )
	{
		_ERROR("event_add failed");
	}
}
void TServerSocket::acceptcb()
{
	lua_getref(LState, read_ref);
	if (!lua_isfunction(LState, -1))
	{
		_ERROR("call back function is not valid:%d", read_ref);
		return;
	}
	TArgPool ArgPoolObj;
	ArgPoolObj.AddArg((double)socket_fd);
	int Ret = CallLua(LState, read_ref, ArgPoolObj);
	if (0 != Ret)
	{
		_ERROR("call function failed:%d,%d", Ret, read_ref);
	}
}
TServerSocket::~TServerSocket()
{
	event_del(listen_ev);
	delete listen_ev;
	listen_ev = NULL;
}
/*
static void socket_read_hook(struct bufferevent *bufev, void *arg)
{
#define MAX_LENGTH 1024
	while( true )
	{
		TClientSocket* socket_obj = (TClientSocket*)arg;
		size_t length = EVBUFFER_LENGTH(EVBUFFER_INPUT(bufev));
		if( length == 0 )
			break;
		if (length > MAX_LENGTH) length = MAX_LENGTH;
		char data[length];
		size_t readlen = bufferevent_read(bufev, data, length);
		socket_obj -> readcb(data, readlen);
	}
}*/

static void socket_read_hook(struct bufferevent *bufev, void *arg)
{
//	while (true)
//	{		
		TClientSocket* socket_obj = (TClientSocket*)arg;
		size_t length = EVBUFFER_LENGTH(EVBUFFER_INPUT(bufev));
		if( length == 0 )
		{
			return;
		}
		if (length + socket_obj -> curlen > socket_obj->max_len)
			length = socket_obj->max_len  - socket_obj -> curlen;
		char *buff = socket_obj -> GetReadBuffer();
		size_t readlen = bufferevent_read(bufev, buff + socket_obj -> curlen, length);
		socket_obj -> curlen += readlen;
		if (! socket_obj -> IsGateWayDataThrowed())
		{
			if (socket_obj -> curlen >= gateWayDataLen && memcmp(buff, gateWayData, gateWayDataLen) == 0)
			{
				socket_obj -> DelReadBufferData(gateWayDataLen);
				socket_obj -> SetGateWayDataThrowed();
				if (socket_obj -> curlen <= 0)
				{
					return;
				}
			}
			else
			{
				_WARN("read_hook erro:Invalid gateway data = %s.", (char *)buff);
				socket_obj -> on_error(USER_DEFINED_ERR_CODE);
				return;
			}
		}
		std::vector<PtoNode> PtoList;
		while ( socket_obj -> curlen >= sizeof(Head) )
		{
			Head* head = (Head*)buff;
			size_t dataLen, proId;
			dataLen = ntohl(head -> len);
			proId = ntohs(head -> pro);
			if (socket_obj -> curlen >= dataLen + sizeof(Head) )
			{
				struct PtoNode Node(buff+sizeof(Head), dataLen, head->zip, proId);
				PtoList.push_back(Node);
				socket_obj -> DelReadBufferData(dataLen + sizeof(Head));
				continue;
			}
			break;
		}
		if (PtoList.size() > 0)
			socket_obj->readcb(PtoList);
	}
//}

static void socket_write_hook(struct bufferevent *bufev, void *arg)
{
	TClientSocket* socket_obj = (TClientSocket*)arg;
	socket_obj -> writecb();
}
static void socket_error_hook(struct bufferevent *bufev, short flag, void *arg)
{
	TClientSocket* socket_obj = (TClientSocket*)arg;
	socket_obj -> on_error(flag);
//	delete socket_obj;
}

TClientSocket::TClientSocket(int _socket_fd, int type, int _read_ref, int _write_ref, int _error_ref):TSocket(_socket_fd),type(type)
{
	read_ref = _read_ref;
	write_ref = _write_ref;
	error_ref = _error_ref;
	curlen = 0;
	bev = bufferevent_new(socket_fd, socket_read_hook, socket_write_hook, socket_error_hook, (void*)this);
	bufferevent_enable(bev, EV_READ|EV_WRITE);
	socket_setnonblocking(socket_fd);
	
	// Type is 1 means connections between servers which needs bigger buffer
	// Type is 0 means connections between server and Players which need smaller buffer
	max_len = (type == 1) ? 1024 * 1024 * 10 : 1024 * 10;
	ReadBuffer = new char[max_len];
	memset(ReadBuffer, '\0', max_len);
	
	// Specially, type is 2 means Player Client which needs throw GateWay data
	throwGW = (type == 2) ? 0 : 1;
}

TClientSocket::~TClientSocket()
{
	lua_unref(LState, read_ref);
	lua_unref(LState, write_ref);
	lua_unref(LState, error_ref);
	int Len = EVBUFFER_LENGTH(EVBUFFER_OUTPUT(bev));
	if (Len > 0 )
	{
		if ( evbuffer_write(bev->output, socket_fd) != Len )
		{
			_WARN("Lost data when destroy client");
		}
	}
	bufferevent_disable(bev, EV_WRITE|EV_READ);
	bufferevent_free(bev);
	bev = NULL;
	delete []ReadBuffer;
	ReadBuffer = NULL;
}

int TClientSocket::write(const char* dataptr, size_t length)
{
	return bufferevent_write(bev, dataptr, length);
}
/*
int TClientSocket::readcb(const char* dataptr, size_t length)
{
	TArgPool ArgPoolObj;
	ArgPoolObj.AddArg((double)socket_fd);
	ArgPoolObj.AddArg(std::string(dataptr, length));
	int Ret = CallLua(LState, read_ref, ArgPoolObj);
	if ( 0 != Ret)
	{
		_ERROR("call function failed:%d", Ret);
		return -1;
	}
	return length;
}
*/
bool TClientSocket::IsGateWayDataThrowed()
{
	return this->throwGW == 1;
}

void TClientSocket::SetGateWayDataThrowed()
{
	this->throwGW = 1;
}


//int TClientSocket::readcb(const char* dataptr, size_t length, size_t zip, size_t pro)
int TClientSocket::readcb(std::vector<PtoNode>& PtoList)
{
	lua_getref(LState, read_ref);
	if( ! lua_isfunction(LState, -1) )
		return -1;
	lua_pushnumber(LState, socket_fd);
	lua_newtable(LState);
	for(unsigned int i = 0; i < PtoList.size(); i++){
		lua_pushnumber(LState, i+1);
		lua_newtable(LState);

		lua_pushstring(LState, "data");
		lua_pushlstring(LState, PtoList[i].data.c_str(), PtoList[i].data.length());
		lua_rawset(LState, -3);

		lua_pushstring(LState, "zip");
		lua_pushinteger(LState, PtoList[i].zip);
		lua_rawset(LState, -3);

		lua_pushstring(LState, "proid");
		lua_pushinteger(LState, PtoList[i].proid);
		lua_rawset(LState, -3);
		
		lua_rawset(LState, -3);
	}
	int Ret = callscript(LState, 2, LUA_MULTRET);
	if ( 0 != Ret)
	{
		_ERROR("%s\ncall function failed:%d", luaL_checkstring(LState, -1), Ret);
		return -1;
	}
	return Ret;
}

int TClientSocket::writecb()
{
	TArgPool ArgPoolObj;
	ArgPoolObj.AddArg((double)socket_fd);
	int Ret = CallLua(LState, write_ref, ArgPoolObj);
	if ( 0 != Ret)
	{
		_ERROR("call function failed:%d", Ret);
		return -1;
	}
	return 0;
}

int TClientSocket::on_error(short flag)
{
	TArgPool ArgPoolObj;
	ArgPoolObj.AddArg((double)socket_fd);
	ArgPoolObj.AddArg((double)flag);
	int Ret = CallLua(LState, error_ref, ArgPoolObj);
	if ( 0 != Ret)
	{
		_ERROR("call function failed:%d", Ret);
		return -1;
	}
	return 0;
}

char* TClientSocket::GetReadBuffer()
{
	return this->ReadBuffer;
}

int TClientSocket::DelReadBufferData(unsigned int length)
{
	this -> curlen = this -> curlen - length;
	memmove(this -> ReadBuffer, this -> ReadBuffer + length, this -> curlen);
	return this -> curlen;
}

int IsSocketFd(lua_State* L)
{
	int socket_fd = luaL_checkint(L, 1);
	if (!is_socketfd(socket_fd))
		lua_pushnil(L);
	else
		lua_pushnumber(L, socket_fd);
	return 1;
}

bool is_socketfd(int socket_fd)
{
	struct stat stat_buf;
	bzero(&stat_buf, sizeof(stat_buf));
	int res = fstat(socket_fd, &stat_buf);
	if ( res < 0 )
		return false;
	return S_ISSOCK(stat_buf.st_mode);
}

int checksocketfd(lua_State* L, int narg)
{
	int socket_fd = luaL_checkint(L, narg);
	if (! is_socketfd(socket_fd) )
	{
		const char *msg = lua_pushfstring(L, "socket fd expected, got %s", luaL_typename(L, narg));
		luaL_argerror(L, narg, msg);
	}
	return socket_fd;
}

static int create_socket(lua_State* L)
{
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		lua_pushnil(L);
	    lua_pushstring(L, strerror(errno));
		return 2;
	}
	socket_setnonblocking(socket_fd);
	lua_pushnumber(L, socket_fd);
	return 1;
}

static int bind_socket(lua_State* L)
{
	int socket_fd = checksocketfd(L, 1);
	const char* address = luaL_checkstring(L, 2);
	unsigned short port = luaL_checkint(L, 3);
	struct sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(port);
	local.sin_family = AF_INET;
	//'*' means INADDR_ANY
	if (strcmp(address, "*") && !inet_pton(AF_INET, address, &local.sin_addr)) 
	{
		lua_pushnil(L);
		lua_pushfstring(L, "ip address or * expected, got %s", address);
		return 2;
	}
	int optval = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		_RUNTIME_ERROR("socket %d setsockopt SO_REUSEADDR failed errno=%d", socket_fd, errno);
	}

	if ( 0 != bind(socket_fd, (struct sockaddr *) &local, sizeof(local)) )
	{
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	lua_pushnumber(L, socket_fd);
	return 1;
}

static int listen_socket(lua_State* L)
{
	int narg = lua_gettop(L);
	if (narg != 2)
	{
		luaL_error(L, "need 2 arguments:fd,read_cb");
		return 0;
	}

	int read_ref = -1;
	int socket_fd = checksocketfd(L, 1);
	
	luaL_checktype(L, 2, LUA_TFUNCTION);
	read_ref = lua_ref(L, true);	

	if ( 0 != listen(socket_fd, 5) )
	{
		lua_pushnil(L);
		lua_pushfstring(L, strerror(errno));
		return 2;
	}
	/*
	if (AllSocket[socket_fd])
	{
		_RUNTIME_ERROR("socket %d is already in AllSocket", socket_fd);
	}
	*/
	TServerSocket* socket_obj = new TServerSocket(socket_fd, read_ref);
	//AllSocket[socket_fd] = socket_obj;
	//lua_pushnumber(L, socket_fd);
	lua_pushlightuserdata(L, socket_obj);
	return 1;
}

static int connect_socket(lua_State* L)
{
	int narg = lua_gettop(L);
	if (narg != 7)
	{
		luaL_error(L, "need 6 arguments:fd, ip, port, read_cb, write_cb, error_cb, sock_type");	
		return 0;
	}

	int socket_fd = checksocketfd(L, 1);
	const char* ip_address = luaL_checkstring(L, 2);
	int port = luaL_checkint(L, 3);
	int sock_type = lua_tointeger(L, 4);

	int read_ref = -1, write_ref = -1, error_ref = -1;
	luaL_checktype(L, 5, LUA_TFUNCTION);
	luaL_checktype(L, 6, LUA_TFUNCTION);
	luaL_checktype(L, 7, LUA_TFUNCTION);
	
	error_ref = lua_ref(L, true);
	write_ref = lua_ref(L, true);
	read_ref = lua_ref(L, true);

	struct sockaddr_in remote;
	memset(&remote, 0, sizeof(remote));
	if (!inet_pton(AF_INET, ip_address, &remote.sin_addr))
	{
		lua_pushnil(L);
		lua_pushfstring(L, "ip address expected, got %s", ip_address);
		return 2;
	}
	remote.sin_port = htons(port);
	remote.sin_family = AF_INET;
	int err = connect(socket_fd, (struct sockaddr*)&remote, sizeof(remote));
	if (err < 0 and errno != EINPROGRESS )
	{
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	/*
	if (AllSocket[socket_fd])
	{
		_RUNTIME_ERROR("socket %d is already in AllSocket", socket_fd);
	}
	*/
	TClientSocket* socket_obj = new TClientSocket(socket_fd, sock_type, read_ref, write_ref, error_ref);
	//AllSocket[socket_fd] = socket_obj;
	lua_pushlightuserdata(L, socket_obj);
	return 1;
}

static int accept_socket(lua_State* L)
{
	int narg = lua_gettop(L);
	if (narg != 5)
	{
		luaL_error(L, "need 5 arguments:fd, read_cb, write_cb, error_cb, sock_type");	
		return 0;
	}
	int socket_fd = checksocketfd(L, 1);
	int read_ref = -1, write_ref = -1, error_ref = -1;
	int sock_type = lua_tointeger(L, 2);
	luaL_checktype(L, 3, LUA_TFUNCTION);
	luaL_checktype(L, 4, LUA_TFUNCTION);
	luaL_checktype(L, 5, LUA_TFUNCTION);
	
	error_ref = lua_ref(L, true);
	write_ref = lua_ref(L, true);
	read_ref = lua_ref(L, true);

	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	int client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_len);
	if (client_fd < 0 )
	{
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		lua_pushnumber(L, errno);
		return 3;
	}

	/*
	if (AllSocket[client_fd])
	{
		_RUNTIME_ERROR("socket %d is already in AllSocket", client_fd);
	}
	*/
	TClientSocket* socket_obj = new TClientSocket(client_fd, sock_type, read_ref, write_ref, error_ref);
	//AllSocket[client_fd] = socket_obj;
	//????fd??????ip
	char client_ip[256] = {0};
	inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
	lua_pushnumber(L, client_fd);
	lua_pushlightuserdata(L, socket_obj);
	lua_pushstring(L, client_ip);
	return 3;
}

static int write_policy(lua_State* L)
{
	TClientSocket* socket_obj =(TClientSocket*) lua_touserdata(L, 1);
	size_t length = 0;
	const char* data = luaL_checklstring(L, 2, &length);
	if ( !socket_obj )
	{
		lua_pushnil(L);
		lua_pushstring(L, "cann't find obj for socketobj");
		return 2;
	}
	lua_pushnumber(L, socket_obj->write(data, length));
	return 1;
}

static int write_socket(lua_State* L)
{
	TClientSocket* socket_obj =(TClientSocket*)lua_touserdata(L, 1);
	if ( !socket_obj )
	{
		lua_pushnil(L);
		lua_pushstring(L, "cann't find obj for socketobj");
		return 2;
	}
	size_t length = 0;
	Head head;
	const char *temp = luaL_checklstring(L, 2, &length);
	head.len = htonl(length);
	if (!lua_isnumber(L, 3))
	{
		lua_pushnil(L);
		lua_pushstring(L, "number expected on param 3");
		return 2;
	}
	head.zip = lua_tonumber(L, 3);
	if (!lua_isnumber(L, 4))
	{
		lua_pushnil(L);
		lua_pushstring(L, "number expected on param 4");
		return 2;
	}
	head.pro = htons(lua_tonumber(L, 4));
	size_t write_len = socket_obj -> write((char *)&head, sizeof(head));
	write_len += socket_obj -> write(temp, length);
	lua_pushnumber(L, write_len);
	return 1;
}

static int close_socket(lua_State* L)
{
	int err = errno;
	if( lua_gettop(L) > 1 ){
		TSocket* socket_obj = (TSocket*)lua_touserdata(L, 2); 
		delete socket_obj;
	}else{
		int socket_fd = checksocketfd(L, 1); 
		close(socket_fd);
	}   
	if (err != errno)
	{   
		lua_pushnumber(L, errno);
		lua_pushstring(L, strerror(errno));
		return 2;
	}   
	lua_pushnumber(L, 0); 
	return 1;
}

struct http_data 
{
	struct evhttp_request * req;
	struct evhttp_connection * conn;
	//lua_State * L;
	int cb;
};

void free_http(struct http_data * hd)
{
	//evhttp_request_free(hd->req);
	evhttp_connection_free(hd->conn);
	lua_unref(LState, hd->cb);
	free(hd);
}

void http_cb(struct evhttp_request * req, void *arg)
{
	struct http_data * hd = (struct http_data *) arg;

	TArgPool ArgPoolObj;
	if (req != NULL)
	{
		//lua_pushnumber(LState, req->response_code);
		ArgPoolObj.AddArg((double)req->response_code);
		struct evbuffer * eb = req->input_buffer;
		//??????????????nil???????????????
		if ( EVBUFFER_LENGTH(eb) > 0 )
		{
			ArgPoolObj.AddArg(std::string((char*)EVBUFFER_DATA(eb), EVBUFFER_LENGTH(eb)));
			//lua_pushlstring(LState, (char *)EVBUFFER_DATA(eb), EVBUFFER_LENGTH(eb));
		}
		else
		{
			ArgPoolObj.AddArg();
			//lua_pushnil(LState);
		}
	}
	else
	{
		//lua_pushnumber(LState, -1); //http connect time out
		//lua_pushnil(LState);
		ArgPoolObj.AddArg((double)-1);
		ArgPoolObj.AddArg();
	}
	int Ret = CallLua(LState, hd->cb, ArgPoolObj);
	if (Ret != 0)
	{
		_ERROR("call function failed:%d,%s", Ret, luaL_checkstring(LState, -1));
		free_http(hd);
		return;
	}
	free_http(hd);
}

static int http_request(lua_State * L, enum evhttp_cmd_type type)
{
	const char * addr = luaL_checkstring(L, 1);
	int port = (int)luaL_checknumber(L, 2);
	size_t len = 0;
	const char * _uri = luaL_checklstring(L, 3, &len);
	char reqstr[len + 1];
	memset(reqstr, '\0', len + 1);
	memcpy(reqstr, _uri, len);
	if (!lua_isfunction(L, 4)) {
		luaL_error(L, "need http call back func");
		return 0;
	}
	//??callback ??????????
	lua_pushvalue(L, 4);
	int cb = lua_ref(L, true);

	struct http_data * hd = (struct http_data *)malloc(sizeof(struct http_data));
	hd->cb = cb;
	
	struct evhttp_request * req = evhttp_request_new(http_cb, (void *)hd);
	hd->req = req;
	hd->req->major = 1;
	hd->req->minor = 1;//HTTP/1.0
	struct evhttp_connection * conn = evhttp_connection_new(addr, port);
	hd->conn = conn;

	//add http head
	//http head???????????table????????????????????
	if (lua_istable(L, -1))
	{
		int head_index = lua_gettop(L);
		lua_pushnil(L);
		while( lua_next(L, head_index) )
		{
			const char* value = luaL_checkstring(L, -1);
			const char* key = luaL_checkstring(L, -2);
			//for debug
			//std::cout << key << ":" << value << std::endl;
			evhttp_add_header(req->output_headers, key, value);
			//remove value
			lua_pop(L, 1);
		}
	}

	if (type == EVHTTP_REQ_POST)
	{
		char* dataptr = strchr(reqstr, '?');
		if (dataptr)
		{
			*dataptr = '\0';
			dataptr += 1;
			if (evbuffer_add( req->output_buffer, dataptr, strlen(dataptr) ) )
			{
				luaL_error(L, "evbuffer_add failed:%s", dataptr);
				free_http(hd);
				return 0;
			}
		}	
	}

	if (evhttp_make_request(conn, req, type, reqstr)) {
		luaL_error(L, "evhttp request failed:%s %d %s", addr, port, reqstr);
		free_http(hd);
		return 0;
	}

	return 0;
}

static int http_get(lua_State * L)
{
	return http_request(L, EVHTTP_REQ_GET);
}

static int http_post(lua_State * L)
{
	return http_request(L, EVHTTP_REQ_POST);
}

void dns_callback(int result, char type, int count, int ttl, void *addresses, void *arg)
{
	union resolve_arg_t
	{
		void * p;
		int ref;
	} resolve_arg;
	resolve_arg.p = arg;
	int cb_ref = resolve_arg.ref;
	
	TArgPool ArgPoolObj;
	const char* ip = NULL;
	if (result==DNS_ERR_NONE && type==EVDNS_TYPE_A && count!=0)
	{
		ip = inet_ntoa(((struct in_addr *)addresses)[0]);
		ArgPoolObj.AddArg(std::string(ip));
	}
	else
	{
		ArgPoolObj.AddArg();
	}
	//for debug
	//std::cout << "Call DNS CallBack size " << ArgPoolObj.Size() << "ip:" << ip << " DESC:" << ArgPoolObj.Desc() << std::endl;
	int Ret = CallLua(LState, cb_ref, ArgPoolObj);
	if (Ret != 0)
	{
		_ERROR("dns callback:%d, %s", Ret, luaL_checkstring(LState, -1));
	}
	lua_unref(LState, cb_ref);
}

int init_dns()
{
	if (evdns_init()) {
		_ERROR("evdns init failed");	
		return -2;
	}
	if (evdns_resolv_conf_parse(DNS_OPTIONS_ALL, RESOLV) ) {
		_ERROR("evdns resolv conf load failed:%s", RESOLV);
		return -1;
	}
	return 0;
}

static int resolve(lua_State * L)
{
	const char * name = luaL_checkstring(L, 1);
	if (!lua_isfunction(L, 2)) {
		luaL_error(L, "dns resolve call back function is not valid");
		return 0;
	}

	int cb = lua_ref(L, true);

	evdns_resolve_ipv4(name, 0, dns_callback, (void *)cb);
	return 0;
}





/*
	Http server is added by Ray in 2013.06.27.
*/
void GenericHandler(evhttp_request *req, void *arg)
{
	lua_State *L = (lua_State *)arg;
	lua_getglobal (L, "HTTP");			// call funciton HTTP.ProHttpRequest in Lua
	lua_getfield(L, -1, "ProHttpRequest");
	if (!lua_isfunction(L, -1))
	{
		printf("httpsvr: no function 'ProHttpRequest' found!\n");
		return;
	}
	
//	const struct evhttp_uri * uri_elems = evhttp_request_get_evhttp_uri(req);
//	const char* host = evhttp_uri_get_host(uri_elems);
	const char* host = req->remote_host;
	if (NULL == host)
		lua_pushnil(L);
	else
		lua_pushstring(L, host);
//	int port = evhttp_uri_get_port(uri_elems);
	int port = req->remote_port;
	if (-1 == port)		//  port 80 is default
		port = 80;
	lua_pushnumber(L, port);
	
	
	char *decoded_uri = evhttp_decode_uri(req->uri);
	if (NULL == decoded_uri)
	{
		luaL_error(L, "Decode url failed.");
		return;
	}
	lua_pushstring(L, decoded_uri);
	callscript(L, 3, 1);
	free(decoded_uri);
	
	size_t dataLen;
	const char *data = lua_tolstring(L, -1, &dataLen);
	lua_pop(L, 1);
	
	struct evbuffer *buf = evbuffer_new();
	if (NULL == buf)
	{
		_ERROR("%s", "httpsvr: failed to create response buffer.\n");
		return;
	}
	evbuffer_add(buf, data, dataLen);
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);
}

int InitHttp(lua_State *L, int Port)
{
    struct evhttp *httpd;
	httpd = evhttp_start("0.0.0.0", Port);
	if (NULL == httpd)
	{
		printf("httpsvr: start failed!\n");
		return 0;
	}
	evhttp_set_gencb(httpd, GenericHandler, L);
	printf("httpsvr: start ok on %d.\n", Port);
	return 1;
}

static int init_http_svr(lua_State *L)
{
	lua_Number port = lua_tonumber(L, -1);
	if(!port)
	{
		luaL_error(L, "Invaid httpSvr start port: %s.", lua_tostring(L, -1));
		return 0;
	}
	int result = InitHttp(L, port);
	lua_pushnumber(L, result);
	return 1;
}


static size_t CurlGetCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::string  *pResult = (std::string *)userdata;	
	unsigned long sizes = size * nmemb;
	pResult -> append((char*)ptr, sizes);
	return sizes;
}

// lua calls as CurlHttpGet(url, timeout_sec)
int CurlHttpGet(lua_State *L)
{
	float timeout_sec = lua_tonumber(L, -1);
	if(!timeout_sec)
	{
		luaL_error(L, "CurlHttpGet faild: number expected on param #2.\n");
		return 0;
	}
	const char *url = lua_tostring(L, -2);
	if (NULL == url)
	{
		luaL_error(L, "CurlHttpGet faild: String expected on param #1.\n");
		return 0;
	}
	lua_pop(L, 2);
	
	CURL * curl = curl_easy_init();
	if (NULL == curl)
	{
		luaL_error(L, "curl_easy_init() failed");
		return 0;
	}

/*    struct curl_slist *headerlist = NULL;

	char buf[128] = {0};
	snprintf(buf, sizeof(buf), "%s", "Expect:");
    headerlist = curl_slist_append(headerlist, buf);
	
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);*/

	char errBuffer[CURL_ERROR_SIZE] = {0};
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errBuffer);
	curl_easy_setopt(curl, CURLOPT_URL, url);
// 	for debug	
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 4L);
	
	std::string *Result = new std::string;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, Result);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlGetCallback);

	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (int)(timeout_sec*1000));
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (int)(timeout_sec*1000));

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
//	std::cout << "curlget1: " << *Result << std::endl;
	CURLcode ret = curl_easy_perform(curl);
	if(ret != CURLE_OK)
	{
		lua_pushnil(L);
		lua_pushlstring(L, errBuffer, CURL_ERROR_SIZE);
		//luaL_error(L, "curl get %s faild: %s.\n", url, errBuffer);
		curl_easy_cleanup(curl);
		delete Result;
		return 2;
	}
//	std::cout << "curlget2: " << *Result << std::endl;
	int http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
//	curl_slist_free_all(headerlist);
	curl_easy_cleanup(curl);
//	std::cout << "curlget3: " << *Result << std::endl;
	lua_pushlstring(L, Result->data(), Result->length());
	lua_pushnumber(L, http_code);
	delete Result;
	return 2;
}

// lua calls as CurlHttpPost(url, field, timeout_sec)
int CurlHttpPost(lua_State *L)
{
	float timeout_sec = lua_tonumber(L, -1);
	if(!timeout_sec)
	{
		luaL_error(L, "CurlHttpPost faild: number expected on param #3.\n");
		return 0;
	}
	const char *field = lua_tostring(L, -2);
	if (NULL == field)
	{
		luaL_error(L, "CurlHttpPost faild: String expected on param #2.\n");
		return 0;
	}
	const char *url = lua_tostring(L, -3);
	if (NULL == url)
	{
		luaL_error(L, "CurlHttpPost faild: String expected on param #1.\n");
		return 0;
	}
	lua_pop(L, 3);
	
	CURL * curl = curl_easy_init();
	if (NULL == curl)
	{
		luaL_error(L, "curl_easy_init() failed");
		return 0;
	}

	char errBuffer[CURL_ERROR_SIZE] = {0};
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errBuffer);
	curl_easy_setopt(curl, CURLOPT_URL, url);
// 	for debug	
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);	
	
	std::string *Result = new std::string;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, Result);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlGetCallback);

	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (int)(timeout_sec*1000));
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (int)(timeout_sec*1000));

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, field);
	
	CURLcode ret = curl_easy_perform(curl);
	if(ret != CURLE_OK)
	{
		lua_pushnil(L);
		lua_pushlstring(L, errBuffer, CURL_ERROR_SIZE);
//		luaL_error(L, "CurlHttpPost %s faild: %s.\n", url, errBuffer);
		curl_easy_cleanup(curl);
		delete Result;
		return 2;
	}

	int http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	curl_easy_cleanup(curl);
	
	lua_pushlstring(L, Result->data(), Result->length());
	lua_pushnumber(L, http_code);
	delete Result;
	return 2;
}


static const luaL_reg netlib[] = {
	{"socket", create_socket},
	{"bind", bind_socket},
	{"listen", listen_socket},
	{"connect", connect_socket},
	{"accept", accept_socket},
	{"write", write_socket},
	{"IsSocketfd", IsSocketFd},
	{"writePolicy", write_policy},
	{"close", close_socket},
	{"http_get", http_get},
	{"http_post", http_post},
	{"dns_resolve",  resolve},
	{"http_svr_start",  init_http_svr},
	{"curlGet",  CurlHttpGet},
	{"curlPost",  CurlHttpPost},
	
	{NULL, NULL},
};



void luaopen_netlib(lua_State* L)
{
	init_dns();
	luaL_register(L, "netlib", netlib);
}

