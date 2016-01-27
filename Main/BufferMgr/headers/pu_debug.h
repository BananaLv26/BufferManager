#ifndef __PU_DEBUG__
#define __PU_DEBUG__

// #define DBG_LEVEL 0
#define EBUG

// debug mode, -DEBUG
#ifdef EBUG
    #define FNAME() fprintf(stderr, "\n%s (%s, line %d)\n", __func__, __FILE__, __LINE__)
    #define DBG(...) do{fprintf(stderr, "(%s, %d): ", __func__, __LINE__); \
                    fprintf(stderr, __VA_ARGS__);           \
                    /*fprintf(stderr, "\n");*/} while(0)
#else
    #define FNAME()  do{}while(0)
    #define DBG(...) do{}while(0)
#endif //EBUG

#define ERR(...) do{fprintf(stderr, "(%s, %d): ", __func__, __LINE__); \
    fprintf(stderr, __VA_ARGS__);           \
    /*fprintf(stderr, "\n");*/} while(0)

#endif