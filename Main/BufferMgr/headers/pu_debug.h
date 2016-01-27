#ifndef __PU_DEBUG__
#define __PU_DEBUG__

#define DBG_LEVEL 1
// #define EBUG

// debug mode, -DEBUG
#if(DBG_LEVEL <= 0)
    #define DBG(...) do{fprintf(stderr, "(%s, %d): ", __func__, __LINE__); \
                    fprintf(stderr, __VA_ARGS__);           \
                    /*fprintf(stderr, "\n");*/} while(0)
#else
    #define DBG(...) do{}while(0)
#endif 

#if(DBG_LEVEL <= 1)
    #define DBG1(...) do{fprintf(stderr, "(%s, %d): ", __func__, __LINE__); \
                    fprintf(stderr, __VA_ARGS__);           \
                    /*fprintf(stderr, "\n");*/} while(0)
#else
    #define DBG1(...) do{}while(0)
#endif

#if(DBG_LEVEL <= 2)
    #define DBG2(...) do{fprintf(stderr, "(%s, %d): ", __func__, __LINE__); \
                    fprintf(stderr, __VA_ARGS__);           \
                    /*fprintf(stderr, "\n");*/} while(0)
#else
    #define DBG2(...) do{}while(0)
#endif


#define ERR(...) do{fprintf(stderr, "(%s, %d): ", __func__, __LINE__); \
    fprintf(stderr, __VA_ARGS__);           \
    /*fprintf(stderr, "\n");*/} while(0)

#endif