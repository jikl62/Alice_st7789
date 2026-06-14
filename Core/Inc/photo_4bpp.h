#ifndef __PHOTO_4BPP_H
#define __PHOTO_4BPP_H

#include <stdint.h>

#define PHOTO_WIDTH         240U
#define PHOTO_HEIGHT        320U
#define PHOTO_BPP           4U
#define PHOTO_PACKED_SIZE   ((PHOTO_WIDTH * PHOTO_HEIGHT) / 2U)

extern const uint16_t photo_palette[16];
extern const uint8_t photo_pixels_4bpp[PHOTO_PACKED_SIZE];

#endif /* __PHOTO_4BPP_H */
