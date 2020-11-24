#include<iostream>
#include<vector>
#include<string>

using namespace std;

int NodeNumbers = 0;

vector<vector<int> > Vec;

class node{
public:
    node();
public:
    int nodeType;
    double nodeData;

    double GetNode(){
        this->nodeType += 1;
        return nodeData;
    }

private:
    int www;
    string s;
    int SetWWW(){
        this->www += 5;
        return this->www;
    }

};

node::node():nodeType(77){
    cout<<"hello world"<<endl;
    return;
}

int main(){
    node n;
    int a = 5;
    double b = 7;
    int c;
    c = a + b;
    cout<<c<<endl;
    return 0;
}
