#include "clint.h"

#include <iostream>
#include <chrono>
#include <thread>

void printhi(CL::funcargs args) {
    if(args.size() == 0) {
        std::cout << "no args presented\n";
        return;
    }
    std::cout << "Hi, " << args[0] << "!\n";
}

void funnymatrix(CL::funcargs args) {
    for(size_t i = 0; i < 10; ++i) {
        for(size_t j = 0; j < 10; ++j) {
            std::cout << "#";
        }
        std::cout<<"\n";
    }
}

void longFunction(CL::funcargs args) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5100));
    std::cout<<"I finished!\n";
}

int main() {
    CL::Clint clint;
    clint.SetMode("hi", &printhi);
    clint.SetMode("matrix", &funnymatrix);
    clint.SetMode("long", &longFunction);

    CL::ClintOptions clOptions;
    clOptions.showDuration = false;
    clint.SetOptions(clOptions);

    clint.Start();

    return 0;
}
