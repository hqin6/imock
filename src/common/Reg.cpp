#include "Reg.h"
#include "Log.h"

using namespace std;

Reg::Reg(const string& s)
{
    m_regs = s;
}

bool Reg::Init()
{
    int ret = 0;
    int flags = REG_NOSUB | REG_EXTENDED;
    if (0 != (ret = regcomp(&m_reg, m_regs.c_str(), flags)))
    {
        char buf[128];
        regerror(ret, &m_reg, buf, sizeof(buf));
        GLOG(IM_ERROR, "reg(%s) error:%s", m_regs.c_str(), buf);
        return false;
    }
    return true;
}

bool Reg::Match(const string& s) const
{
    if (0 == regexec(&m_reg, s.c_str(), 0, NULL, 0))
    {
        return true;
    }
    return false;
}

const string& Reg::GetPattern()const
{
    return m_regs;
}
