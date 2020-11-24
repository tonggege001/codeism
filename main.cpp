#include <string>
#include "AsmSim.h"
#include "KeySim.h"

using namespace std;

int main(int argc, char *argv[]){
    string file1(argv[1]), file2(argv[2]);

    // 通过对比汇编代码实现codesim
    double asmsim = SimilarMaxsub(file1, file2);

    //通过对比关键字向量实现codesim，效果不如asm
    // double keysim = KeySim(file1, file2);

    printf("%lf", asmsim);

}
