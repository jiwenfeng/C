#include <sys/types.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <iconv.h>
#include "api.hpp"
#include "cjson.hpp"
#include "base64.hpp"
#include "luacall.hpp"
#include "zlib.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"

#define MD5_LENGTH 16

static void multi_cb(void * arg)
{
	union unpack_arg{
		void* p;
		int ref;
	} uarg;
	uarg.p = arg;

	TArgPool ArgPoolObj;
	int Ret = CallLua(LState, uarg.ref, ArgPoolObj);

	if (Ret != 0) {
		_ERROR("call multi_cb function failed:%d,%s", Ret, luaL_checkstring(LState, -1));
		return;
	}
}

static void once_cb(void * arg)
{
	multi_cb(arg);
	union unpack_arg{
		void* p;
		int ref;
	} uarg;
	uarg.p = arg;
	lua_unref(LState, uarg.ref);
}

typedef unsigned int (*call_func)(callcb cb, void * arg, float timeout);
static int _call(lua_State * L, call_func func, callcb cb)
{

	if (!lua_isfunction(L, 1)) {
		luaL_error(L, "need function to call out");
		return 0;
	}

	float timeout = luaL_checknumber(L, 2);

	if(timeout < 1)
	{
		luaL_error(L, "timeout must be greater than 1");
		return 0;
	}

	lua_pop(L, 1); //pop timeout.

	int ref = lua_ref(L, true);
	unsigned int handler = func(cb, (void *)ref, timeout);
	lua_pushnumber(L, handler);
	return 1;
}

static int _call_once(lua_State * L)
{
	return _call(L, call_once, once_cb);
}

static int _call_multi(lua_State * L)
{
	return _call(L, call_multi, multi_cb);
}

static int _rm_call(lua_State * L)
{
	unsigned int handler = luaL_checkint(L, 1);
	void * arg;
	if (rm_call(handler, &arg)) {
		union unpack_arg{
			void* p;
			int ref;
		} uarg;
		uarg.p = arg;
		lua_unref(LState, uarg.ref);
		lua_pushboolean(L, 1);
		return 1;
	}
	
	lua_pushboolean(L, 0);
	return 1;
}

unsigned int md5_sum(const char* input, int length, char* buffer, int buff_size)
{
	unsigned int len;
	if( buff_size < MD5_LENGTH)
		return 0;
	EVP_MD_CTX mdctx;
	EVP_DigestInit(&mdctx, EVP_md5());
	EVP_DigestUpdate(&mdctx, (unsigned char*)input, length);
	EVP_DigestFinal(&mdctx, (unsigned char*)buffer, &len);
	return len;
}

static int _zlib_compress(lua_State *L)
{
	size_t srclen = 0;
	const char* srcstr = luaL_checklstring(L, 1, &srclen);
	unsigned long destlen = 1024 * 1024;
	char deststr[destlen];
	bzero(deststr, destlen);
	int iRet = ::compress((Bytef *)deststr, &destlen, (Bytef *)srcstr, (unsigned long)srclen);
	if(iRet == Z_OK)
	{
		lua_pushlstring(L, deststr, destlen);
		return 1;
	}
	else
	{
		_ERROR("%s compress error for %d\n", srcstr, iRet);	
		return 0;
	}
}

static int _zlib_uncompress(lua_State *L)
{
	size_t srclen = 0;
	const char* srcstr = luaL_checklstring(L, 1, &srclen);
	unsigned long destlen = 1024 * 1024;
	char deststr[destlen];
	bzero(deststr, destlen);
	int iRet = ::uncompress((Bytef *)deststr, &destlen, (Bytef *)srcstr, (unsigned long)srclen);
	if(iRet == Z_OK)
	{
		lua_pushlstring(L, deststr, destlen);
		return 1;
	}
	else
	{
		_ERROR("%s uncompress error for %d\n", srcstr, iRet);	
		return 0;
	}
}

