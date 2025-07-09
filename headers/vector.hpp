#ifndef STACK_MAIN_H
#define STACK_MAIN_H

typedef void* Canary_t;
typedef void* VectorElem_t;

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "configFile.hpp"

#ifdef VECTOR_DEBUG
    #define V_DBG(...) __VA_ARGS__
#else
    #define V_DBG(...)
#endif

#ifdef VECTOR_CANARY_PROTECTION
    #define V_CAN_PR(...) __VA_ARGS__
#else
    #define V_CAN_PR(...)
#endif

#ifdef VECTOR_HASH_PROTECTION
    #define V_HASH_PR(...) __VA_ARGS__
#else
    #define V_HASH_PR(...)
#endif

struct Vector
{
    V_CAN_PR(Canary_t leftVectorCanary;)

    size_t   coefCapacity;
    uint64_t errorStatus;

    void** data;
    size_t       size;
    size_t       capacity;

    #ifdef VECTOR_HASH_PROTECTION
    uint64_t dataHashSum;
    uint64_t vectorHashSum;
    #endif

    V_CAN_PR(Canary_t rightVectorCanary;)
};

enum VectorError
{
    OK                       = 0,
    POINTER_ERROR            = 1 << 0,
    ALLOC_ERROR              = 1 << 1,
    SIZE_ERROR               = 1 << 2,
    LEFT_VECTOR_CANARY_DIED  = 1 << 3,
    RIGHT_VECTOR_CANARY_DIED = 1 << 4,
    LEFT_DATA_CANARY_DIED    = 1 << 5,
    RIGHT_DATA_CANARY_DIED   = 1 << 6,
    EMPTY_VECTOR             = 1 << 7,
    VECTOR_HASH_ERROR        = 1 << 8,
    DATA_HASH_ERROR          = 1 << 9,
    INIT_HASH_ERROR          = 1 << 10,   
    INDEX_OUT_OF_RANGE       = 1 << 11,
    NUMBER_OF_ERRORS
};

const size_t       START_SIZE       = 16;
const VectorElem_t POISON           = (VectorElem_t)-666;
const size_t       REDUCER_CAPACITY = 2;
const uint64_t     HASH_COEFF       = 33;

const Canary_t L_DATA_KANAR  = (void*)0xEDAA;
const Canary_t R_DATA_KANAR  = (void*)0xF00D;
const Canary_t L_STACK_KANAR = (void*)0xBEDA;
const Canary_t R_STACK_KANAR = (void*)0x0DED;

void vectorCtor(Vector* stk);
void vectorDtor(Vector* stk);

VectorError  vectorPush(Vector* vec, VectorElem_t value);
VectorElem_t vectorPop (Vector* vec);
VectorElem_t vectorGet (const Vector* vec, const size_t index);

uint64_t vectorVerify(Vector* vec);

void        vectorDump     (const Vector vec);
VectorError vectorErrorDump(const Vector vec);

#endif
