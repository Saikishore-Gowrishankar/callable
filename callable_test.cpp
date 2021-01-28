#include "callable.h"
#include <iostream>
#include <future>
int main()
{
    std::packaged_task<int(int, int)> task([](int, int){std::cout << "goodbye world!" << std::endl; return 0;});
    auto f = task.get_future();
    auto myFunction = callable_factory<int,int>(std::move(task));
    myFunction(2,3);
}