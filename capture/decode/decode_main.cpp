#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>
#include <arpa/inet.h>
#include <errno.h>

#include <stdio.h>
#include "decode_log.h"

using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::compiler;

#define PATH "./decode/protocol/"

class decode_inst
{
public:
	decode_inst(const char *path)
	{
		tree.MapPath("", path);
		importer = new Importer(&tree, NULL);
	}
	~decode_inst()
	{
		delete importer;
	}

	void import_file(const char *file)
	{
		importer->Import(file);
	}

	int decode_msg(const char *str, int sz, char *data, int datalen)
	{
		int offset = 0;
		uint32_t name_len = ntohl(*(uint32_t *)str);
		offset += sizeof(uint32_t);
		char *name = new char[name_len];
		memcpy(name, str + offset, name_len);
		offset += name_len;
		google::protobuf::Message *msg = create_msg(name);
		if(NULL == msg)
		{
			decode_error("Could Not find Message:%s\n", name);
			return 0;
		}
		if(!msg->ParseFromArray(str + offset, sz - offset))
		{
			decode_error("Decode TypeName %s Error:%d\n", name, sz - offset);
			delete msg;
			return 0;
		}
		snprintf(data, datalen, "%s : {\n%s}", msg->GetTypeName().c_str(), msg->DebugString().c_str());
		delete msg;
		return 0;
	}

private:
	google::protobuf::Message *create_msg(const char *type_name)
	{
		const Descriptor *dsc = importer->pool()->FindMessageTypeByName(type_name);
		if(NULL == dsc)
		{
			return NULL;
		}
		const google::protobuf::Message *message = factory.GetPrototype(dsc);
		if(NULL == message)
		{
			return NULL;
		}
		return message->New();
	}

private:
	Importer *importer;
	DynamicMessageFactory factory;
	DiskSourceTree tree;
};

extern "C" void *create()
{
	DIR *dir = opendir(PATH);
	if(NULL == dir)
	{
		decode_error("%s\n", strerror(errno));
		return NULL;
	}
	decode_inst *d = new decode_inst(PATH);
	struct dirent *ent = NULL;
	while((ent = readdir(dir)) != NULL)
	{
		if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
		{
			continue;
		}
		d->import_file(ent->d_name);
	}
	closedir(dir);
	return d;
}

extern "C" int decode(void *arg, const char *str, int length, char *ptr, int buflen)
{
	if((uint32_t)length < sizeof(uint32_t))
	{
		return 0;
	}
	uint32_t sz = ntohl(*(uint32_t *)str);
	if(sz > (uint32_t)length)
	{
		return 0;
	}
	if(sz > (uint32_t)buflen)
	{
		return length;
	}
	decode_inst *d = static_cast<decode_inst *>(arg);
	if(NULL == d)
	{
		return length;
	}
	if(-1 == d->decode_msg(str + sizeof(uint32_t), sz, ptr, buflen))
	{
		return length;
	}
	return sz + sizeof(uint32_t);
}

extern "C" void destroy(void *arg)
{
	decode_inst *d = static_cast<decode_inst *>(arg);
	delete d;
	google::protobuf::ShutdownProtobufLibrary();
}
