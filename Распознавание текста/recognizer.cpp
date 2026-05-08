#include "recognizer.h"
#include <leptonica/allheaders.h>
#include <iostream>

// Путь к папке tessdata
static const std::string TESSDATA_PATH = "C:/Users/Admin/source/repos/Распознавание текста/Распознавание текста/tessdata";

TextRecognizer::TextRecognizer() : api(nullptr), initialized(false) {
    api = new tesseract::TessBaseAPI();
}

TextRecognizer::~TextRecognizer() {
    if (api) {
        api->End();
        delete api;
    }
}

bool TextRecognizer::init(const std::string& languages) {
    if (api->Init(TESSDATA_PATH.c_str(), languages.c_str())) {
        std::cerr << "Tesseract initialization failed." << std::endl;
        std::cerr << "TESSDATA_PATH = " << TESSDATA_PATH << std::endl;
        return false;
    }

    // Настройки для многоязычного распознавания
    api->SetVariable("ocr_engine_mode", "1");  // OEM_LSTM_ONLY
    api->SetPageSegMode(tesseract::PSM_AUTO);

    initialized = true;
    return true;
}

std::string TextRecognizer::recognizeFromFile(const std::string& imagePath) {
    if (!initialized) {
        std::cerr << "Recognizer not initialized." << std::endl;
        return "";
    }

    Pix* image = pixRead(imagePath.c_str());
    if (!image) {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
        return "";
    }

    api->SetImage(image);
    char* outText = api->GetUTF8Text();
    std::string result(outText);
    delete[] outText;
    pixDestroy(&image);

    return result;
}

void TextRecognizer::setPageSegMode(int mode) {
    if (api) {
        api->SetPageSegMode(static_cast<tesseract::PageSegMode>(mode));
    }
}