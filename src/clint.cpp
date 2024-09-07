#include "clint.h"

#include <iostream>
#include <chrono>
#include <thread>

void f1(CL::funcargs funcargs) {
    std::cout << "f1\n";
}
void f2(CL::funcargs funcargs) {
    std::cout << "f2\n";
}
void f3(CL::funcargs funcargs) {
    std::cout << "f3\n";
}
void b1(CL::funcargs funcargs) {
    std::cout << "b1\n";
}
void b2(CL::funcargs funcargs) {
    std::cout << "b2\n";
}
void b3(CL::funcargs funcargs) {
    std::cout << "b3\n";
}

int main() {
    CL::Clint clintSub;
    clintSub.SetMode("f1", &f1);
    clintSub.SetMode("f2", &f2);
    clintSub.SetMode("f3", &f3);

    CL::Clint clintMain;
    clintMain.SetMode("b1", &b1);
    clintMain.SetMode("b2", &b2);
    clintMain.SetMode("b3", &b3);
    clintMain.SetMode("clint", &clintSub);

    clintMain.Start();

    return 0;
}
