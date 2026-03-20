#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

using namespace std;

void execute(vector<char *> &args)
{
    pid_t pid = fork();
    if (pid == 0)
    {
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

        vector<char *> args = toCharArray(tokens);
        execute(args);
    }
}