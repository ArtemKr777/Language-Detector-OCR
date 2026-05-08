#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>
#include <cctype>
#include <limits>

#define NOMINMAX

#ifdef _WIN32
#include <windows.h>
#endif

#include "recognizer.h"
#include "languagedetector.h"
#include "utils.h"

using namespace std::chrono;

const std::string TEST_IMAGES_DIR = "C:/Users/Admin/source/repos/Распознавание текста/Распознавание текста/test_images";

std::wstring toWide(const std::string& str) {
    return Utils::stringToWstring(str);
}

bool fileExists(const std::string& path) {
    std::ifstream f(path.c_str());
    return f.good();
}

// Извлечение имени файла из полного пути
std::string getFilename(const std::string& path) {
    size_t slashPos = path.find_last_of("/\\");
    if (slashPos != std::string::npos) {
        return path.substr(slashPos + 1);
    }
    return path;
}

// БЕЗОПАСНЫЙ ВВОД ЦЕЛОГО ЧИСЛА
int safeGetInt(const std::wstring& prompt, int minValue, int maxValue) {
    int value;
    while (true) {
        std::wcout << prompt;
        std::wcin >> value;

        if (std::wcin.fail()) {
            std::wcout << L"Ошибка: введите число!" << std::endl;
            std::wcin.clear();
            std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
            continue;
        }

        if (value < minValue || value > maxValue) {
            std::wcout << L"Ошибка: введите число от " << minValue << L" до " << maxValue << L"!" << std::endl;
            continue;
        }

        std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
        return value;
    }
}

// Функция получения списка изображений из папки
std::vector<std::string> getImageList(const std::string& directory) {
    std::vector<std::string> images;

    std::string searchPath = directory + "/*.*";

    struct _finddata_t fileInfo;
    intptr_t handle = _findfirst(searchPath.c_str(), &fileInfo);

    if (handle == -1) {
        std::wcout << L"Папка не найдена: " << toWide(directory) << std::endl;
        return images;
    }

    do {
        std::string filename = fileInfo.name;
        if (filename != "." && filename != "..") {
            std::string ext;
            size_t dotPos = filename.find_last_of('.');
            if (dotPos != std::string::npos) {
                ext = filename.substr(dotPos);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                    ext == ".bmp" || ext == ".tiff" || ext == ".tif") {
                    images.push_back(directory + "/" + filename);
                }
            }
        }
    } while (_findnext(handle, &fileInfo) == 0);

    _findclose(handle);
    return images;
}

// Отображение главного меню
int showMainMenu() {
    std::wcout << L"\n========================================" << std::endl;
    std::wcout << L"   OCR LANGUAGE DETECTOR" << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"1. Выбрать изображение для распознавания" << std::endl;
    std::wcout << L"0. Выход" << std::endl;
    std::wcout << L"----------------------------------------" << std::endl;

    return safeGetInt(L"Ваш выбор: ", 0, 1);
}

// Выбор изображения из списка
std::string selectImage() {
    std::vector<std::string> images = getImageList(TEST_IMAGES_DIR);

    if (images.empty()) {
        std::wcout << L"\nНет изображений в папке: " << toWide(TEST_IMAGES_DIR) << std::endl;
        return "";
    }

    std::wcout << L"\n========================================" << std::endl;
    std::wcout << L"   СПИСОК ДОСТУПНЫХ ИЗОБРАЖЕНИЙ" << std::endl;
    std::wcout << L"========================================" << std::endl;

    for (size_t i = 0; i < images.size(); i++) {
        std::string filename = getFilename(images[i]);
        std::wcout << L"  " << (i + 1) << L". " << toWide(filename) << std::endl;
    }

    std::wcout << L"----------------------------------------" << std::endl;

    int choice = safeGetInt(L"Введите номер изображения (1-" + std::to_wstring(images.size()) + L"): ",
        1, static_cast<int>(images.size()));

    return images[choice - 1];
}

