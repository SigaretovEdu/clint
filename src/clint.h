#pragma once

#include <algorithm>
#include <cmath>
#include <csignal>
#include <iostream>
#include <map>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <vector>

static termios oldSettings;

namespace CL {

    typedef std::vector<std::string> CMD;

    class ModeViewer {
        typedef int (*Function)(CMD&);
        typedef int (ModeViewer::*SubMode)(CMD&);
        typedef std::map<std::string, std::pair<Function, std::string>> ModeTable;
        typedef std::map<std::string, std::pair<ModeViewer, std::string>> SubModeTable;

      public:
        void AddMode(const std::string& name, Function func, const std::string& description = "") {
            symbols[name].first = func;
            symbols[name].second = description;
        }

        void AddMode(const std::string& name, const ModeViewer& viewer,
                     const std::string& description = "") {
            viewers[name].first = viewer;
            viewers[name].second = description;
        }

        std::vector<std::pair<std::string, std::string>> GetNames() const {
            std::vector<std::pair<std::string, std::string>> names;
            for(const auto& item: symbols) {
                names.push_back(std::make_pair(item.first, item.second.second));
            }
            for(const auto& item: viewers) {
                names.push_back(std::make_pair(item.first, item.second.second));
            }
            std::sort(
                names.begin(), names.end(),
                [](const std::pair<std::string, std::string>& l,
                   const std::pair<std::string, std::string>& r) { return l.first < r.first; });
            return names;
        }

        void ShowHelp(bool b) {
            showHelp = b;
        }

        int operator()(CMD& cmd) {
            if(cmd.empty()) {
                std::cout << "Not enough arguments\n";
                return 0;
            }
            const auto modeName = cmd[0];
            cmd.erase(cmd.begin());

            if(modeName == "help" && showHelp) {
                return Help(cmd);
            }
            if(symbols.find(modeName) != symbols.end()) {
                return symbols[modeName].first(cmd);
            }
            if(viewers.find(modeName) != viewers.end()) {
                return viewers[modeName].first(cmd);
            }

            std::cout << modeName << " is not defined\n";
            return 0;
        }

      private:
        void PrintModeFull(const std::string& mode, size_t offset, const std::string& desc) {
            std::cout << mode;
            if(desc.empty()) {
                std::cout << "\n";
                return;
            }
            for(size_t i = 0; i < offset - mode.size(); ++i) {
                std::cout << ' ';
            }
            winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            size_t width = w.ws_col - offset;
            size_t pos = 0;
            while(pos < desc.size()) {
                if(pos) {
                    for(size_t i = 0; i < offset; ++i) {
                        std::cout << ' ';
                    }
                }
                size_t printLen = std::min(desc.size() - pos, width);
                std::cout << desc.substr(pos, printLen) << "\n";
                pos += printLen;
            }
        }

        int Help(std::vector<std::string>& args) {
            auto modes = GetNames();
            if(args.size() == 1 && args[0] == "-v") {
                size_t maxLen = 0;
                for(const auto& mode: modes) {
                    maxLen = std::max(maxLen, mode.first.size());
                }
                maxLen = std::max<size_t>(maxLen, 9);
                maxLen += 4;
                for(const auto& mode: modes) {
                    PrintModeFull(mode.first, maxLen, mode.second);
                }
                std::cout << "help [-v]";
                for(size_t i = 0; i < maxLen - 9; ++i) {
                    std::cout << ' ';
                }
                std::cout << "Show this message\n";
            } else {
                std::cout << "List of options: ";
                for(const auto& mode: modes) {
                    std::cout << mode.first << ", ";
                }
                std::cout << "help [-v]\n";
            }
            return 1;
        }

      private:
        ModeTable symbols;
        SubModeTable viewers;
        bool showHelp = true;
    };

    struct ClintColors {
        std::string RESET = "\033[0m";
        std::string BOLD = "\033[1m";
        std::string DIM = "\033[2m";
        std::string ITALIC = "\033[3m";
        std::string UNDERLINE = "\033[4m";
        std::string BLINK = "\033[5m";
        std::string REVERSE = "\033[7m";
        std::string HIDDEN = "\033[8m";
        std::string BOLD_ITALIC = "\033[3;1m";

        std::string BLACK = "\033[0;30m";
        std::string RED = "\033[0;31m";
        std::string GREEN = "\033[0;32m";
        std::string YELLOW = "\033[0;33m";
        std::string BLUE = "\033[0;34m";
        std::string MAGENTA = "\033[0;35m";
        std::string CYAN = "\033[0;36m";
        std::string WHITE = "\033[0;37m";
        std::string GRAY = "\033[90m";
    };

