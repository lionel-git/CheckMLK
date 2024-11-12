#include <iostream>
#include "checker.h"

#pragma warning(disable: 4514 5045)

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

class MyData : public RootDAta, public checker<MyData> //MODIFICATION: Inherit from checker<MyData>
{
public:
    MyData() = default;

    MyData(int /*v*/)
    {
    }

    MyData(const MyData& data) : checker<MyData>(data)    //MODIFICATION: call copy constructor
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

class MyData2 : public RootDAta, public checker<MyData2>     //MODIFICATION: Inherit from checker<MyData2>
{
public:
    MyData2()
    {
    }

    MyData2(int v) : checker<MyData2>(fmt::format("toto, v={}", v))   //MODIFICATION: call construcot with context
    {
    }

    MyData2(const MyData2& data) : checker<MyData2>(data)      //MODIFICATION: call copy constructor        
    {
        std::cout << "**** Copy constructor of MyData2" << std::endl;
    }

    ~MyData2()
    {
    }
};

void test1()
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

void test2()
{
    MyData d1;
    MyData d2 = d1; // Rem: Call copy constructor !!!
    d2 = d2;        // Call operator=

    d2.testInplace(d1);
}

void sepLine(const std::string& msg)
{
    std::cout << "--" << msg << "----------------- " << std::endl;
}

void callback(long id, const std::string& class_name, const std::string& event)
{
    std::cout << "***** Callback called with id=" << id << " "<< class_name << " Event:" << event << std::endl;
    // Put breakpoint here to check origin for controlled new/delete
}

int main(int /*argc*/, char** /*argv*/) 
{
    try
    {
        checker_common::setThreshold(2);
        checker_common::setOutput("log.txt");

        checker<MyData>::addControlIds({ 5, 7 });
        checker<MyData>::setCallback(callback);

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
    catch(...)
    {
        std::cerr << "Unknown exception" << std::endl;
    }
}
