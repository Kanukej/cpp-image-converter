#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

#include <iostream>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char sign[2];
    unsigned int size;
    unsigned int reserved;
    unsigned int padding;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    unsigned int size;
    int width;
    int height;
    unsigned short num_planes;
    unsigned short bits_per_pixel;
    unsigned int compression_type;
    unsigned int num_bytes;
    int hdpi;
    int vdpi;
    int num_colors;
    int val_colors;
}
PACKED_STRUCT_END

static const unsigned int FILE_HEADER_SIZE = 14;
static const unsigned int INFO_HEADER_SIZE = 40;
static const unsigned short NUM_PLANES = 1;
static const unsigned short BITS_PER_PIXEL = 24;
static const unsigned int COMPRESSION_TYPE = 0;
static const unsigned int HDPI = 11811;
static const unsigned int VDPI = 11811;
static const unsigned int NUM_COLORS = 0;
static const unsigned int VAL_COLORS = 0x1000000;
static const int NUM_CHANNELS = 3;
static const int ALIGN_TO = 4;

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return ALIGN_TO * ((w * NUM_CHANNELS + NUM_CHANNELS) / ALIGN_TO);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {    
    ofstream out(file, ios::binary);
    
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const int s = GetBMPStride(w);
    
    BitmapFileHeader file_header {
        {'B', 'M'},
        static_cast<unsigned int>(s * h + FILE_HEADER_SIZE + INFO_HEADER_SIZE),
        0x0000000,
        FILE_HEADER_SIZE + INFO_HEADER_SIZE
    };
    out.write((char*)&file_header, FILE_HEADER_SIZE);
    BitmapInfoHeader info_header {
        INFO_HEADER_SIZE,
        w,
        h,
        NUM_PLANES,
        BITS_PER_PIXEL,
        COMPRESSION_TYPE,
        static_cast<unsigned int>(s * h),
        HDPI,
        VDPI,
        NUM_COLORS,
        VAL_COLORS
    };
    out.write((char*)&info_header, INFO_HEADER_SIZE);
    
    std::vector<char> buff(s, 0);

    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), buff.size());
    }

    return out.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    // читаем заголовок: он содержит формат, размеры изображения
    // и максимальное значение цвета
    ifs.read((char*)&file_header, sizeof(file_header));
    if (ifs.bad()) {
        return {};
    }
    ifs.read((char*)&info_header, sizeof(info_header));
    if (ifs.bad()) {
        return {};
    }

    // мы поддерживаем изображения только формата P6
    // с максимальным значением цвета 255
    if (file_header.sign[0] != 'B' && file_header.sign[1] != 'M') {
        return {};
    }

    Image result(info_header.width, info_header.height, Color::Black());
    std::vector<char> buff(GetBMPStride(info_header.width), 0);

    for (int y = info_header.height - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), buff.size());
        if (ifs.bad()) {
            return {};
        }

        for (int x = 0; x < info_header.width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib
