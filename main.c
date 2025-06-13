// blp2_to_dds.c
// Minimal BLP v2 → DDS converter for DXT-encoded textures.
//   gcc -std=c99 -O2 -o blp2_to_dds blp2_to_dds.c
//
// Usage:  ./blp2_to_dds <input.blp> <output.dds>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <windows.h>
#include "type_headers.h"

static int convert(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) { perror("open input"); return 1; }

    BLPHeader h;
    if (fread(&h, sizeof h, 1, in) != 1) { fprintf(stderr, "read header\n");
        return 1; }

    if (memcmp(h.magic, "BLP2", 4) != 0 || h.version != 1) {
        fprintf(stderr, "not a BLP v2 file\n"); return 1;
    }

    if (h.compression != 2) {
        fprintf(stderr, "only DXT-encoded BLP2 supported\n"); return 1;
    }

    uint32_t fourCC; // four char code
    switch (h.pixelFormat) {
        case 0:
        case 1:
        case 4:
            fourCC = 0x31545844; // DXT1
            break;
        case 5:
            fourCC = 0x33545844; // DXT3
            break;
        case 7:
            fourCC = 0x35545844; // DXT5
            break;
        default:
            fprintf(stderr, "unsupported pixelFormat %u\n", h.pixelFormat); return 1;
    }

    /* Build DDS header */
    DDSHeader dds = {0};
    dds.magic        = 0x20534444;      // "DDS "
    dds.size         = 124;
    dds.flags        = 0x0002100F;      // caps | height | width | pixelfmt | mipmapcount | linearsize
    dds.height       = h.height;
    dds.width        = h.width;
    dds.pitchOrSize  = h.mipSize[0];
    dds.mipMapCount  = (h.mipOffset[1] != 0) ? 16 : 1;

    dds.pfSize       = 32;
    dds.pfFlags      = 0x4;             // DDPF_FOURCC
    dds.pfFourCC     = fourCC;

    dds.caps1        = 0x401008;        // texture | mipmap | complex

    FILE *out = fopen(dst, "wb");
    if (!out) { perror("open output"); return 1; }

    fwrite(&dds, sizeof dds, 1, out);


    for (int i = 0; i < 16 && h.mipOffset[i]; ++i) {
        uint32_t off  = h.mipOffset[i];
        uint32_t size = h.mipSize[i];

        fseek(in, off, SEEK_SET);
        uint8_t *buf = malloc(size);
        if (!buf) { fprintf(stderr, "oom\n"); return 1; }
        fread(buf, size, 1, in);
        fwrite(buf, size, 1, out);
        free(buf);
    }

    fclose(out);
    fclose(in);
    printf("Wrote DDS: %s\n", dst);
    return 0;
}

// get all files in directory
char** dir_list(void) {
    char**  files = malloc(sizeof(char*) * (MAXIMUM_FILES + 1));
    if (!files)
        return NULL;

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile("*.blp", &findData);

    if  (hFind == INVALID_HANDLE_VALUE) {
        printf("Error finding files: %lu", GetLastError());
        return NULL;
    }

    int count = 0;
    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;

        files[count] = _strdup(findData.cFileName);
        if (!files[count])
            break;

        count++;

        if (count >= MAXIMUM_FILES)
            break;

    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);
    files[count] = NULL;
    return files;
}

// iterate through all files and convert
void converter_call(void) {
    DWORD file_attributes = GetFileAttributesA(OUT_DIR);
    if (file_attributes == INVALID_FILE_ATTRIBUTES || !(file_attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        if (!CreateDirectoryA(OUT_DIR, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
            fprintf(stderr, "Could not create\"%s\": %lu\n", OUT_DIR, GetLastError());
            return;
        }
    }

    char** files = dir_list();
    if (!files)
        return;

    for (int i = 0; files[i] != NULL; i++) {
        char* orig_name = files[i];
        size_t len = strlen(orig_name);

        char stem[MAXIMUM_PATH];
        memcpy(stem, orig_name, len - 4);
        stem[len - 4] = '\0';

        char outpath[MAXIMUM_PATH];
        _snprintf(outpath, sizeof(outpath), "%s\\%s.dds", OUT_DIR, stem);

        convert(orig_name, outpath);

        free(orig_name);
    }
    free(files);
}

int main(void) {

    converter_call();
    return 0;
}