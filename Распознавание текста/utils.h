#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <map>
#include <vector>

namespace Utils {

    // Конвертация UTF-8 → wstring (для вывода в консоль)
    std::wstring stringToWstring(const std::string& str);

    // Нормализация текста: нижний регистр, удаление пробелов/знаков,
    // транслитерация кириллицы, удаление диакритики
    std::string normalizeText(const std::string& text);

    // Извлечение биграмм с частотами
    std::map<std::string, int> extractBigrams(const std::string& text);

    // Преобразование частот в вероятности
    std::map<std::string, double> frequenciesToProbabilities(const std::map<std::string, int>& freqs);

    // Загрузка эталонных профилей для 6 языков
    std::map<std::string, std::map<std::string, double>> loadLanguageProfiles();

    // Удаление диакритики (поддержка немецкого, французского, итальянского, польского)
    std::string removeDiacritics(const std::string& text);
}

#endif // UTILS_H