    struct ClintOptions {
        std::string Prompt = "~> ";
        ClintColors colors;
    };

    class Clint {
      public:
        Clint(const ModeViewer& modeViewer, const ClintOptions& clintOptions):
            viewer(modeViewer), options(clintOptions) {}

        void Run() {
            EnableRawMode();

            while(1) {
                PrintPrompt();
                char c;
                std::string input = ReadInput();
                CMD cmd = ParseInput(input);
                if(!cmd.empty()) {
                    lastResult = viewer(cmd);
                    if(!(!history.empty() && history.back().first == input)) {
                        history.push_back(std::make_pair(input, lastResult));
                    }
                    usingHist = history.size();
                }
            }
        }

      private:
        static void WriteConsole(const std::string& text) {
            write(STDOUT_FILENO, text.data(), text.size());
        }

        static void WriteConsole(char c) {
            write(STDOUT_FILENO, &c, 1);
        }

        class Escapes {
          public:
            static void GoUpNLines(int n) {
                WriteConsole("\033[" + std::to_string(n) + "F");
            }
            static void GoToZeroPosInLine() {
                WriteConsole("\033[0G");
            }
            static void ClearTillEndOfScreen() {
                WriteConsole("\033[0J");
            }
        };

        void PrintPrompt(int code = -1) {
            WriteConsole(options.colors.BOLD);
            int result = (code == -1) ? lastResult : code;
            if(result) {
                WriteConsole(options.colors.GREEN);
            } else {
                WriteConsole(options.colors.RED);
            }
            WriteConsole(options.Prompt);
            WriteConsole(options.colors.RESET);
        }

        size_t GetInputHeight(const std::string& input) {
            winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            size_t len = input.size() + options.Prompt.size();
            return std::ceil(static_cast<float>(len) / w.ws_col);
        }

        void ClearLastLines(int n) {
            if(--n > 0) {
                Escapes::GoUpNLines(n);
            } else {
                Escapes::GoToZeroPosInLine();
            }
            Escapes::ClearTillEndOfScreen();
        }

        void RefreshInput(const std::string& oldInput, const std::string& newInput, int code = -1) {
            ClearLastLines(GetInputHeight(oldInput));
            PrintPrompt(code);
            WriteConsole(newInput);
        }

        std::string ReadInput() {
            std::string input;
            char c;
            while(read(STDIN_FILENO, &c, 1) == 1) {
                if(iscntrl(c)) {  // control key
                    switch(c) {
                        case 10: {
                            WriteConsole('\n');
                            return input;
                        }
                        case 27: {
                            char seq[2];
                            read(STDIN_FILENO, &seq[0], 1);
                            read(STDIN_FILENO, &seq[1], 1);
                            if(seq[0] == '[') {
                                switch(seq[1]) {
                                    case 'A': {
                                        usingHist = std::max(0, usingHist - 1);
                                        RefreshInput(input, history[usingHist].first,
                                                     history[usingHist].second);
                                        break;
                                    }
                                    case 'B': {
                                        usingHist = std::min<size_t>(history.size(), usingHist + 1);
                                        if(usingHist == history.size()) {
                                            RefreshInput(input, "", lastResult);
                                        } else {
                                            RefreshInput(input, history[usingHist].first,
                                                         history[usingHist].second);
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                        case 127: {
                            if(!input.empty()) {
                                ClearLastLines(GetInputHeight(input));
                                input.pop_back();
                                PrintPrompt();
                                WriteConsole(input);
                            }
                        }
                    }
                } else {  // just input
                    input += c;
                    WriteConsole(c);
                }
            }
        }

        CMD ParseInput(const std::string& input) {
            CMD tokens;
            size_t left = 0, right;
            std::string token;
            while((right = input.find(' ', left)) != std::string::npos) {
                token = input.substr(left, right - left);
                tokens.push_back(token);
                left = right + 1;
            }
            if(left < input.size()) {
                tokens.push_back(input.substr(left, input.size() - left));
            }

            return tokens;
        }

        static void EnableRawMode() {
            tcgetattr(0, &oldSettings);
            atexit(DisableRawMode);

            auto raw = oldSettings;
            raw.c_iflag &= ~(IXON);
            raw.c_lflag &= ~(ECHO | ICANON);
            tcsetattr(0, TCSAFLUSH, &raw);
        }

        static void DisableRawMode() {
            tcsetattr(0, TCSAFLUSH, &oldSettings);
        }

      private:
        ClintOptions options;
        ModeViewer viewer;
        std::vector<std::pair<std::string, int>> history;

        int lastResult = 1;
        int usingHist = -1;
    };
}  // namespace CL
