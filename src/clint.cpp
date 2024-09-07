#include "clint.h"

#include <iostream>

int Say1(std::vector<std::string>& args) {
    std::cout << "1!\n";
    return 1;
}
int Say2(std::vector<std::string>& args) {
    std::cout << "2!\n";
    return 1;
}
int Say3(std::vector<std::string>& args) {
    std::cout << "3!\n";
    return 1;
}
int Say4(std::vector<std::string>& args) {
    std::cout << "4!\n";
    return 1;
}

int main(int argc, char** argv) {
    CL::ModeViewer viewer1, viewer2;
    viewer1.AddMode("say1", &Say1);
    viewer1.AddMode("say2", &Say2);
    viewer2.AddMode("say3", &Say3);
    viewer2.AddMode("say4", &Say4);
    viewer1.AddMode("say34", viewer2);

    CL::Clint clint(viewer1, CL::ClintOptions());
    clint.Run();
    return 0;
}
