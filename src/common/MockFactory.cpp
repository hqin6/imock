#include "MockFactory.h"
#include "Log.h"
#include "INIConf.h"

using namespace std;

MockFactory::~MockFactory()
{
}

BaseMock* MockFactory::GetMock(const string& area)
{
    if (! g_iniConf)
    {
        GLOG(IM_ERROR, "no ini conf");
        return NULL;
    }
    string p;
    if (! g_iniConf->Get(area, "protocol", ISet::Str, &p))
    {
        GLOG(IM_ERROR, "no protocol in %s", area.c_str());
        return NULL;
    }
    if (p.empty())
    {
        GLOG(IM_ERROR, "protocol is null in %s", area.c_str());
        return NULL;
    }
    BaseMock* m = MockMap(p);
    if (m)
    {
        m->SetName(area);
    }
    return m;
}
