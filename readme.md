Just derive suspicious class from mlk::checker<>

Example:
```cpp
class MyData2 : public RootData, public mlk::checker<MyData2>
```

Cf main.cpp for various examples.


