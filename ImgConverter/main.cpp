#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>
#include <bmp_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>
#include <memory>

using namespace std;

enum class Format {
    JPEG,
    PPM,
    BMP,
    UNKNOWN
};
    
Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }
    
    if (ext == ".bmp"sv) {
        return Format::BMP;
    }

    return Format::UNKNOWN;
}
    
class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};

class JPEGFormatImage : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const {
        return img_lib::SaveJPEG(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const {
        return img_lib::LoadJPEG(file);
    }
};

class PPMFormatImage : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const {
        return img_lib::SavePPM(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const {
        return img_lib::LoadPPM(file);
    }
};

class BMPFormatImage : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const {
        return img_lib::SaveBMP(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const {
        return img_lib::LoadBMP(file);
    }
};


ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
    switch (GetFormatByExtension(path)) {
        case Format::JPEG:
            return new JPEGFormatImage();
        case Format::PPM:
            return new PPMFormatImage();
        case Format::BMP:
            return new BMPFormatImage();
        case Format::UNKNOWN:
            return nullptr;
    }
}

int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];
    ImageFormatInterface* input_formatter = GetFormatInterface(in_path);
    if (input_formatter == nullptr) {
        cerr << "Unknown format of the input file" << endl;
        return 2;
    }
    ImageFormatInterface* output_formatter = GetFormatInterface(out_path);
    if (output_formatter == nullptr) {
        cerr << "Unknown format of the output file" << endl;
        return 3;
    }

    img_lib::Image image = input_formatter->LoadImage(in_path);
    if (!image) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!output_formatter->SaveImage(out_path, image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;
    
    delete input_formatter;
    delete output_formatter;
}
