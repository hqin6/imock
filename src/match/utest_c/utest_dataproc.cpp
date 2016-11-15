
#include "foo.pb.h"
#include "addr.pb.h"
#include "gtest/gtest.h"
#define private public
#define protected public

#include "Log.h"
#include "Fmt.h"
#include "Dat.h"

using namespace std;
using namespace tutorial;
Logger* g_log;

TEST(all, all)
{
    srand(time(NULL));
    g_log = new Logger("a.log");
    if (! g_log->Load())
    {
        return ;
    }
    bool b;

    Fmt fmt;
    b = fmt.Load("fmt.xml");
    EXPECT_EQ(b, true);
    if (! b)
    {
        return ;
    }

    Dat dat;
    b = dat.Load("dat.xml", &fmt, "", "");
    EXPECT_EQ(b, true);
    if (! b)
    {
        return ;
    }
    EList<QA*>& v = dat.GetQA();
    int size = 5;
    EXPECT_EQ(v.size(), size);
    if (size > v.size())
    {
        return ;
    }
    QA** vit = v.begin();
    {
        QA* qa = *vit;
        RRMessage* req = new RRMessage;
        bool ret = qa->Query(req);
        EXPECT_EQ(ret, true);
        bool m = false;
        if (req->m_body == "zhangsan|1|zs@fuck.com|010-111_MOBILE")
        {
            m = true;
        }
        else if (req->m_body == "|ffff|011-123_,xxx")
        {
            m = true;
        }
        EXPECT_EQ(m, true);
        RRMessage res;
        int len = 10;
        res.m_body = string((char*)&len, sizeof(int)) + "aaa123,tailv";
        bool b = qa->Match(&res, false, "");
        EXPECT_EQ(b, true);
        string s = res.DebugString();
        EXPECT_EQ(s, "a {\n  length : 10\n  data : aaa\n  l2 : 123\n  tail : tailv\n }\n");
        delete req;
    }
    {
        vit = v.next();
        QA* qa = *vit;
        RRMessage* req = new RRMessage;
        bool ret = qa->Query(req);
        EXPECT_EQ(ret, true);
        int a = 10;
        string s((char*)&a, sizeof(int));
        s = "a=" + s + ";c=ccc;b=0-1-2,b21:21b22:22}t100";
        EXPECT_EQ(req->m_body, s);
        RRMessage res;
        Pair p;
        p.set_key("av.bv");
        p.set_value(100);
        string ps;
        p.SerializeToString(&ps);
        char buf[1024];
        snprintf(buf, sizeof(buf), "a=10;b=20;email=1@2|c=30.len=%d:",
                (int)ps.size());
        string ss(buf);
        ss += ps;
        res.m_body = ss;
        bool b = qa->Match(&res, false, "");
        EXPECT_EQ(b, true);
        s = res.DebugString();

        string exp = "a {\n  args {\n    a : 10\n    b : 20\n    email {\n      n : 1\n      t : 2\n     }\n   }\n  args {\n    c : 30\n   }\n  len : len=9\n  k {\n    key {\n      a : av\n      b : bv\n     }\n    value : 100\n   }\n }\n";
        EXPECT_EQ(s, exp);
        delete req;
    }
    {
        vit = v.next();
        QA* qa = *vit;
        RRMessage* req = new RRMessage;
        bool ret = qa->Query(req);
        EXPECT_EQ(ret, true);
        Pair p;
        p.set_key("3;hel1cc");
        p.set_value(200);
        string ps;
        p.SerializeToString(&ps);
        EXPECT_EQ(req->m_body, ps);

        AddressBook ab;
        {
            Person* p = ab.add_person();
            p->set_name("zhangsan");
            p->set_id(3);
            p->set_email("zs@163.com");
            Person::PhoneNumber* pp = p->add_phone();
            pp->set_number("010-11");
            pp->set_type(Person_PhoneType_WORK);
            pp = p->add_phone();
            pp->set_number("123");
            pp->set_type(Person_PhoneType_MOBILE);
        }
        {
            Person* p = ab.add_person();
            p->set_name("lisi");
            p->set_id(4);
        }
        string e;
        ab.SerializeToString(&e);

        RRMessage res;
        res.m_body = e;
        bool b = qa->Match(&res, false, "");
        EXPECT_EQ(b, true);
        string s = res.DebugString();
        string exp = "a {\n  pb {\n    person {\n      name : zhangsan\n      id : 3\n      email {\n        name : zs\n        net : 163.com\n       }\n      phone {\n        number : 010-11\n        type : WORK\n       }\n      phone {\n        number : 123\n        type : MOBILE\n       }\n     }\n    person {\n      name : lisi\n      id : 4\n     }\n   }\n }\n";
        EXPECT_EQ(s, exp);

        delete req;
    }
    {
        vit = v.next();
        QA* qa = *vit;
        RRMessage* req = new RRMessage;
        bool ret = qa->Query(req);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(req->m_body, "a=100;b=200;url=http://www.baidu.com");
        RRMessage res;
        res.m_body = "http%3A%2F%2Fwww.baidu.com";
        bool b = qa->Match(&res, false, "");
        EXPECT_EQ(b, true);
        EXPECT_EQ(res.DebugString(), "a {\n  url : http://www.baidu.com\n }\n");
        delete req;
    }
    {
        vit = v.next();
        QA* qa = *vit;
        RRMessage* req = new RRMessage;
        bool ret = qa->Query(req);
        EXPECT_EQ(ret, true);
        //{"a":["100","101"],"b":"200","c":[{"c1":"001","c2":"002"},{"c3":"003","c4":"004"}]}
        EXPECT_EQ(req->m_body, "%7B%22a%22%3A%5B100%2C101%5D%2C%22b%22%3A200%2C%22c%22%3A%5B%7B%22c1%22%3A%22001%22%2C%22c2%22%3A%22002%22%7D%2C%7B%22c3%22%3A%22003%22%2C%22c4%22%3A%22004%22%7D%5D%7D");
        RRMessage res;
        res.m_body = "%7B%22a%22%3A%5B%22100%22%2C%22101%22%5D%2C%22b%22%3A%22200%22%2C%22c%22%3A%5B%7B%22c1%22%3A%22001%22%2C%22c2%22%3A%22002%22%7D%2C%7B%22c3%22%3A%22003%22%2C%22c4%22%3A%22004%22%7D%5D%7D";
        bool b = qa->Match(&res, false, "");
        EXPECT_EQ(b, true);
        EXPECT_EQ(res.DebugString(), "a {\n  url {\n    a : 100\n    a : 101\n    b : 200\n    c {\n      c1 : 001\n      c2 : 002\n     }\n    c {\n      c3 : 003\n      c4 : 004\n     }\n   }\n }\n");
        delete req;
    }
    return ;
}
