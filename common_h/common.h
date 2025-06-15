#ifndef COMMON_H
#define COMMON_H

#define RESET "?__reset__?"
#define START "?__start__?"
#define STOP  "?___stop__?"

#define NO_MSG  0  
#define BAD_MSG 1
#define GOOD_MSG 2

#define KEY "5thAveMile"


#define XOR_CIPHER(data, len, key, keyLen)         \
    do {                                           \
        for (size_t i = 0; i < (len); ++i) {       \
            (data)[i] ^= (key)[i % (keyLen)];      \
        }                                          \
    } while (0)

#endif