static int _rsa_sign(lua_State *L)
{
	BIO *private_file = BIO_new(BIO_s_file());
	if(!BIO_read_filename(private_file, "key/rsa_private.pem"))
	{
		_ERROR("private_file load error\n");	
		lua_pushnil(L);
		return 1;
	}
 	RSA* private_key = PEM_read_bio_RSAPrivateKey(private_file, NULL, NULL, NULL);
	if(!private_key)
	{
		_ERROR("private_key load error\n");	
		lua_pushnil(L);
		return 1;
	}
	size_t source_len = 0;
	unsigned char* source = (unsigned char*)luaL_checklstring(L, 1, &source_len);
	int s_type = RSA_PKCS1_PADDING;
	unsigned int destlen = RSA_size(private_key);
	char dest[destlen];
	bzero(dest, destlen);
	int res = RSA_private_encrypt(source_len, source, (unsigned char*)dest, private_key, s_type);
	RSA_free(private_key);
	BIO_free_all(private_file);
	if(res == -1)
	{
		_ERROR("%s sign error for %d\n", source, res);	
		lua_pushnil(L);
		return 1;
	}
	lua_pushlstring(L, dest, destlen);
	return 1;
}

static int _rsa_verify(lua_State *L)
{
	BIO *public_file = BIO_new(BIO_s_file());
	if(!BIO_read_filename(public_file, "key/rsa_public.pem"))
	{
		_ERROR("public_file load error\n");	
		lua_pushnil(L);
		return 1;
	}
 	RSA* public_key = PEM_read_bio_RSA_PUBKEY(public_file, NULL, NULL, NULL);
	if(!public_key)
	{
		_ERROR("public_key load error\n");	
		lua_pushnil(L);
		return 1;
	}
	unsigned int destlen = RSA_size(public_key);
	char dest[destlen];
	bzero(dest, destlen);
	size_t source_len = 0;
	unsigned char* source = (unsigned char*)luaL_checklstring(L, 1, &source_len);
	int s_type = RSA_PKCS1_PADDING;
	int res = RSA_public_decrypt((int)source_len, source, (unsigned char*)dest, public_key, s_type);
	RSA_free(public_key);
	BIO_free_all(public_file);
	if(res == -1)
	{
		_ERROR("%s verify error for %d\n", source, res);	
		lua_pushnil(L);
		return 1;
	}
	lua_pushstring(L, dest);
	return 1;
}

static int _base64_encode(lua_State* L)
{
	size_t len = 0;
	const char* str = luaL_checklstring(L, 1, &len);
	size_t size = BASE64_LENGTH(len) + 1;
	char* buff = (char*)malloc(size);
	bzero(buff, size);
	base64_encode((const unsigned char*)str, len, buff, size);
	lua_pushstring(L, buff);
	free(buff);
	buff = NULL;
	return 1;
}

static int _base64_decode(lua_State* L)
{
	size_t len = 0;
	const char* str = luaL_checklstring(L, 1, &len);
	char* output = (char*)malloc(len);
	bzero(output, len);
	int size = base64_decode(str, (unsigned char*)output, len);
	if( size <= 0 )
	{
		lua_pushnil(L);
	}
	else
	{
		lua_pushlstring(L, output, size);
	}
	free(output);
	output = NULL;
	return 1;
}

static int _md5_sum(lua_State* L)
{
	size_t len = 0;
	const char* str = luaL_checklstring(L, 1, &len);
	char output[MD5_LENGTH];
	bzero(output, sizeof(output));
	unsigned int size = md5_sum(str, len, output, sizeof(output));
	if( size <= 0 )
	{
		lua_pushnil(L);
		return 1;
	}
	char md5str[MD5_LENGTH*2+1];
	bzero(md5str, sizeof(md5str));
	for( size_t i = 0; i < size; i++)
	{
		sprintf(md5str+i*2, "%02x", (unsigned char)output[i]);
	}
	lua_pushstring(L, md5str);
	lua_pushlstring(L, output, size);
	return 2;
}

static int _lengine_exit(lua_State* L)
{
	int count = lua_gettop(L);
	if(count == 2)
	{
		int arg = luaL_checkint(L, 1);
		const char* str = luaL_checkstring(L, 2);
		_ERROR("lengine is exit for %d:%s", arg, str);	
		exit(arg);
	}
	else if(count == 1)
	{
		int arg = luaL_checkint(L, 1);
	   	_ERROR("lengine is exit for %d", arg);
		exit(arg);
	}
	else
	{
		_ERROR("lengine is exit");	
		exit(0);
	}
	return 0;
}

