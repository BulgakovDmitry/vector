#include "../headers/vector.hpp"
#include <myLib.hpp>
#include <math.h>
#include <inttypes.h>

static void vectorDataDump(Vector vec);

#ifdef VECTOR_CANARY_PROTECTION
static void installDataCanaries (Vector* vec);
static void removeDataCanaries  (Vector* vec);
static void installVectorCanaries(Vector* vec);
static void removeVectorCanaries (Vector* vec);
#endif

#ifdef VECTOR_HASH_PROTECTION
static uint64_t vectorDataHashCalc  (const char* start, const char* end);
static uint64_t vectorStructHashCalc(const Vector* vec); 
#endif

#define VERIFICATION(...)                                                  \
do                                                                         \
{                                                                          \
    if (verifyError != OK)                                                 \
    {                                                                      \
        V_DBG(fprintf(stderr, RED "Error: verifyError != OK\n" RESET);)    \
        vectorDump(*vec);                                                  \
        vectorErrorDump(*vec);                                             \
        __VA_ARGS__                                                        \
    }                                                                      \
} while (0)          

#ifdef VECTOR_CANARY_PROTECTION

static void installDataCanaries(Vector* vec)
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)

    vec->data[0]                 = L_DATA_KANAR;   // INSTALLING A NEW LEFT  CANARY ON DATA
    vec->data[vec->capacity - 1] = R_DATA_KANAR;   // INSTALLING A NEW RIGHT CANARY ON DATA
}

static void removeDataCanaries(Vector* vec)
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)

    vec->data[0]                 = POISON;         // REMOVING THE OLD LEFT  CANARY (CHANGE TO POISON)
    vec->data[vec->capacity - 1] = POISON;         // REMOVING THE OLD RIGHT CANARY (CHANGE TO POISON)
}
static void installVectorCanaries(Vector* vec)
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)

    vec->leftVectorCanary  = L_STACK_KANAR;         // INSTALLING A NEW LEFT  CANARY ON VECTOR
    vec->rightVectorCanary = R_STACK_KANAR;         // INSTALLING A NEW RIGHT CANARY ON VECTOR
}

static void removeVectorCanaries(Vector* vec)
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)

    vec->leftVectorCanary  = 0;                     // REMOVING THE OLD LEFT  CANARY (CHANGE TO 0)
    vec->rightVectorCanary = 0;                     // REMOVING THE OLD RIGHT CANARY (CHANGE TO 0)
}
#endif

#ifdef VECTOR_HASH_PROTECTION
static uint64_t vectorDataHashCalc(const char* start, const char* end)
{
    V_DBG(ASSERT(start, "start = nulptr", stderr);)
    V_DBG(ASSERT(end,   "end = nulptr", stderr);)
    V_DBG(bool check = end > start; ASSERT(check, "end > start", stderr);)

    uint64_t hashSum = 5381;
    char* current = const_cast<char*>(start);
    while (current < end)
    {
        hashSum += hashSum * HASH_COEFF + (unsigned char)(*current);
        current++;
    }
    return hashSum;
}

static uint64_t vectorStructHashCalc(const Vector* vec) 
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)

    Vector tmp        = *vec;          // local copy
    tmp.dataHashSum   = 0;             // to keep it out of the hash
    tmp.vectorHashSum = 0;

    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&tmp);
    const size_t         size  = sizeof(tmp);

    uint64_t hash = 5381u;                     
    for (size_t i = 0; i < size; ++i)
        hash = (hash * HASH_COEFF) + bytes[i];

    return hash;
}
#endif

void vectorCtor(Vector* vec)
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)
    
    memset(vec, 0, sizeof(*vec)); // Zeroize the structure to prevent garbage from getting into the hash

    V_CAN_PR(installVectorCanaries(vec);)
    
    vec->coefCapacity = 2;
    vec->size = 0;
    vec->capacity = START_SIZE;

    vec->data = (VectorElem_t*)calloc(vec->capacity, sizeof(VectorElem_t));
    if (!vec->data)
    {
        vec->errorStatus = ALLOC_ERROR;
        return;
    }

    V_CAN_PR(installDataCanaries(vec);)

    for (size_t i = 1; i < vec->capacity - 1; i++) 
        vec->data[i] = POISON;

    #ifdef VECTOR_HASH_PROTECTION
    vec->dataHashSum   = vectorDataHashCalc(reinterpret_cast<const char*>(vec->data), 
                                            reinterpret_cast<const char*>(vec->data + vec->capacity));
    vec->vectorHashSum = vectorStructHashCalc(vec);
    #endif

    VectorError verifyError = (VectorError)vectorVerify(vec);
    if (verifyError != OK)
    {
        vec->errorStatus |= INIT_HASH_ERROR;
        vectorDump(*vec);
        vectorErrorDump(*vec);
    }
}

void vectorDtor(Vector* vec)
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)

    V_CAN_PR(removeDataCanaries(vec);)
    
    FREE(vec->data);
    
    V_CAN_PR(removeVectorCanaries(vec);)

    V_HASH_PR(vec->dataHashSum = 0; vec->vectorHashSum = 0;)
    memset(vec, 0, sizeof(*vec));
}

VectorError vectorPush(Vector* vec, VectorElem_t value)
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)

    if (!vec)
        return POINTER_ERROR;

    VectorError verifyError = (VectorError)vectorVerify(vec);
    VERIFICATION(return verifyError;);

    if (vec->size >= vec->capacity - 2) // CHECKING FOR IMPLEMENTATION
    {    
        V_CAN_PR(removeDataCanaries(vec);)

        size_t newCapacity = vec->capacity * vec->coefCapacity;
        void* newData = (VectorElem_t*)realloc(vec->data, newCapacity * sizeof(VectorElem_t));
        if (!newData)
        {
            V_CAN_PR(installDataCanaries(vec);)
            
            vec->errorStatus |= ALLOC_ERROR;

            #ifdef VECTOR_HASH_PROTECTION
                vec->dataHashSum  = vectorDataHashCalc(reinterpret_cast<char*>(vec->data), 
                                                       reinterpret_cast<char*>(vec->data + vec->capacity));
                vec->vectorHashSum = vectorStructHashCalc(vec);
            #endif

            return ALLOC_ERROR;
        }

        vec->data = (VectorElem_t*)newData;
        vec->capacity = newCapacity;
        
        for (size_t i = vec->size + 1; i < vec->capacity - 1; i++) // Initialize new memory
            vec->data[i] = POISON;

        V_CAN_PR(installDataCanaries(vec);)
    }    
   
    vec->size++;
    vec->data[vec->size] = value;

    #ifdef VECTOR_HASH_PROTECTION
    vec->dataHashSum  = vectorDataHashCalc(reinterpret_cast<const char*>(vec->data), 
                                           reinterpret_cast<const char*>(vec->data + vec->capacity));
    vec->vectorHashSum = vectorStructHashCalc(vec);
    #endif

    verifyError = (VectorError)vectorVerify(vec); // final check
    VERIFICATION(vec->errorStatus = verifyError; return verifyError;);

    return OK;
}

VectorElem_t vectorPop(Vector* vec)
{
    if (!vec) 
    {
        V_DBG(fprintf(stderr, RED "Error: nullptr passed to vectorPop\n" RESET);)
        return POISON;
    }

    VectorError verifyError = (VectorError)vectorVerify(vec);
    VERIFICATION(return POISON;);
   
    if (vec->size == 0) // Checking if stack is empty
    {
        V_DBG(fprintf(stderr, RED "Error: stack is empty\n" RESET);)
        vec->errorStatus |= EMPTY_VECTOR;
        vectorErrorDump(*vec);
        return POISON;
    }
    
    VectorElem_t temp = vec->data[vec->size];   
    vec->data[vec->size] = POISON;   
    vec->size--;

    if (vec->size < vec->capacity / (REDUCER_CAPACITY * vec->coefCapacity) && vec->capacity > START_SIZE)
    {
        V_CAN_PR(removeDataCanaries(vec);)
        
        size_t newCapacity = vec->capacity / vec->coefCapacity;

        void* newData = (VectorElem_t*)realloc(vec->data, newCapacity * sizeof(VectorElem_t));
        if (!newData)
        {
            vec->capacity *= vec->coefCapacity; // save the old capacity (since realloc failed)
            
            vec->errorStatus |= ALLOC_ERROR;
            
            V_CAN_PR(installDataCanaries(vec);)

            #ifdef VECTOR_HASH_PROTECTION
                vec->dataHashSum   = vectorDataHashCalc(reinterpret_cast<char*>(vec->data), \
                                                        reinterpret_cast<char*>(vec->data + vec->capacity));
                vec->vectorHashSum = vectorStructHashCalc(vec);
            #endif

            return temp;
        }
        else
        {
            vec->data = (VectorElem_t*)newData;
            vec->capacity = newCapacity;
            
            V_CAN_PR(installDataCanaries(vec);)
        }
    }

    #ifdef VECTOR_HASH_PROTECTION
    vec->dataHashSum  = vectorDataHashCalc(reinterpret_cast<const char*>(vec->data), 
                                           reinterpret_cast<const char*>(vec->data + vec->capacity));
    vec->vectorHashSum = vectorStructHashCalc(vec);
    #endif

    verifyError = (VectorError)vectorVerify(vec);
    VERIFICATION();

    return temp;
}

