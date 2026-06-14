#ifndef __PHOTO_H
#define __PHOTO_H

#include <stdint.h>

#define PHOTO_WIDTH   96U
#define PHOTO_HEIGHT  128U
#define PHOTO_SIZE    (PHOTO_WIDTH * PHOTO_HEIGHT * 2U)

extern const uint8_t photo_data[PHOTO_SIZE];

#endif /* __PHOTO_H */