void json2lua(lua_State* L, const cJSON* json)
{
	int i = 1;
	cJSON* child = NULL;
	switch( json->type )
	{
		case cJSON_NULL:
			lua_pushnil(L);
			break;
		case cJSON_True:
			lua_pushboolean(L, 1);
			break;
		case cJSON_False:
			lua_pushboolean(L, 0);
			break;
		case cJSON_Number:
			lua_pushnumber(L, json->valuedouble);
			break;
		case cJSON_String:
			lua_pushstring(L, json->valuestring);
			break;
		case cJSON_Array:
			lua_newtable(L);
			child = json->child;
			while( child )
			{
				lua_pushnumber(L, i++);
				json2lua(L, child);
				lua_settable(L, -3);
				child = child->next;
			}
			break;
		case cJSON_Object:
			lua_newtable(L);
			child = json->child;
			while( child )
			{
				lua_pushstring(L, child->string);
				json2lua(L, child);
				lua_settable(L, -3);
				child = child->next;
			}
			break;
	}
}

static int _json_decode(lua_State* L)
{
	const char* str = luaL_checkstring(L, 1);
	cJSON* json = cJSON_Parse(str);
	if ( ! json )
	{
		lua_pushnil(L);
	}
	else
	{
		json2lua(L, json);
		cJSON_Delete(json);
		json = NULL;
	}
	return 1;
}