// Функция обработки одного изображения
void processImage(const std::string& imagePath) {
    // Выводим имя файла
    std::wcout << L"\n=== Обработка изображения ===" << std::endl;
    std::wcout << L"Файл: " << toWide(getFilename(imagePath)) << std::endl;

    // Первичное распознавание (все языки)
    TextRecognizer recognizer;
    if (!recognizer.init("rus+eng+deu+fra+ita+pol")) {
        std::wcout << L"Ошибка инициализации Tesseract." << std::endl;
        return;
    }

    std::string initialText = recognizer.recognizeFromFile(imagePath);
    if (initialText.empty()) {
        std::wcout << L"Текст не распознан." << std::endl;
        return;
    }

    std::wcout << L"\n--- Первичное распознавание (все языки) ---" << std::endl;
    std::wcout << toWide(initialText) << std::endl;

    // Определение языка
    auto start = high_resolution_clock::now();

    LanguageDetector detector;
    DetectionResult detResult = detector.detectLanguage(initialText);

    auto end = high_resolution_clock::now();
    double elapsed = duration<double>(end - start).count();

    // Вывод результатов
    std::wcout << L"\n--- Определение языка ---" << std::endl;
    std::wcout << L"Время работы алгоритма: " << elapsed << L" секунд" << std::endl;
    std::wcout << L"Определённый язык: " << toWide(detResult.language) << std::endl;
    std::wcout << L"Уверенность: " << detResult.confidence << std::endl;
    std::wcout << L"Пояснение: " << toWide(detResult.explanation) << std::endl;

    // Вывод всех расстояний
    auto distances = detector.getLastDistances();
    std::wcout << L"\n--- Расстояния Манхэттена для всех языков ---" << std::endl;
    for (const auto& d : distances) {
        std::wcout << L"  " << toWide(d.first) << L": " << d.second << std::endl;
    }

    // Повторное распознавание с выбранным языком
    if (detResult.language != "unknown") {
        std::string tesseractLang;
        if (detResult.language == "english") tesseractLang = "eng";
        else if (detResult.language == "russian") tesseractLang = "rus";
        else if (detResult.language == "german") tesseractLang = "deu";
        else if (detResult.language == "french") tesseractLang = "fra";
        else if (detResult.language == "italian") tesseractLang = "ita";
        else if (detResult.language == "polish") tesseractLang = "pol";
        else tesseractLang = "eng";

        TextRecognizer refinedRecognizer;
        if (refinedRecognizer.init(tesseractLang)) {
            std::string refinedText = refinedRecognizer.recognizeFromFile(imagePath);
            std::wcout << L"\n--- Повторное распознавание (язык: "
                << toWide(tesseractLang) << L") ---" << std::endl;
            std::wcout << toWide(refinedText) << std::endl;
        }
        else {
            std::wcout << L"\nНе удалось инициализировать Tesseract с языком "
                << toWide(tesseractLang) << std::endl;
        }
    }
    else {
        std::wcout << L"\nЯзык не определён надёжно. Повторное распознавание пропущено." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Настройка консоли
    _setmode(_fileno(stdout), _O_U16TEXT);

#ifdef _WIN32
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 16;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
#endif

    // Главный цикл программы
    while (true) {
        int choice = showMainMenu();

        if (choice == 0) {
            std::wcout << L"\nПрограмма завершена." << std::endl;
            break;
        }
        else if (choice == 1) {
            std::string imagePath = selectImage();
            if (!imagePath.empty()) {
                processImage(imagePath);
            }
            else {
                std::wcout << L"\nНе удалось выбрать изображение." << std::endl;
            }

            // Пауза перед возвратом в меню
            std::wcout << L"\nНажмите Enter, чтобы продолжить..." << std::endl;
            std::wcin.get();
            std::wcin.get();  // двойной вызов для очистки буфера
        }
    }

    return 0;
}