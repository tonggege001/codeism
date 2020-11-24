#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <fcntl.h>
#include "KeySim.h"

using namespace std;

// 收集所有标识符名称
void GetFunctionName(string &filename, unordered_set<string>& function_name){
    char buffer[4096];
    // 将错误流重定向到stdout，因为clang -dump-tokens的输出到了错误流
    string str = "clang -fsyntax-only -Xclang -dump-tokens -fno-color-diagnostics "+ filename + " 2>&1";
    FILE * pipe = popen(str.c_str(), "r");
    if(!pipe){
        return;
    }

    while(!feof(pipe)){
        fgets(buffer, 4096, pipe);

        string line(buffer, strlen(buffer));
        // 以下代码有bug
        string::size_type findres = line.find(string("identifier"));

        if(findres != 0){
            continue;
        }

        findres = line.find(filename);
        if(findres == string::npos){
            continue;
        }

        char func_name_buffer[200];
        sscanf(buffer, "identifier '%[^']s'", func_name_buffer);
        string func_name = string(func_name_buffer);
        if(func_name != "main")
            function_name.insert(to_string(func_name.length())+func_name);
        else
            function_name.insert(func_name);
    }
}

// 产生一个临时文件
string CreateTempFile(string & assembleFileName) {
    char tmpName[256];
    tmpnam(tmpName);
    return tmpName;
}

//判断汇编标签名是否含有函数名称
bool FuncInAsmName(string & asmName, unordered_set<string>& function_name){
    for(auto & str : function_name){
        if (asmName.find(str) != string::npos){
            return true;
        }
    }
    return false;
}

//生成汇编代码
void GenerateAssembly(string& filename, string& assembleFileName,
                      unordered_set<string> &function_name, unordered_map<string, string>& functions){
    char buffer[4096];
    assembleFileName = CreateTempFile(assembleFileName);
    string command = "g++ -S -o "+ assembleFileName +" "+ filename;
    system(command.c_str());

    FILE * f = fopen(assembleFileName.c_str(), "r");
    if (f == NULL){
        fprintf(stderr, "GenerateAssembly assembleFile open error\n");
        return;
    }

    // 以下获得所有汇编文件中的所有行，编写一个小的自动机解析每个函数
    //state = 0, 寻找第一个函数， 1：接受函数内容，
    int state = 0;
    string func_key;
    string func_val;
    while(!feof(f)){
        fgets(buffer, 4096, f);
        string buffStr(buffer);

        if(state == 0){
            if (buffStr[0] == '\t'){
                continue;
            }
            if (buffStr[0] == '.'){
                continue;
            }
            string::size_type retPos = buffStr.find(':');
            if(retPos == string::npos){
                continue;
            }

            string funAsmName = buffStr.substr(0, retPos);
            if (FuncInAsmName(funAsmName, function_name)){
                state = 1;
                func_key = funAsmName;
            }
        }
        else if(state == 1){
            if(buffStr.find(".cfi_endproc") != string::npos){
                functions[func_key] = func_val;
                func_val = "";
                func_key = "";
                state = 0;
                continue;
            }
            if (buffStr[0] == '\t'){
                func_val += buffStr;
                continue;
            }
            if(buffStr[0] == '.'){              // 不搜集标签
                continue;
            }
        }
    }
    fclose(f);
    remove(assembleFileName.c_str());
}

// 获得文件的汇编函数
unordered_map<string, string> GetAllFunctions(string filename){
    unordered_set<string> srcFunctionName;                //函数名称
    string srcAssembleFileName;                          // 生成的汇编文件
    unordered_map<string, string> srcFunctions;            // 保存所有汇编函数

    GetFunctionName(filename, srcFunctionName);
    GenerateAssembly(filename,srcAssembleFileName,srcFunctionName, srcFunctions);
    return srcFunctions;
}

vector<string> SplitIntoLine(string & s){
    vector<string> a;
    string parsed;
    stringstream  iss(s);
    while(getline(iss, parsed, '\n')){
        if(parsed.empty() || parsed == "\n")
            continue;
        a.push_back(parsed);
    }
    return a;
}


// 获得两个字符串的汇编最长公共子序列
int MaxSubSequence(string & fun1, string & fun2){
    vector<string> a = SplitIntoLine(fun1);
    vector<string> b = SplitIntoLine(fun2);
    if(a.size() + 5 > 1000){
        fprintf(stderr, "the first function asm code is too long, data=%s", fun1.c_str());
        exit(-1);
    }

    if(b.size()+5 > 1000){
        fprintf(stderr, "the second function asm code is too long, data=%s", fun2.c_str());
        exit(-1);
    }

    int dp[a.size()+5][b.size()+5];
    fill(dp[0], dp[0]+(a.size()+5)*(b.size()+5), 0);

    for(int i = 1;i<=a.size()+1;i++){
        for(int j = 1;j<=b.size();j++){
            if(a[i-1] == b[j-1]){
                dp[i][j] = dp[i-1][j-1] + 1;
            }
            else{
                dp[i][j] = max(dp[i-1][j], dp[i][j-1]);
            }
        }
    }
    return dp[a.size()][b.size()];
}

//获得两个文件的公共子序列相似度
double SimilarMaxsub(string & filename1, string& filename2){

    unordered_map<string, string> srcFunctions = GetAllFunctions(filename1);
    unordered_map<string, string> dstFunctions = GetAllFunctions(filename2);
    vector<string> src,dst;

    for(auto iter=srcFunctions.begin(); iter!=srcFunctions.end();++iter)
        src.emplace_back(iter->second);
    for(auto iter=dstFunctions.begin(); iter!=dstFunctions.end();++iter)
        dst.emplace_back(iter->second);

    int leftSim = 0, rightSim = 0;
    for(int i = 0;i<src.size();i++){
        int max_idx = -1;
        int max_val = -1;
        for(int j = 0;j<dst.size();j++){
            int val = MaxSubSequence(src[i], dst[j]);
            if(max_val < val){
                max_val = val;
                max_idx = j;
            }
        }
        if(max_val != -1){
            leftSim += max_val;
            dst.erase(dst.begin() + max_idx);
        }
    }

    src.clear(); dst.clear();
    for(auto iter=srcFunctions.begin(); iter!=srcFunctions.end();++iter)
        src.emplace_back(iter->second);
    for(auto iter=dstFunctions.begin(); iter!=dstFunctions.end();++iter)
        dst.emplace_back(iter->second);

    for(int i = 0;i<dst.size();i++){
        int max_idx = -1;
        int max_val = -1;
        for(int j = 0;j<src.size();j++){
            int val = MaxSubSequence(dst[i], src[j]);
            if(max_val < val){
                max_val = val;
                max_idx = j;
            }
        }
        if(max_val != -1){
            rightSim += max_val;
            src.erase(src.begin() + max_idx);
        }
    }

    int totalSim = leftSim + rightSim;
    int totalLine = 0;
    for(auto iter=srcFunctions.begin(); iter!=srcFunctions.end();++iter)
        totalLine += SplitIntoLine(iter->second).size();
    for(auto iter=dstFunctions.begin(); iter!=dstFunctions.end();++iter)
        totalLine += SplitIntoLine(iter->second).size();

    return totalSim*1.0/totalLine*1.0;
}










