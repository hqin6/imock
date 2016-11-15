#include "ProtoBuf.h"
#include "EList.h"
#include "Str.h"
#include <libgen.h>
#include "Log.h"
#include "define.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>

using namespace ::google::protobuf;
using namespace std;
using namespace google::protobuf::compiler;
using namespace google::protobuf::io;

class IM_ErrorCollector : public MultiFileErrorCollector
{
    public:
        IM_ErrorCollector(){}
        ~IM_ErrorCollector(){}
        void AddError(const string& filename,
                int line, int column, const string& message) 
        {
            GLOG(IM_ERROR, "%s:%d:%d %s",
                    filename.c_str(), line + 1, column + 1, message.c_str());
        }
        void AddWarning(const string& filename, 
                int line, int column, const string& message) {
            GLOG(IM_WARN, "%s:%d:%d %s",
                    filename.c_str(), line + 1, column + 1, message.c_str());
        }
};

Message* ProtoBuf::CreateMessage(const string& name, Importer* ipt)
{
    const Message* tpl = GetMessageProtoType(name, ipt);
    if (tpl)
    {
        return tpl->New();
    }
    return NULL;
}

const Message* ProtoBuf::GetMessageProtoType(const string& name, Importer* ipt)
{
    const Descriptor* desc = NULL;
    desc = DescriptorPool::generated_pool()->FindMessageTypeByName(name);

    if (desc)
    {
        return MessageFactory::generated_factory()->GetPrototype(desc);
    }
    desc = ipt->pool()->FindMessageTypeByName(name);
    if (desc)
    {
        DynamicMessageFactory* df = new DynamicMessageFactory;
        return df->GetPrototype(desc);
    }
    return NULL;
}

bool ProtoBuf::ImportFile(const string& pbfile, const string& pbinc,
        DiskSourceTree& dst, Importer*& ipt, 
        const FileDescriptor*& fd)
{
    // 分离proto_file的目录和文件名
    char* dd;
    dd = strdup(pbfile.c_str());
    string dir = dirname(dd);
    free(dd);
    dd = strdup(pbfile.c_str());
    string file = basename(dd);
    free(dd);

    // 增加proto_inc
    EList<string> arrPath;
    if (! pbinc.empty())
    {
        Str::SplitTrimNoNull(pbinc, arrPath, ":");
    }

    // 构造proto,遍历所有的service
    dst.MapPath("", dir);
    GLOG(IM_DEBUG, "add %s to map path", dir.c_str());
    for(string* i = arrPath.begin(); ! arrPath.end(); i = arrPath.next())
    {
        GLOG(IM_DEBUG, "add %s to map path", i->c_str());
        dst.MapPath("", *i);
    }
    dst.MapPath("", SETUP_ROOT_DIR "" PROTO_DIR "/");
    dst.MapPath("", SETUP_ROOT_DIR "/dev/proto/");
    ipt = new Importer(&dst, new IM_ErrorCollector());
    fd = ipt->Import(file);
    return fd != NULL;
}

bool ProtoBuf::ParseFromString(Message* msg, const string& s)
{
    if (s.size() < 64 * 1024 * 1024)
    {// 小于64M
        return msg->ParseFromString(s);
    }
    ArrayInputStream input(s.c_str(), s.size());
    CodedInputStream decoder(&input);
    decoder.SetTotalBytesLimit(s.size() + 1, 64 * 1024 * 1024);
    return msg->ParseFromCodedStream(&decoder) && decoder.ConsumedEntireMessage();
}
