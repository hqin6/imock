#include "gtest/gtest.h"
#include "ProcTitle.h"
#include <string>
#include <iostream>
using namespace std;

extern char** g_argv;

TEST(ProcTitle, Set)
{
    ProcTitle::SaveEnv(g_argv);
    string s(1300, 'a');
    ProcTitle::Set(s.c_str());
    int len = strlen(g_argv[0]);
    int minlen = 1300 < len ? 1300 : len;
    EXPECT_EQ(strncmp(s.c_str(), g_argv[0], minlen), 0);
    EXPECT_GT(minlen , 100);
    return ;
}
