#include <iostream>
#include "checker.h"


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

class MyData : public RootDAta, public checker<MyData>
{
public:
    MyData()
    {
    }

    MyData(int v)
    {
    }

    MyData(const MyData& data) : checker<MyData>(data)    //MODIFICATION: Inherit from checker<MyData>
    {
        std::cout << "Copy constructor of MyData" << std::endl;
    }

    ~MyData()
    {
    }
};

class MyData2 : public RootDAta, public checker<MyData2>               //MODIFICATION: Inherit from checker<MyData2>
{
public:
    MyData2()
    {
    }

    MyData2(int v) : checker<MyData2>(std::format("toto, v={}", v))   //MODIFICATION: call construcot with context
    {
    }

    MyData2(const MyData2& data) : checker<MyData2>(data)              //MODIFICATION: call copy constructor
    {
        std::cout << "Copy constructor of MyData" << std::endl;
    }

    ~MyData2()
    {
    }
};








int main(int artgc, char** argv) 
{
    try
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
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unknown exception" << std::endl;
    }
}