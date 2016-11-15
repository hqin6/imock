/**
 * 文件名称：ProtoBuf.h
 * 摘要：
 *
 * 作者：heqin<hqin6@163.com>
 * 日期：2015.02.13
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef _PROTOBUF_H_
#define _PROTOBUF_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h> 


class ProtoBuf
{
    public:
        static ::google::protobuf::Message* CreateMessage(const std::string& name,
                ::google::protobuf::compiler::Importer* ipt);

        static const ::google::protobuf::Message* GetMessageProtoType(const std::string& name,
                ::google::protobuf::compiler::Importer* ipt);

        static bool ImportFile(
                const std::string& pbfile, const std::string& pbinc,
                google::protobuf::compiler::DiskSourceTree& dst,
                google::protobuf::compiler::Importer*& ipt,
                const google::protobuf::FileDescriptor*& fd);
        static bool ParseFromString(::google::protobuf::Message* msg, const std::string& s);
};


#endif
