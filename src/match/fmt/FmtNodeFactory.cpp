#include "FmtNodeFactory.h"
#include "KVFmtNode.h"
#include "PBFmtNode.h"
//#include "UAFmtNode.h"
//#include "ABCFmtNode.h"
#include "Log.h"
#include "FmtNode.h"
#include "JsonFmtNode.h"

Node* FmtNodeFactory::Get(TiXmlElement* e)
{
    const char* fmt = e->Attribute("fmt");
    if (! fmt)
    {
        GLOG(IM_DEBUG, "%s: new default fmt node", e->Value());
        return new FmtNode();
    }
    if ('k' == *fmt && 'v' == *(fmt+strlen(fmt)-1))
    {
        GLOG(IM_DEBUG, "%s:new kv fmt node", e->Value());
        return new KVFmtNode();
    }
    if (0 == strcmp(fmt, "pb"))
    {
        GLOG(IM_DEBUG, "%s:new pb fmt node", e->Value());
        return new PBFmtNode();
    }
    if (0 == strcmp(fmt, "json"))
    {
        GLOG(IM_DEBUG, "%s:new json fmt node", e->Value());
        return new JsonFmtNode();
    }
    GLOG(IM_DEBUG, "%s: new default fmt node", e->Value());
    return new FmtNode();
}
