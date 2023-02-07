#include "repllib.hpp"

#include <cstdlib>

int main(int argc, char** argv){
    MXPSQL::REPLLIB::Repl repl;

    repl.command_add("sys", [](std::string args, std::istream& is, std::ostream& os, std::ostream& es) -> int {
        if(system(NULL) == 0) {
            es << "Command preprocessor does not exist!" << std::endl;
            return -5;
        }

        return system(args.c_str());
    }, true);

    repl.command_add("echo", [](std::string args, std::istream& is, std::ostream& os, std::ostream& es) -> int {
        os << args << std::endl;

        return 0;
    }, true);

    repl.errhandler_set([](int code, std::istream& is, std::ostream& os, std::ostream& es) -> bool {
        es << "There was a problem with the command, it returned " << code << " instead of a 0." << std::endl;
        return false;
    }, true);

    repl.prefix_set("SOSH@ ");

    repl.exit_set_command("exit", true);

    repl.start(std::cin, std::cout, std::cerr);

    return 0;
}