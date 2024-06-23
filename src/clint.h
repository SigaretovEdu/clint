#pragma once

#include <chrono>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace CL {

    typedef const std::vector<std::string>& funcargs;
    typedef void (*funcptr)(funcargs);

    struct ClintOptions {
        std::string prefix = "=> ";
        bool showDuration = true;
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
            internalModes[name] = ptr;
        }
        void SetMode(const std::vector<std::string>& names, clintfuncptr ptr) {
            for(const auto& name: names) {
                internalModes[name] = ptr;
            }
        }
        void SetOptions(const ClintOptions& opts) {
            options = opts;
        }
        void Start() {
            this->MainCycle();
        }

        Clint& operator()(const std::string& mode, const std::vector<std::string>& args) {
            modes.at(mode)(args);
            return *this;
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
                std::string name, arg;
                ss >> name;
                while(ss >> arg) {
                    args.push_back(arg);
                }

                if(!this->ParseInternal(name, args)) {
                    if(name.empty() || modes.find(name) == modes.end()) {
                        (*this) << name << " is not defined\n";
                    } else {
                        auto startTime = std::chrono::steady_clock::now();
                        modes.at(name)(args);
                        auto endTime = std::chrono::steady_clock::now();
                        auto diffTime = endTime - startTime;
                        if(options.showDuration) {
                            std::cout << "\nExecution: "
                                      << std::chrono::duration<double, std::milli>(diffTime).count()
                                      << "\n";
                        }
                    }
                }
            }
            return 0;
        }

        int ParseInternal(const std::string& name, funcargs args) {
            if(name.empty() || internalModes.find(name) == internalModes.end()) {
                return 0;
            }
            (this->*(internalModes[name]))(args);
            return 1;
        }

        void Help(funcargs) {
            (*this) << "User modes:     ";
            for(const auto& [name, func]: modes) {
                (*this) << name << " ";
            }
            (*this) << "\n";
            (*this) << "Internal modes: ";
            for(const auto& [name, func]: internalModes) {
                (*this) << name << " ";
            }
            (*this) << "\n";
        }
        void Exit(funcargs) {
            exit = true;
        }

        std::map<std::string, funcptr> modes;
        std::map<std::string, clintfuncptr> internalModes;
        ClintOptions options;
        bool exit = false;
    };
}  // namespace CL
