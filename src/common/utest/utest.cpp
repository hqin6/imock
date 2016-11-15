#include "gtest/gtest.h"
#include <iostream>

char ** g_argv = NULL;

int main(int argc, char** argv)
{
    g_argv = argv;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
