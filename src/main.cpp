#include "../headers/vector.hpp"

int main(void)
{
    Stack_t stk = {};
    stackCtor(&stk);
    
    for (int i = 0; i < 16; i++)
        stackPush(&stk, i + 1);

    for (int j = 0; j < 9; j++)
        stackPop(&stk);

    stackDump(stk);
    stackDtor(&stk);
    return 0;
}