#pragma once

#include <chrono>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace CL {

    typedef const std::vector<std::string>& funcargs;
    typedef void (*funcptr)(funcargs);

    struct ClintOptions {
        std::string prefix = "=> ";
        bool showDuration = false;
    };

    class Clint {
      private:
        typedef void (Clint::*clintfuncptr)(funcargs);

      public:
        Clint() {
            this->SetMode("help", &Clint::Help);
            this->SetMode("ls", &Clint::Help);
            this->SetMode("ll", &Clint::Help);
            this->SetMode("exit", &Clint::Exit);
        }

        void SetMode(const std::string& name, funcptr ptr) {
            modes[name] = ptr;
        }
        void SetMode(const std::vector<std::string>& names, funcptr ptr) {
            for(const auto& name: names) {
                modes[name] = ptr;
            }
        }
        void SetMode(const std::string& name, clintfuncptr ptr) {
            internalmodes[name] = ptr;
        }
        void SetMode(const std::vector<std::string>& names, clintfuncptr ptr) {
            for(const auto& name: names) {
                internalmodes[name] = ptr;
            }
        }
        void SetMode(const std::string& name, Clint* cl) {
            submodes[name] = cl;
        }
        void SetMode(const std::vector<std::string>& names, Clint* cl) {
            for(const auto& name: names) {
                submodes[name] = cl;
            }
        }

        void SetOptions(const ClintOptions& opts) {
            options = opts;
        }
        void Start() {
            this->MainCycle();
        }

        void Exec(const std::vector<std::string>& args) {
            if(args.empty()) {
                return;
            }
            std::string name = args[0];
            std::vector<std::string> subargs(args.begin() + 1, args.end());

            this->Exec(name, subargs);
        }
        void Exec(const std::string& name, const std::vector<std::string>& args) {
            if(internalmodes.find(name) != internalmodes.end()) {
                (this->*(internalmodes[name]))(args);
                return;
            } else if(modes.find(name) != modes.end()) {
                auto startTime = std::chrono::steady_clock::now();
                modes[name](args);
                auto endTime = std::chrono::steady_clock::now();
                auto diffTime = endTime - startTime;
                if(options.showDuration) {
                    std::cout << "\nExecution: "
                              << std::chrono::duration<double, std::milli>(diffTime).count()
                              << "\n";
                }
                return;
            } else if(submodes.find(name) != submodes.end()) {
                submodes[name]->Exec(args);
                return;
            } else {
                std::cout << name << " is not defined\n";
                return;
            }
        }

        Clint& operator<<(const std::string& str) {
            std::cout << str;
            return *this;
        }

      private:
        int MainCycle() {
            std::string line;
            while(!exit) {
                (*this) << options.prefix;

                getline(std::cin, line);
                std::stringstream ss(line);

                std::vector<std::string> args;
                std::string arg;
                while(ss >> arg) {
                    args.push_back(arg);
                }

                this->Exec(args);
            }
            return 0;
        }

        void Help(funcargs) {
            std::vector<std::string> allmodes;
            for(const auto&[name, func]: modes) {
                allmodes.push_back(name);
            }
            for(const auto&[name, func]: submodes) {
                allmodes.push_back(name);
            }
            for(const auto&[name, func]: internalmodes) {
                allmodes.push_back(name);
            }
            std::sort(allmodes.begin(), allmodes.end());

            for(const auto& mode: allmodes) {
                (*this) << mode << "\n";
            }
        }
        void Exit(funcargs) {
            exit = true;
        }

        std::map<std::string, funcptr> modes;
        std::map<std::string, Clint*> submodes;
        std::map<std::string, clintfuncptr> internalmodes;
        ClintOptions options;
        bool exit = false;
    };
}  // namespace CL