static int _utf8_encode(lua_State* L)
{
	size_t len = 0;
	const char* str = luaL_checklstring(L, 1, &len);
	const char* from_code = luaL_checkstring(L, 2);
	const char* to_code = luaL_checkstring(L, 3);
	iconv_t handle = iconv_open(to_code, from_code);
	if( handle == (iconv_t)(-1) )
	{
		lua_pushnil(L);
		return 1;
	}
	char* input = (char*)malloc(len+1);
	strncpy(input, str, len+1);
	char* ptrin = input;
	char* output = (char*)malloc(len*2);
	size_t out_len = len*2;
	int ret = 0;
	char* ptrout = output;
	iconv(handle, NULL, NULL, NULL, NULL);
#ifdef __FreeBSD__
	ret = iconv(handle, &str, &len, &output, &out_len);
#else
	ret = iconv(handle, &input, &len, &output, &out_len);
#endif
	if( ret < 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	//printf("_utf8_encode:str:%p, len:%d, ptrout:%p, output:%p, out_len:%d, diff:%d, ret:%d\n", str, len, ptrout, output, out_len, output-ptrout, ret);
	lua_pushlstring(L, ptrout, output - ptrout);
	free(ptrout);
	free(ptrin);
	ptrout = output = NULL;
	ptrin = input = NULL;
	return 1;
}

/*
static int name2oid(char *name, int *oidp, size_t len)
{
        int oid[2];

        oid[0] = 0;
        oid[1] = 3;

        int i = sysctl(oid, 2, oidp, &len, name, strlen(name));
        if (i < 0)
                return i;
        return (len/sizeof(int));
}
static int oidfmt(int *oid, int len, char *fmt, u_int *kind)
{
        int qoid[CTL_MAXNAME+2];
        u_char buf[BUFSIZ];
        int i;
        size_t j;

        qoid[0] = 0;
        qoid[1] = 4;
        memcpy(qoid + 2, oid, len * sizeof(int));

        j = sizeof(buf);
        i = sysctl(qoid, len + 2, buf, &j, 0, 0);
        if (i)
		return 1;

        if (kind)
                *kind = *(u_int *)buf;

        if (fmt)
                strcpy(fmt, (char *)(buf + sizeof(u_int)));
        return 0;
}
static int _sysctl(lua_State*L)
{
#define SYSCTL_ERROR(msg) do{\
	lua_pushnil(L);\
	lua_pushstring(L, msg);\
	lua_pushstring(L, strerror(errno));\
	return 3;\
}while(0)

	size_t strlength = 0;
	const char* str = luaL_checklstring(L, 1, &strlength);
	char name[BUFSIZ];
	if (strlength > BUFSIZ-1)
		SYSCTL_ERROR("not buffer enough");
	bzero(name, sizeof(name));
	strncpy(name, str, strlength);
	int mib[CTL_MAXNAME];
	bzero(mib, sizeof(mib));
	int mib_len = name2oid(name, mib, sizeof(mib));
	if (mib_len < 0 )
		SYSCTL_ERROR("name2oid error");
	u_int type = 0;
	if( oidfmt(mib, mib_len, NULL, &type) )
		SYSCTL_ERROR("oidfmt error");
	strlength = sizeof(name);
	if( sysctlbyname(str, name, &strlength, NULL, NULL) )
		SYSCTL_ERROR("sysctlbyname error");
	switch(type & CTLTYPE)
	{
		case CTLTYPE_INT:
			lua_pushnumber(L, *(int*)name);
			break;
		case CTLTYPE_LONG:
			lua_pushnumber(L, *(long*)name);
			break;
		case CTLTYPE_ULONG:
			lua_pushnumber(L, *(unsigned long*)name);
			break;
		case CTLTYPE_UINT:
			lua_pushnumber(L, *(unsigned int*)name);
			break;
		case CTLTYPE_QUAD:
			lua_pushnumber(L, *(LUA_NUMBER*)name);
			break;
		case CTLTYPE_STRING:
			lua_pushlstring(L, name, strlength);
			break;
		default:
			lua_pushnil(L);
			lua_pushstring(L, "invalid type");
			return 2;
	}
	return 1;
}
*/

static int _getrusage(lua_State* L)
{
#define PUSHRUSAGE(name, value) do{\
	lua_pushstring(L, name);\
	lua_pushnumber(L, value);\
	lua_settable(L, -3);\
}while(0)
	struct rusage ret;
	if( getrusage(RUSAGE_SELF, &ret) == 0 )
	{
		lua_newtable(L);	
		PUSHRUSAGE("ru_utime", (ret.ru_utime.tv_sec*1000000 + ret.ru_utime.tv_usec)/1000000.0);
		PUSHRUSAGE("ru_stime", (ret.ru_stime.tv_sec*1000000 + ret.ru_stime.tv_usec)/1000000.0);
		PUSHRUSAGE("ru_maxrss", ret.ru_maxrss);
		PUSHRUSAGE("ru_ixrss", ret.ru_ixrss);
		PUSHRUSAGE("ru_idrss", ret.ru_idrss);
		PUSHRUSAGE("ru_isrss", ret.ru_isrss);
		PUSHRUSAGE("ru_minflt", ret.ru_minflt);
		PUSHRUSAGE("ru_majflt", ret.ru_majflt);
		PUSHRUSAGE("ru_nswap", ret.ru_nswap);
		PUSHRUSAGE("ru_inblock", ret.ru_inblock);
		PUSHRUSAGE("ru_oublock", ret.ru_oublock);
		PUSHRUSAGE("ru_msgsnd", ret.ru_msgsnd);
		PUSHRUSAGE("ru_msgrcv", ret.ru_msgrcv);
		PUSHRUSAGE("ru_nsignals", ret.ru_nsignals);
		PUSHRUSAGE("ru_nvcsw", ret.ru_nvcsw);
		PUSHRUSAGE("ru_nivcsw", ret.ru_nivcsw);
		return 1;
	}
	else
	{
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
}

static int _hamc_sha1(lua_State *L)
{
	size_t key_len, txt_len;
	const char *key = luaL_checklstring(L, -1, &key_len);
	if (NULL == key)
	{
		lua_pushnil(L);
		lua_pushstring(L, "String expected on param #2.");
		return 2;
	}
	const char *txt = luaL_checklstring(L, -2, &txt_len);
	if (NULL == txt)
	{
		lua_pushnil(L);
		lua_pushstring(L, "String expected on param #1.");
	}
	lua_pop(L, 2);
	
	char *result = oauth_sign_hmac_sha1_raw(txt, txt_len, key, key_len);
	lua_pushstring(L, result);
	return 1;
}



static const luaL_reg lengine[] = {
	{"call_multi", _call_multi},
	{"call_once", _call_once},
	{"rm_call", _rm_call},
	{"encode64", _base64_encode},
	{"decode64", _base64_decode},
	{"md5", _md5_sum},
	{"utf8_encode", _utf8_encode},
	{"jsondecode", _json_decode},
	//{"sysctl", _sysctl},
	{"getrusage", _getrusage},
	{"shutdown", _lengine_exit},
	{"compress", _zlib_compress},
	{"uncompress", _zlib_uncompress},
	{"rsa_sign", _rsa_sign},
	{"rsa_verify", _rsa_verify},
	{"hamc_sha1", _hamc_sha1},

	{NULL, NULL},
};

void luaopen_lengine(lua_State* L)
{
	init_callout();
	luaL_register(L, "lengine", lengine);
}
