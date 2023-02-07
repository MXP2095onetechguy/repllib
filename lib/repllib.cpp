#include "./repllib.hpp"

namespace MRE = MXPSQL::REPLLIB;

MRE::Repl::Repl(){}
MRE::Repl::~Repl(){}

bool MRE::Repl::command_add(std::string command, MRE::ReplCMDHandler handler, bool override){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    if (this->cmd_handlers.find(command) != this->cmd_handlers.end()){
        if(!override){
            return false;
        }
    }
    this->cmd_handlers[command] = handler;

    return true;
}

bool MRE::Repl::command_remove(std::string command){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    auto it = this->cmd_handlers.find(command);
    if(it == this->cmd_handlers.end()){
        return false;
    }

    this->cmd_handlers.erase(it);
    return true;
}

MRE::ReplCMDHandler MRE::Repl::command_get(std::string command){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    return this->cmd_handlers[command];
}

std::vector<std::string> MRE::Repl::command_list(){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    std::vector<std::string> cmds;
    for(auto entry : this->cmd_handlers){
        cmds.push_back(entry.first);
    }
    return cmds;
}


void MRE::Repl::prefix_set(std::string prefix){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    this->prompt = prefix;
}

std::string MRE::Repl::prefix_get(){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    return this->prompt;
}


bool MRE::Repl::errhandler_set(MRE::ReplErrHandler errhandler, bool override){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    if(this->error_handler){
        if(!override){
            return false;
        }
    }
    else{
        this->error_handler = errhandler;
    }
    return true;
}

MRE::ReplErrHandler MRE::Repl::errhandler_get(){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    return this->error_handler;
}


bool MRE::Repl::exit_set_command(std::string command, bool override){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    if(this->exit_cmd != ""){
        if(!override){
            return false;
        }
    }
    this->exit_cmd = command;
    return true;
}

bool MRE::Repl::exit_set_command(std::string command, MRE::ReplExitHandler callback, bool override){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    if(this->exit_cmd != ""){
        if(!override){
            return false;
        }
    }
    this->exit_cmd = command;
    this->exit_callback = callback;
    return true;
}

bool MRE::Repl::exit_set_handler(MRE::ReplExitHandler callback, bool override){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    if(this->exit_handler){
        if(!override){
            return false;
        }
    }

    this->exit_handler = callback;
    return true;
}

MRE::ReplExitHandler MRE::Repl::exit_get_handler(){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    return this->exit_handler;
}

std::string MRE::Repl::exit_get_command(){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    return this->exit_cmd;
}

void MRE::Repl::exit_disable(){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    this->exit_cmd = "";
    this->exit_handler = nullptr;
}


void MRE::Repl::start(std::istream& is, std::ostream& os, std::ostream& es){
    if(this->exit_get_command() == ""){
        es << "The exit command was not set. REPLLib will not run to prevent a softlock." << std::endl;
        return;
    }

    while(run){
        os << this->prompt;

        std::string line;
        if(!std::getline(is, line, '\n')){
            es << "A failure was detected when trying to getline." << std::endl;
        }

        {
            std::string cmd;
            std::string args;

            {
                size_t space_pos = line.find(' ', 0);

                if(space_pos == std::string::npos){
                    cmd = line;
                }
                else{
                    cmd = line.substr(0, space_pos);
                    args = line.substr(space_pos);
                }

                if(cmd == this->exit_cmd){
                    if(!this->exit_callback || (this->exit_callback && this->exit_callback(args, is, os, es))){
                        this->stop();
                        return;
                    }
                }

                if(this->cmd_handlers.find(cmd) != this->cmd_handlers.end()){
                    ReplCMDHandler handler = this->cmd_handlers[cmd];
                    if(!handler){
                        es << "Command does not have a handler" << std::endl;
                        continue;
                    }
                    int exit_c = handler(args, is, os, es);
                    if(exit_c != 0){
                        if(this->error_handler && this->error_handler(exit_c, is, os, es)){
                            es << "The error handler has decided to abort the REPL after evaluating the error code." << std::endl;
                            this->stop();
                            return;
                        }
                    }
                }
            }
        }

        os << std::endl;
    }
}

void MRE::Repl::stop(){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    this->run = false;
}

void MRE::Repl::reset(){
    std::unique_lock<std::recursive_mutex> lock(this->semtex);
    this->run = true;
}