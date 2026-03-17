#include<iostream>
#include<sstream>
#include<string>
#include<vector>

using namespace std;

vector<string> parse(const string& input){
    vector<string> args;
    stringstream ss(input);
    string word;

    while(ss>>word){
        args.push_back(word);
    }
    return args;
}

int main(){
    while(true){
        string input;
        cout<<"my_shell> "<<flush;

        if(!getline(cin, input)) break;
        if(input.empty()) continue;
        if(input=="exit") break;

        vector<string> tokens = parse(input);

        for(const auto& token:tokens){
            cout<<"["<<token<<"]";
        }
        cout<<endl;
    }
}