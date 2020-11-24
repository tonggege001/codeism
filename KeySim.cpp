#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <cmath>

using namespace std;

// 收集所有标识符名称
unordered_map<string, int> GetKeyVec(string & filename){
    char buffer[4096];
    unordered_map<string, int> tokens;

    // 将错误流重定向到stdout，因为clang -dump-tokens的输出到了错误流
    string str = "clang -fsyntax-only -Xclang -dump-tokens -fno-color-diagnostics "+ filename + " 2>&1";
    FILE * pipe = popen(str.c_str(), "r");

    while(!feof(pipe)){
        char * res = fgets(buffer, 4096, pipe);
        if (res == NULL){
            break;
        }

        string line(buffer, strlen(buffer));

        // 过滤标识符
        string::size_type findres = line.find(string("identifier"));
        if(findres == 0){
            continue;
        }

        //过滤非本文件的内容
        findres = line.find(filename);
        if(findres == string::npos){
            continue;
        }

        char token_name_buffer[200];
        sscanf(buffer, "%s ", token_name_buffer);
        string token_name = string(token_name_buffer);
        if(tokens.find(token_name) == tokens.end())
            tokens[token_name] = 1;
        else
            tokens[token_name] += 1;
    }
    return tokens;
}


void CompleteVector(unordered_map<string, int> &m1, unordered_map<string, int> &m2){
    for(auto iter = m1.begin(); iter!=m1.end();++iter){
        if(m2.find(iter->first) == m2.end()){
            m2[iter->first] = 0;
        }
    }

    for(auto iter = m2.begin(); iter!=m2.end();++iter){
        if(m1.find(iter->first) == m1.end()){
            m1[iter->first] = 0;
        }
    }
    return;
}


double KeySim(string filename1, string filename2){
    unordered_map<string, int> keyvec1 = GetKeyVec(filename1);
    unordered_map<string, int> keyvec2 = GetKeyVec(filename2);
    CompleteVector(keyvec1, keyvec2);

    double sa=0.0, sb=0.0, mul=0.0;
    for(auto iter = keyvec1.begin(); iter!=keyvec1.end();++iter){
        string idx = iter->first;
        sa += keyvec1[idx] * keyvec1[idx];
        sb += keyvec2[idx] * keyvec2[idx];
        mul += keyvec1[idx] * keyvec2[idx];
    }
    double score = mul / (sqrt(sa)*sqrt(sb));
    return score;
}


