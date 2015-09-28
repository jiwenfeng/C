#include "global.hpp"
#include "callout.hpp"
#include "oauth.h"			// for 'oauth_sign_hmac_sha1_raw' in function '_hamc_sha1'

void luaopen_lengine(lua_State* L);
int base64_encode(const char* input, int length, char* buffer, int buff_size);
