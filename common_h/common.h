#ifndef COMMON_H
#define COMMON_H

#define MSG_SIZE 15 //includes null terminator

#define PING     "______ping___0"
#define RESET    "______reset__1"
#define START    "______start__2"
#define STOP     "_______stop__3"
#define RECEIVED "___received__4"

#define TICK 300 //ms

#define PARSE_OFFSET MSG_SIZE - 2

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