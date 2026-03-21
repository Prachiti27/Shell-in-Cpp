#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <fcntl.h>

using namespace std;

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

void execute(vector<string> &tokens)
{
    vector<vector<string>> commands;
    vector<string> current;

    for (auto &token : tokens)
    {
        if (token == "|")
        {
            commands.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(token);
        }
    }
    commands.push_back(current);

    int n = commands.size();
    int prev_fd = -1;

    for (int i = 0; i < n; i++)
    {
        int pipe_fd[2];
        if (i < n - 1)
        {
            pipe(pipe_fd);
        }

        pid_t pid = fork();

        if (pid == 0)
        {
            if (prev_fd != -1)
            {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if (i < n - 1)
            {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }
            int in_fd = -1, out_fd = -1;
            vector<string> cmd;

            for(int j=0;j<commands[i].size();j++){
                if(commands[i][j] == "<"){
                    in_fd = open(commands[i][j+1].c_str(), O_RDONLY);
                    j++;
                }
                else if(commands[i][j] == ">"){
                    out_fd = open(commands[i][j+1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    j++;
                }
                else{
                    cmd.push_back(commands[i][j]);
                }
            }

            if(in_fd != -1){
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if(out_fd != -1){
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            vector<char*> args = toCharArray(cmd);
            execvp(args[0], args.data());
            perror("execvp failed");
            exit(1);
        }
        else if(pid>0){
            if(prev_fd != -1) close(prev_fd);
            if(i<n-1){
                close(pipe_fd[1]);
                prev_fd = pipe_fd[0];
            }
        }
        else{
            perror("fork failed");
        }
    }

    for(int i=0;i<n;i++) wait(NULL);
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

        if (handleBuiltins(tokens))
            continue;

        execute(tokens);
    }
}