#include "threadPool.hpp"
#include <chrono>
#include <iostream>

int globalFunc(int a, int b)
{
    return a + b;
}

class TestClass {
private:
    int a, b;

public:
    TestClass(int a, int b)
        : a(a), b(b) {};

    int operator()()
    {
        return a + b;
    }
};

class ClassTest {
public:
    int add(int a, int b) { return a + b; }
};

int main(int argc, char **argv)
{
    ThreadPool pool(3);
    // print <hello, world>
    pool.submit([] { std::cout << "hello, world\n"; });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // return 1 + 2;
    auto ret = pool.submit(globalFunc, 1, 2);
    std::cout << "globalFunc ret --> " << ret.get() << std::endl;

    // return 2 + 3;
    ret = pool.submit(TestClass(2, 3));
    std::cout << "operator() ret --> " << ret.get() << std::endl;

    ClassTest ct;
    ret = pool.submit(&ClassTest::add, &ct, 5, 6);
    std::cout << "class  ret --> " << ret.get() << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
