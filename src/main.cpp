#include "../headers/vector.hpp"

int main(void)
{
    Vector vec = {};
    vectorCtor(&vec);
    
    /*for (int i = 0; i < 16; i++)
        stackPush(&stk, i + 1);

    for (int j = 0; j < 9; j++)
        stackPop(&stk);*/

    //stackDump(stk);
    vectorDtor(&vec);
    return 0;
}