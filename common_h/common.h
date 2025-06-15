#ifndef COMMON_H
#define COMMON_H

#define MSG_SIZE 15 //includes null terminator

#define RESET    "______reset___"
#define START    "______start___"
#define STOP     "_______stop___"
#define RECEIVED "___received___"

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