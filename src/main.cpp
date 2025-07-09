#include "../headers/vector.hpp"

int main()
{
    Vector vec = {};
    vectorCtor(&vec);
    
    for (int i = 1; i < 8; i++)
        vectorPush(&vec, (VectorElem_t)(uintptr_t)i);

    printf("%p\n", vectorGet(&vec, 3));

    vectorDump(vec);
    vectorDtor(&vec);
    return 0;
}