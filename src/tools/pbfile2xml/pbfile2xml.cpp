#include <iostream>
#include <stdio.h>
#include "ProtoBuf.h"
#include "Log.h"

using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::compiler;

Logger* g_log = NULL;

static void usage(const char* programName)
{
    printf("\n");
    printf("Usage: %s [OPTION] \n", programName);
    printf("\n");
    printf("  -p <proto file>    [required] the proto file will be to xml\n");
    printf("  -m <message name>  [required] the message name in proto file will be to xml\n");
    printf("  -i <include dir>   [optional] the include dir for proto file\n");
    printf("  -o <xml file>      [optional] the xml result file will be written\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -p a.proto -m test.Request\n", programName);
}

static string Xml(const Descriptor* d, int blank)
{
    string s;
    for (int i = 0; i < d->field_count(); i++)
    {
        const FieldDescriptor* f = d->field(i);
        s += string(blank, ' ') + "<" + f->name() + ">"; 
        if (f->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE)
        {
            s += "\n" + Xml(f->message_type(), blank + 4) + string(blank, ' ');
        }
        s += "</" + f->name() + ">\n";
    }
    return s;
}

int main(int argc, char* argv[])
{
    string inc = "";
    string file = "";
    string msgName = "";
    string objxml = "";

    while (true)
    {
        int c = getopt(argc, argv, "i:p:hm:o:");
        if (-1 == c)
        {
            break;
        }
        switch (c)
        {
            case 'i':
                inc = optarg;
                break;
            case 'p':
                file = optarg;
                break;
            case 'm':
                msgName = optarg;
                break;
            case 'o':
                objxml = optarg;
                break;
            default :
                usage(argv[0]);
                return -1;
        }
    }
    if (file.empty() || msgName.empty())
    {
        usage(argv[0]);
        return -1;
    }

    DiskSourceTree dst;
    Importer* ipt = NULL;
    const FileDescriptor* fd = NULL;
    if (! ProtoBuf::ImportFile(file, inc, dst, ipt, fd))
    {
        printf("can't import %s\n", file.c_str());
        return -1;
    }
    Message* msg = ProtoBuf::CreateMessage(msgName, ipt);
    if (! msg)
    {
        printf("can't find %s in %s\n", msgName.c_str(), file.c_str());
        return -1;
    }
    string xml = Xml(msg->GetDescriptor(), 0);
    // 写文件
    FILE* fp = NULL;
    if (objxml.empty())
    {
        cout << xml;
    }
    else
    {
        fp = fopen(objxml.c_str(), "w");
        if (NULL == fp)
        {
            printf("open %s failed.", objxml.c_str());
            return -1;
        }
        fwrite(xml.c_str(), xml.size(), 1, fp);
        fclose(fp);
    }
    return 0;
}
