#ifndef MYLIB_H
#define MYLIB_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <locale.h>


/*===============================================_____COLORS_____===============================================*/
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define MANG    "\033[1;35m"
#define WHITE   "\033[1;37m"
#define GREEN   "\033[1;32m"
#define BLUE    "\033[1;34m"
#define CEAN    "\033[1;36m"
#define YELLOW  "\033[1;33m"

#define _RED   (T) "\033[1;31m" T "\033[0m"
#define _MANG  (T) "\033[1;35m" T "\033[0m"
#define _WHITE (T) "\033[1;37m" T "\033[0m"
#define _GREEN (T) "\033[1;32m" T "\033[0m"
#define _BLUE  (T) "\033[1;34m" T "\033[0m"
#define _CEAN  (T) "\033[1;36m" T "\033[0m"
#define _YELLOW(T) "\033[1;33m" T "\033[0m"
/*==============================================================================================================*/


/*===============================================_____MODIFICATIONS_____========================================*/
#define FREE(ptr_) \
    do{free(ptr_)  ; ptr_ = NULL;} while(false)

#define FCLOSE(ptr_) \
    do{fclose(ptr_); ptr_ = NULL;} while(false) 
/*==============================================================================================================*/


/*===============================================_____ERRORS_____===============================================*/
#define SYNTAX_ERROR(errorMessage, output)                                         \
    do                                                                             \
    {                                                                              \
        fprintf(output, "Syntax Error: file %s, line %d \n", __FILE__, __LINE__);  \
        fprintf(output, errorMessage "\nexit with 1 status\n");                    \
        exit(1);                                                                   \
    }                                                                              \
    while (false)

#define ASSERT(ptr, report, output)                                                \
    do                                                                             \
    {                                                                              \
        if (!ptr)                                                                  \
        {                                                                          \
            fprintf(output, "ERROR IN FILE %s: LINE %d: ", __FILE__, __LINE__);    \
            fprintf(output, report "\n");                                          \
            exit(1);                                                               \
        }                                                                          \
    }                                                                              \
    while (false)
/*==============================================================================================================*/

/*==============================================================================================================*/


/*===============================================_____DEBUG_____================================================*/
#define SUCCESS \
    printf("\n%s!!!%s_________%sTHE PROGRAM FINISHED WITHOUT SIGFAULT%s_________%s!!!%s\n", RED, YELLOW, GREEN, YELLOW, RED, RESET)

#define PRINT(num) \
    printf("%s!!!%s_________%sTHIS IS OK%s_________%s!!!%s (%s%d%s) %s\n", RED, YELLOW, GREEN, YELLOW, RED, MANG, YELLOW, num,  MANG, RESET);

#define DEBUG 1

#if  (DEBUG == 1) 
    #define DBG( ... ) __VA_ARGS__
#else 
    #define DBG( ... )
#endif
/*==============================================================================================================*/


/*===============================================_____WORKING_WITH_FILES_____===================================*/
size_t getFileSize(FILE* file);
char*  readFileToBuffer(FILE* file, size_t size_file);
/*==============================================================================================================*/


/*===============================================_____ENCODINGS___==============================================*/
#define RUSSIAN_ENCODING setlocale(LC_ALL, "Russian")
/*==============================================================================================================*/

/*===============================================_____CMP_____==================================================*/
const double DOUBLE_EPSILON  = 1e-9;
bool doubleCmp(double a, double b);
/*==============================================================================================================*/


#endif