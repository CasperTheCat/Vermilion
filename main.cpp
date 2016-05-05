#include <iostream>
#include "core/types/types.h"
#include <typeinfo>

using namespace std;
using namespace Vermilion;

int main() {
    float2 hello(2.f,2.f);
    float2 rekt(2.f,2.f);
    float2 x;
    x = hello;
    x += rekt;
    cout << typeid("HELLO").name() << endl;
    cout << x.x << endl;
    return 0;
}

/*
        core/engines/meshEngine.cpp
        core/engines/meshEngine.h
        core/engines/loggingEngine.cpp
        core/engines/loggingEngine.h
 */