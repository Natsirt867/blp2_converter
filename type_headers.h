#ifndef TYPE_HEADERS_H
#define TYPE_HEADERS_H

#include <stdint.h>

#define MAXIMUM_FILES 1024
#define MAXIMUM_PATH 1024
#define OUT_DIR "./dds"

#pragma pack(push, 1)

typedef struct {
    char   magic[4];            // "BLP2"
    uint32_t version;           // always 1
    uint8_t compression;        // 0=JPEG 1=RAW 2=DXT
    uint8_t alphaBits;          // 0/1/4/8
    uint8_t pixelFormat;        // 4=DXT1 5=DXT3 7=DXT5
    uint8_t mipFlags;           // 0=none 1=generated
    uint32_t width;
    uint32_t height;
    uint32_t mipOffset[16];
    uint32_t mipSize[16];
} BLPHeader;


typedef struct {
    uint32_t magic;             // "DDS "
    uint32_t size;              // 124
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];

    /* DDS_PIXELFORMAT */
    uint32_t pfSize;            // 32
    uint32_t pfFlags;
    uint32_t pfFourCC;
    uint32_t pfRGBBitCount;
    uint32_t pfRMask;
    uint32_t pfGMask;
    uint32_t pfBMask;
    uint32_t pfAMask;

    uint32_t caps1;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
} DDSHeader;

static void le32(FILE *f, uint32_t v) { fwrite(&v, 4, 1, f); }

#pragma pack(pop)

#endif
