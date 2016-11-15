#include "MockClientFactory.h"
#include "Log.h"
#include "HttpClientMock.h"

using namespace std;

BaseMock* MockClientFactory::MockMap(const string& p)
{
    if ("http" == p)
    {
        GLOG(IM_DEBUG, "new a http mock");
        return new HttpClientMock();
    }
    else
    {
        GLOG(IM_ERROR, "unrecognized protocol: %s", p.c_str());
    }
    return NULL;
}
