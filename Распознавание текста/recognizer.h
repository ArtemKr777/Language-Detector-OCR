#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <string>
#include <tesseract/baseapi.h>

class TextRecognizer {
public:
    TextRecognizer();
    ~TextRecognizer();

    bool init(const std::string& languages);
    std::string recognizeFromFile(const std::string& imagePath);
    void setPageSegMode(int mode);

private:
    tesseract::TessBaseAPI* api;
    bool initialized;
};

#endif // RECOGNIZER_H