#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <fcntl.h>

using namespace std;

void execute(vector<string> &tokens)
{
    int in_fd = -1, out_fd = -1;
    vector<string> cmd;

    for(int i=0;i<tokens.size();i++) {
        if(tokens[i] == ">"){
            if(i+1 >= tokens.size()){
                cerr << "Error: No file specified for output\n";
                return;
            }

            out_fd = open(tokens[i+1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            
            if(out_fd < 0){
                perror("open failed");
                return;
            }
            i++;
        }
        else if(tokens[i] == "<"){
            if(i+1 >= tokens.size()){
                cerr << "Error: No file specified for input\n";
                return;
            }

            in_fd = open(tokens[i+1].c_str(), O_RDONLY);

            if(in_fd < 0){
                perror("open failed");
                return;
            }
            i++;
        }
        else{
            cmd.push_back(tokens[i]);
        }
    }

    if(cmd.empty()) return;

    vector<char*> args;
    for(auto& s : cmd){
        args.push_back(const_cast<char*>(s.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0)
    {
        if(in_fd != -1){
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if(out_fd != -1){
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        execvp(args[0], args.data());
        perror("execvp failed");
        exit(1);
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
    }
    else
    {
        perror("fork failed");
    }
}

vector<char *> toCharArray(vector<string> &args)
{
    vector<char *> res;
    for (auto &s : args)
    {
        res.push_back(const_cast<char *>(s.c_str()));
    }
    res.push_back(nullptr);
    return res;
}

vector<string> parse(const string &input)
{
    vector<string> args;
    stringstream ss(input);
    string word;

    while (ss >> word)
    {
        args.push_back(word);
    }
    return args;
}

bool handleBuiltins(vector<string> &tokens)
{
    if (tokens[0] == "exit")
    {
        exit(0);
    }

    if (tokens[0] == "cd")
    {
        if (tokens.size() < 2)
        {
            const char *home = getenv("HOME");
            if (home == nullptr)
            {
                cerr << "cd: Home not set\n";
            }
            else
            {
                if (chdir(home) != 0)
                {
                    perror("cd failed");
                }
            }
        }
        else
        {
            if (chdir(tokens[1].c_str()) != 0)
            {
                perror("cd failed");
            }
        }
        return true;
    }
    return false;
}

int main()
{
    while (true)
    {
        string input;
        cout << "my_shell> " << flush;

        if (!getline(cin, input))
            break;
        if (input.empty())
            continue;

        vector<string> tokens = parse(input);
        if (tokens.empty())
            continue;

        if(handleBuiltins(tokens)) continue;

        execute(tokens);
    }
}