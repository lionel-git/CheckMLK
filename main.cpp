
#include "checker.h"
#include <iostream>

#pragma warning(disable: 4514 5045 5039 4668 4710 4711)

class RootDAta
{
public:
    RootDAta()
    {
    }

    ~RootDAta()
    {
    }
};

class MyData : public RootDAta, public mlk::checker<MyData> //MODIFICATION: Inherit from checker<MyData>
{
public:
    MyData() = default;

    MyData(int /*v*/)
    {
    }

    MyData(const MyData& data) : mlk::checker<MyData>(data)    //MODIFICATION: call copy constructor
    {
        std::cout << "**** Copy constructor of MyData" << std::endl;
    }

    MyData& operator=(const MyData& /*data*/)
    {
        std::cout << "**** Operator=" << std::endl;
        return *this;
    }

    void testInplace(MyData& data)
    {
        new (this) MyData(data);
    }

    // ~MyData() = default;
};

class MyData2 : public RootDAta, public mlk::checker<MyData2>     //MODIFICATION: Inherit from checker<MyData2>
{
public:
    MyData2()
    {
    }

    MyData2(int v) : mlk::checker<MyData2>(fmt::format("toto, v={}", v))   //MODIFICATION: call construcot with context
    {
    }

    MyData2(const MyData2& data) : mlk::checker<MyData2>(data)      //MODIFICATION: call copy constructor        
    {
        std::cout << "**** Copy constructor of MyData2" << std::endl;
    }

    ~MyData2()
    {
    }
};

static void test1()
{
    MyData2 data_spec2;
    MyData2 data_spec17(17);

    MyData* tabPData[10];
    for (int i = 0; i < 10; ++i)
    {
        tabPData[i] = new MyData();
        delete tabPData[i];
        tabPData[i] = new MyData();
    }
    MyData d_temp;

    auto data2 = *tabPData[7];

    for (int i = 0; i < 10; ++i)
    {
        delete tabPData[i];
    }
}

static void test2()
{
    MyData d1;
    MyData d2 = d1; // Rem: Call copy constructor !!!
    d2 = d2;        // Call operator=

    d2.testInplace(d1);
}

static void sepLine(const std::string& msg)
{
    std::cout << "--" << msg << "----------------- " << std::endl;
}

static void callback(long id, const std::string& class_name, const std::string& event)
{
    std::cout << "***** Callback called with id=" << id << " " << class_name << " Event:" << event << std::endl;
    // Put breakpoint here to check origin for controlled new/delete
}

static size_t initChecker()
{
    auto initOutput = mlk::checker_common::setOutput("log.txt");
    auto initThreshold = mlk::checker_common::setThreshold(2);
    size_t initControlsId = 0;
    initControlsId = mlk::checker<MyData>::addControlIds({ 5, 7 });
    auto initCallBack = mlk::checker<MyData>::setCallback(callback);
    return initOutput ^ initThreshold ^ initControlsId ^ initCallBack;
}

//auto initGlobal = initChecker();

int main(int /*argc*/, char** /*argv*/)
{
    try
    {
        initChecker();

        sepLine("test1");
        test1();
        sepLine("test2");
        test2();
        sepLine("End");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
    }
}
