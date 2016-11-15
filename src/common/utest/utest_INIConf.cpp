#include "gtest/gtest.h"
#include "INIConf.h"
#include "Log.h"
#include <string>
#include <iostream>
using namespace std;

Logger* g_log = NULL;

TEST(INIConf, Load)
{
    g_iniConf = new INIConf;
    EXPECT_EQ(g_iniConf->Load("utest_ini.conf"), true);
    int i = 0;
    EXPECT_EQ(g_iniConf->Get("utest", "key_int", ISet::Int, &i), true);
    EXPECT_EQ(i, 1);
    string s = "";
    EXPECT_EQ(g_iniConf->Get("utest", "key_str", ISet::Str, &s), true);
    EXPECT_EQ(s, "hello world");
    bool flag = true;
    EXPECT_EQ(g_iniConf->Get("utest", "key_flag", ISet::Flag, &flag), true);
    EXPECT_EQ(flag, false);
    int size = 100;
    EXPECT_EQ(g_iniConf->Get("utest", "key_size", ISet::Size, &size), true);
    EXPECT_EQ(size, 100*1024*1024);

    bool f = false;
    Command c[] = {
        { "utest2", "key_flag",  ISet::Flag,  (uint64_t)&f, NULL},
        { NULL,     NULL,        NULL,        0           , NULL}
    };
    EXPECT_EQ(g_iniConf->Get(c, 0), true);
    EXPECT_EQ(f, true);
    delete g_iniConf;
    g_iniConf = NULL;
}
