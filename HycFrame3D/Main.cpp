#include <iostream>
#include "PrintLog.h"

int main()
{
    std::cout << "hello world" << std::endl;
    P_LOG(LOG_DEBUG, "hello world\n");

    return 0;
}