VectorElem_t vectorGet(const Vector* vec, const size_t index)
{
    if (!vec) 
    {
        V_DBG(fprintf(stderr, RED "Error: nullptr passed to vectorGet\n" RESET);)
        return POISON;
    }

    VectorError verifyError = (VectorError)vectorVerify(const_cast<Vector*>(vec));
    VERIFICATION(return POISON;);

    if (vec->size == 0) 
    {
        V_DBG(printf(RED "STACK IS EMPTY\n" RESET);)
        (const_cast<Vector*>(vec))->errorStatus |= EMPTY_VECTOR;
        return POISON;
    }

    if (index >= vec->size)
    {
        V_DBG(fprintf(stderr, RED "INDEX %zu OUT OF BOUNDS (size = %zu)\n" RESET,
                      index, vec->size);)
        (const_cast<Vector*>(vec))->errorStatus |= INDEX_OUT_OF_RANGE;
        return POISON;
    }

    #ifdef VECTOR_HASH_PROTECTION
    uint64_t currentDataHash   = vectorDataHashCalc(reinterpret_cast<const char*>(vec->data), 
                                                    reinterpret_cast<const char*>(vec->data + vec->capacity));
    uint64_t currentVectorHash = vectorStructHashCalc(vec);
    
    if (currentDataHash != vec->dataHashSum) 
    {
        (const_cast<Vector*>(vec))->errorStatus |= DATA_HASH_ERROR;
        V_DBG(fprintf(stderr, RED "DATA HASH MISMATCH in vectorGet\n" RESET);)
        return POISON;
    }
    
    if (currentVectorHash != vec->vectorHashSum) 
    {
        (const_cast<Vector*>(vec))->errorStatus |= VECTOR_HASH_ERROR;
        V_DBG(fprintf(stderr, RED "STRUCT HASH MISMATCH in vectorGet\n" RESET);)
        return POISON;
    }
    #endif

    return vec->data[index + 1]; // +1 because of canary
}

#undef VERIFICATION

static void vectorDataDump(const Vector vec)
{
    printf(GREEN "{\n" RESET);

    for (size_t i = 0; i < vec.capacity; ++i)
    {
        printf("  " GREEN "[" MANG "%3zu" GREEN "] = ", i);

        VectorElem_t val = vec.data[i];

        if (val == POISON)
            printf(RED "<POISON>" RESET);
        #ifdef VECTOR_CANARY_PROTECTION
        else if (val == L_DATA_KANAR)
            printf(RED "<L_DATA_CANARY>" RESET);
        else if (val == R_DATA_KANAR)
            printf(RED "<R_DATA_CANARY>" RESET);
        #endif
        else
            printf(RED "%p" GREEN " (%" PRIuPTR ")" RESET, val,
                   (uintptr_t)val);

        putchar('\n');
    }

    printf(GREEN "}\n" RESET);

    #ifdef VECTOR_CANARY_PROTECTION
    printf(BLUE "vec.data" GREEN "[0]          = "RED"%p" GREEN
        " ; must be %p\n" RESET,
        vec.data[0], L_DATA_KANAR);
    printf(BLUE "vec.data" GREEN "[capacity-1] = " RED "%p" GREEN
        " ; must be %p\n" RESET,
        vec.data[vec.capacity - 1], R_DATA_KANAR);
    #endif
}

