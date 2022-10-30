#pragma once
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <cstdio>

constexpr unsigned char BMP_Header[] =
        {0x42, 0x4D,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00,
         0x00, 0x00,
         0x36, 0x00, 0x00, 0x00};
constexpr unsigned char DIB_Header[] =
        {0x28, 0x00, 0x00, 0x00,
         0x03, 0x00, 0x00, 0x00,    /// Width pix
         0x03, 0x00, 0x00, 0x00,    /// Height pix
         0x01, 0x00,
         0x18, 0x00,                ///24 bits for one pixel (3 * 8)
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x13, 0x0B, 0x00, 0x00,
         0x13, 0x0B, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00};

constexpr unsigned char size_pixel = 3;
constexpr unsigned char k_row = 4;

enum Color {
    WHITE = 0xFFFFFF,
    GREEN = 0x408000,
    VIOLET = 0x7608AA,
    YELLOW = 0xFFD800,
    BLACK = 0x000000
};

#define MAX(x, y) x > y ? (x) : (y)
#define MIN(x, y) x < y ? (x) : (y)

template<typename T_size, typename T_data>
bool save_array_to_BMP(char *name, T_size height, T_size width, T_data *data)
{
    size_t row_size = (size_pixel * width);
    size_t row_padding_size = (k_row - (row_size % k_row)) % k_row;
    row_size += row_padding_size;
    size_t bitmap_size = row_size * height;
    size_t all_bmp_size = sizeof(BMP_Header) + sizeof(DIB_Header) + bitmap_size;

/// auto *all_bmp = (uint8_t *)malloc(all_bmp_size);
    auto *all_bmp = static_cast<uint8_t *> (malloc(all_bmp_size));

    if(all_bmp == nullptr)
        return false;

    size_t pos = 0;
    memcpy(all_bmp, &BMP_Header, sizeof(BMP_Header));
    pos += sizeof(BMP_Header);
    memcpy(all_bmp + pos, &DIB_Header, sizeof(DIB_Header));
    pos += sizeof(DIB_Header);

    memcpy(all_bmp + sizeof(BMP_Header) + 4, &width, MIN(sizeof(T_size), 4));
    memcpy(all_bmp + sizeof(BMP_Header) + 8, &height, MIN(sizeof(T_size), 4));

    for(size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            Color color;
            switch (data[i*height + j]) {
                case 0:
                    color = WHITE;
                    break;
                case 1:
                    color = GREEN;
                    break;
                case 2:
                    color = VIOLET;
                    break;
                case 3:
                    color = YELLOW;
                    break;
                default:
                    color = BLACK;
                    break;
            }
            memcpy(all_bmp + pos, &color, size_pixel);
            pos += size_pixel;
        }
        if(row_padding_size) {
            uint_fast32_t padding = 0;
            memcpy(all_bmp + pos, &padding, row_padding_size);
            pos += row_padding_size;
        }
    }

    FILE *out = fopen(name, "wb");
    if(out == nullptr)
        return false;

    fwrite(all_bmp, all_bmp_size, 1, out);

    fclose(out);
    free(all_bmp);
    return true;
}
