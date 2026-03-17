#include <iostream>
#include<string>

using namespace std;

int main(){
    while(true){
        string input;
        cout<<"my_shell> "<<flush;

        if(!getline(cin, input)) break;

        if(input.empty()) continue;

        if(input=="exit") break;

        cout<<input<<endl;
    }
}