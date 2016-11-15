#include "MockServerFactory.h"
#include "Log.h"
#include "HttpServerMock.h"

using namespace std;

BaseMock* MockServerFactory::MockMap(const string& p)
{
    if ("http" == p)
    {
        GLOG(IM_DEBUG, "new a http mock");
        return new HttpServerMock();
    }
    else if ("https" == p)
    {
        GLOG(IM_DEBUG, "new a https mock");
        return new HttpServerMock("", true);
    }
    else
    {
        GLOG(IM_ERROR, "unrecognized protocol: %s", p.c_str());
    }
    return NULL;
}
