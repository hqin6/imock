
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
    {
        RRMessage r;
        r.m_body = "4abcd3abc";
        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ(true, res);
    }

    {
        RRMessage r;
        //匹配zhangsan|1
        r.m_body = "zhangsan|1|zs@126.com|zzz_HOME,yyy_MOBILE";
        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        int i = 123;
        char buf[16];
        memcpy(buf, &i, sizeof(int));
        string b(buf, sizeof(int));
        EXPECT_EQ(resp.m_body, b + ",k1=k2;aaa-1");
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        //匹配reg:.*163.COM
        r.m_body = "zhangsan|3|zs@163.com|zzz_HOME,yyy_MOBILE";
        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        int i = 123;
        char buf[16];
        memcpy(buf, &i, sizeof(int));
        string b(buf, sizeof(int));
        EXPECT_EQ(resp.m_body, "k1=k2;aaa-3");
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        //匹配<abd
        r.m_body = "zhangsan|3|zs@126.com|abc_HOME,yyy_MOBILE";
        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        string e1 = "file=aaa;email=zs@fuck.com;a=1;b=";
        string e2 = "a=a1;b=b2|c=c3";
        bool t = false;
        if (e1 == resp.m_body)
        {
            t = true;
        }
        else if (e2 == resp.m_body)
        {
            t = true;
        }
        EXPECT_EQ(t, true);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        //匹配reg:HOM*
        r.m_body = "zhangsan|3|zs@126.com|zzz_HOME,yyy_MOBILE";
        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        string e1 = "file=aaa;email=zs@fuck.com;a=1;b=";
        string e2 = "a=a1;b=b2|c=c3";
        bool t = false;
        if (e1 == resp.m_body)
        {
            t = true;
        }
        else if (e2 == resp.m_body)
        {
            t = true;
        }
        EXPECT_EQ(t, true);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        //匹配yyy
        r.m_body = "zhangsan|3|zs@126.com|zzz_HME,yyy_MOBILE";
        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        string e = "b==b2;email==@3-yyy-bxx";
        EXPECT_EQ(e, resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        //0x03abc4,defgzzz
        string data = "abc";
        string d2 = "defg";
        string tail = "zzz";
        int length = data.size();
        char buf[128];
        char *p = buf;
        memcpy(p, &length, sizeof(int));
        p += sizeof(int);
        strcpy(p, data.c_str());
        p += data.size();
        p += sprintf(p, "%d,", (int)d2.size());
        strcpy(p, d2.c_str());
        p += d2.size();
        strcpy(p, tail.c_str());
        p += tail.size();

        r.m_body = string(buf, p - buf);
        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        if (! res)
        {
            EXPECT_EQ(0, 1);
            return ;
        }

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

        EXPECT_EQ("abcd"+e, resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        //0x03abc4,defgzzz
        string data = "abc";
        string d2 = "defgg";
        string tail = "zzz";
        int length = data.size();
        char buf[128];
        char *p = buf;
        memcpy(p, &length, sizeof(int));
        p += sizeof(int);
        strcpy(p, data.c_str());
        p += data.size();
        p += sprintf(p, "%d,", (int)d2.size());
        strcpy(p, d2.c_str());
        p += d2.size();
        strcpy(p, tail.c_str());
        p += tail.size();

        r.m_body = string(buf, p - buf);
        QA* qa = dat.Match(&r, true, "");
        if (! qa)
        {
            EXPECT_EQ(0, 1);
            return ;
        }
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        if (! res)
        {
            EXPECT_EQ(0, 1);
            return ;
        }

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

        EXPECT_EQ(e, resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        r.m_body = "100,200;100,300;100,400";
        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ("a==a1;b==b1-A-", resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        string b1 = "hello";
        char buf[128];
        char* p = buf;
        p += sprintf(p, "k=key;v=val;a=");
        int len = b1.size();
        memcpy(p, &len, sizeof(int));
        p += sizeof(int);
        p += sprintf(p, ";b=%sb2v;c=cv", b1.c_str());

        r.m_body = string(buf, p - buf);

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        AddressBook ab;
        {
            Person* p = ab.add_person();
            p->set_name("len");
            p->set_id(len);
        }
        string e;
        ab.SerializeToString(&e);

        EXPECT_EQ(e, resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        string s = "b=1;c=2;a=a1:1,a2:2;a=a2:3,a3:4|ddeee";
        r.m_body = s;

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        string e;
        string name = "";
        {
            AddressBook ab;
            {
                Person* p = ab.add_person();
                p->set_name("zhangsan");
                p->set_id(3333);
            }
            ab.SerializeToString(&name);
        }
        {
            AddressBook ab;
            {
                Person* p = ab.add_person();
                p->set_name(name);
                p->set_id(123);
            }
            ab.SerializeToString(&e);
        }

        EXPECT_EQ(e, resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        string s;
        Pair p;
        p.set_key("3;abc12");
        p.set_value(111);
        p.SerializeToString(&s);
        r.m_body = s;

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ("a==aa;email==xx-132-100", resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        string s;
        AddressBook ab;

        Person* p = ab.add_person();
        p->set_name("zhangsan");
        p->set_id(3);
        p->set_email("zs@fuck.com");
        Person::PhoneNumber* pn = p->add_phone();
        pn->set_number("010-111");
        pn->set_type(Person::MOBILE);

        ab.SerializeToString(&s);
        r.m_body = s;

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ("mytypethis is pb", resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        string s = "a=1;b=2;url=p.tanx.com%2Fex%3Fi%3D1%26b%3D2";
        r.m_body = s;

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ("hello", resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;

        string e;
        string name = "";
        {
            AddressBook ab;
            {
                Person* p = ab.add_person();
                p->set_name("zhangsan");
                p->set_id(3333);
                p->set_email("123@163.com");
                Person::PhoneNumber* pn = p->add_phone();
                pn->set_number("010");
                pn->set_type(Person::MOBILE);
                pn = p->add_phone();
                pn->set_number("012");
                pn->set_type(Person::HOME);
            }
            ab.SerializeToString(&name);
        }
        {
            AddressBook ab;
            {
                Person* p = ab.add_person();
                p->set_name(name);
                p->set_id(124);
                p->set_email("222@126.com");
                Person::PhoneNumber* pn = p->add_phone();
                pn->set_number("010");
                pn->set_type(Person::WORK);
            }
            ab.SerializeToString(&e);
        }
        char buf[16];
        int l = e.size();
        memcpy(buf, &l, sizeof(int));
        e = string(buf, sizeof(int)) + e;
        r.m_body = e + "1234,6,,78";

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ("a==a1;b==b1-A-", resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        string s = "a";
        r.m_body = s;

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ("--", resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        string s = "http%3A%2F%2F#http://www.baidu.com";
        r.m_body = s;

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ("http%3A%2F%2F|http%3A%2F%2Fwww.baidu.com", resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;
        string s = "http://xxx.com/ex?js-%7B%22a%22%20%3A%20%22av%22%2C%20%22b%22%20%3A%20%22bv_v1_v2%22%2C%20%22c%22%20%3A%20%5B%7B%22c1%22%20%3A%20%22c1v%22%7D%2C%20%7B%22c2%22%20%3A%20%22c2_v1_v2%22%7D%2C%20%7B%22c3%22%20%3A%20%5B%22c31_v1%22%2C%20%22c32_v2%22%2C%20%22c33_v3%22%5D%7D%5D%7D$k-123";
        r.m_body = s;

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ("{\"a\":[{\"a1\":[11,111],\"a2\":22,\"b\":{\"b1\":[\"1b\",\"1.1b\"],\"b2\":\"b22\"}},{\"a1\":333,\"a2\":22,\"a3\":33,\"b\":{\"b1\":[\"b-1\",\"1.1-b\"],\"b2\":\"b22\"}}]}", resp.m_body);
        EXPECT_EQ(true, res);
    }
    {
        RRMessage r;

        string eq, ea;
        string name = "";
        {
            Person* p = new Person;
            p->set_name("xxnousex");
            p->set_id(1223456);
            p->set_goodman(true);
            p->SerializeToString(&eq);
        }
        {
            Person* p = new Person();
            p->set_name("xxx");
            p->set_id(9876103);
            p->set_goodman(false);
            p->SerializeToString(&ea);
        }
        r.m_body = eq;

        QA* qa = dat.Match(&r, true, "");
        RRMessage resp;
        bool res = qa->Answer(&resp, &r);
        EXPECT_EQ(ea, resp.m_body);
        EXPECT_EQ(true, res);
    }
}
