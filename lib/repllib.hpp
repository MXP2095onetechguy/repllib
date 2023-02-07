#ifndef MXPSQL_REPLLIB_HPP
#define MXPSQL_REPLLIB_HPP

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <iostream>
#include <functional>

namespace MXPSQL{
    namespace REPLLIB{
        using ReplCMDHandler = std::function<int(std::string args, std::istream& is, std::ostream& os, std::ostream& es)>;
        using ReplErrHandler = std::function<bool(int code, std::istream& is, std::ostream& os, std::ostream& es)>;
        using ReplExitHandler = std::function<bool(std::string args, std::istream &is, std::ostream &os, std::ostream &es)>;

        class Repl
        {
        private:
            std::recursive_mutex semtex;
            std::map<std::string, ReplCMDHandler> cmd_handlers;
            std::string prompt = "";
            ReplErrHandler error_handler;

            std::string exit_cmd = "";
            ReplExitHandler exit_handler;

            bool run = true;
            ReplExitHandler exit_callback;

            public:
            Repl();
            ~Repl();

            bool command_add(std::string command, ReplCMDHandler cmd_handler, bool override);
            bool command_remove(std::string command);
            ReplCMDHandler command_get(std::string command);
            std::vector<std::string> command_list();

            void prefix_set(std::string prefix);
            std::string prefix_get();

            bool errhandler_set(ReplErrHandler errhandler, bool override);
            ReplErrHandler errhandler_get();

            bool exit_set_command(std::string command, bool override);
            bool exit_set_command(std::string command, ReplExitHandler callback, bool override);
            bool exit_set_handler(ReplExitHandler callback, bool override);
            ReplExitHandler exit_get_handler();
            std::string exit_get_command();
            void exit_disable();

            void start(std::istream& is, std::ostream& os, std::ostream& es);
            void stop();
            void reset();
        };
    };
};

#endif