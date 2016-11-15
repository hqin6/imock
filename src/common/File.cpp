#include <stdio.h>
#include <string.h>
#include "File.h"
#include "Log.h"

using namespace std;

void File::Write(const string& file, const string& msg, const string& mode)
{
    FILE* p = fopen(file.c_str(), mode.c_str());
    if (!p)
    {
        GLOG(IM_ERROR, "open %s err:%s", 
                file.c_str(), strerror(errno));
        return ;
    }
    fwrite(msg.c_str(), msg.size(), 1, p);
    fclose(p);
    return ;
}