void vectorDump(const Vector vec)
{
    printf(RED  "___vectorDump____________________________________________________________\n" RESET);

    #ifdef VECTOR_CANARY_PROTECTION
    printf(GREEN "{ "
        BLUE  "L_STACK_CANARY" GREEN " = " RED "%p" GREEN ", "
        BLUE  "R_STACK_CANARY" GREEN " = " RED "%p" GREEN " }\n" RESET,
        vec.leftVectorCanary, vec.rightVectorCanary);

    printf(GREEN "{ "
        BLUE  "L_DATA_CANARY"  GREEN " = " RED "%p" GREEN ", "
        BLUE  "R_DATA_CANARY"  GREEN " = " RED "%p" GREEN " }\n" RESET,
        vec.data ? vec.data[0] : nullptr,
        vec.data ? vec.data[vec.capacity - 1] : nullptr);
    #endif

    printf(BLUE "capacity" GREEN " = " RED "%zu" RESET ", "
           BLUE "size"     GREEN " = " RED "%zu" RESET "\n",
           vec.capacity, vec.size);

    printf(CEAN "data" GREEN " [ " MANG "%p" GREEN " ]\n" RESET, vec.data);

    vectorDataDump(vec);

    printf(RED "_________________________________________________________________________\n" RESET);
}


uint64_t vectorVerify(Vector* vec)
{
    V_DBG(ASSERT(vec, "vec = nullptr", stderr);)
    
    if (!vec)
        return POINTER_ERROR;
    
    uint64_t errors = OK;

    if (vec->size > vec->capacity - 2)  // -2 for canaries
        errors |= SIZE_ERROR;
    
    #ifdef VECTOR_CANARY_PROTECTION
    if (vec->leftVectorCanary  != L_STACK_KANAR)
        errors |= LEFT_VECTOR_CANARY_DIED;
        
    if (vec->rightVectorCanary != R_STACK_KANAR)
        errors |= RIGHT_VECTOR_CANARY_DIED;
    
    if (!vec->data)
    {
        if (vec -> capacity == 0)
            errors |= POINTER_ERROR;
    }
    else 
    {
        if (vec->data[0]                 != L_DATA_KANAR)                  
            errors |= LEFT_DATA_CANARY_DIED;                       
            
        if (vec->data[vec->capacity - 1] != R_DATA_KANAR) 
            errors |= RIGHT_DATA_CANARY_DIED;                       
    }
    #endif

    #ifdef VECTOR_HASH_PROTECTION
    if (vec->data && vec->capacity > 0) // check data hash
    {
        uint64_t currentDataHash = vectorDataHashCalc(reinterpret_cast<const char*>(vec->data), 
                                                      reinterpret_cast<const char*>(vec->data + vec->capacity));
        if (currentDataHash != vec->dataHashSum)
            errors |= DATA_HASH_ERROR;
    }

    
    uint64_t currentStackHash = vectorStructHashCalc(vec);
    if (currentStackHash != vec->vectorHashSum) // check stack hash
        errors |= VECTOR_HASH_ERROR;
    #endif
        
    vec->errorStatus = errors;
    return (VectorError)errors;
}
    
static const char* VectorErrors[NUMBER_OF_ERRORS] = {
                                                    "POINTER_ERROR",
                                                    "ALLOC_ERROR",
                                                    "SIZE_ERROR",
                                                    "LEFT_VECTOR_CANARY_DIED",
                                                    "RIGHT_VECTOR_CANARY_DIED",
                                                    "LEFT_DATA_CANARY_DIED",
                                                    "RIGHT_DATA_CANARY_DIED",
                                                    "EMPTY_VECTOR",
                                                    "VECTOR_HASH_ERROR",
                                                    "DATA_HASH_ERROR",
                                                    "INIT_HASH_ERROR",   
                                                   };
VectorError vectorErrorDump(const Vector vec)
{
    printf("%s___vectorErrorDump___~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~%s\n", RED, RESET);
    for (size_t i = 0; i < NUMBER_OF_ERRORS; i++)
    {
        if (vec.errorStatus & (1 << i))
            fprintf(stderr, RED"error: code %zu ( %s )\n"RESET, i + 1, VectorErrors[i]);
    }
    return OK;
}
