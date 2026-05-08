#include "languagedetector.h"
#include "utils.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

LanguageDetector::LanguageDetector() {
    loadProfiles();
}

void LanguageDetector::loadProfiles() {
    profiles = Utils::loadLanguageProfiles();
}

double LanguageDetector::manhattanDistance(
    const std::map<std::string, double>& obs,
    const std::map<std::string, double>& model
) {
    double dist = 0.0;
    for (const auto& m : model) {
        double o = 0.0;
        auto it = obs.find(m.first);
        if (it != obs.end()) o = it->second;
        dist += std::abs(o - m.second);
    }
    return dist;
}

// СОБСТВЕННАЯ РЕАЛИЗАЦИЯ СОРТИРОВКИ ПУЗЫРЬКОМ
// Сортирует вектор пар по значению (расстоянию) по возрастанию
void bubbleSort(std::vector<std::pair<std::string, double>>& arr) {
    size_t n = arr.size();
    bool swapped;

    for (size_t i = 0; i < n - 1; i++) {
        swapped = false;
        for (size_t j = 0; j < n - i - 1; j++) {
            // Сравниваем по значению (второй элемент пары)
            if (arr[j].second > arr[j + 1].second) {
                // Обмен
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        // Если не было обменов, массив уже отсортирован
        if (!swapped) break;
    }
}

DetectionResult LanguageDetector::detectLanguage(const std::string& rawText) {
    DetectionResult result;
    result.language = "unknown";
    result.confidence = 0.0;

    // 1. НОРМАЛИЗАЦИЯ ТЕКСТА
    std::string normalized = Utils::normalizeText(rawText);
    if (normalized.length() < 20) {
        result.explanation = "Text too short after normalization (<20 letters).";
        return result;
    }
 
    // 2. ИЗВЛЕЧЕНИЕ БИГРАММ И РАСЧЁТ ВЕРОЯТНОСТЕЙ
    auto bigramCounts = Utils::extractBigrams(normalized);
    auto observedProbs = Utils::frequenciesToProbabilities(bigramCounts);

    // 3. РАСЧЁТ РАССТОЯНИЙ ДЛЯ ВСЕХ ЯЗЫКОВ
    lastDistances.clear();
    std::vector<std::pair<std::string, double>> distanceList;

    for (const auto& prof : profiles) {
        double dist = manhattanDistance(observedProbs, prof.second);
        lastDistances[prof.first] = dist;
        distanceList.push_back({ prof.first, dist });
    }

    // 4. СОРТИРОВКА ПО РАССТОЯНИЮ (СОБСТВЕННАЯ РЕАЛИЗАЦИЯ - ПУЗЫРЁК)
    bubbleSort(distanceList);

    double bestDist = distanceList[0].second;
    std::string bestLang = distanceList[0].first;

    double secondBestDist = distanceList[1].second;
    std::string secondBestLang = distanceList[1].first;

    // 5. ПРОВЕРКА НА НЕОДНОЗНАЧНОСТЬ
    const double AMBIGUITY_THRESHOLD = 0.05;
    bool isAmbiguous = (secondBestDist - bestDist) < AMBIGUITY_THRESHOLD;

    // 6. РАСЧЁТ УВЕРЕННОСТИ
    double maxPossibleDist = 1.0;
    double confidence = 1.0 - (bestDist / maxPossibleDist);
    if (confidence < 0.0) confidence = 0.0;

    const double CONF_THRESHOLD = 0.45;

    // 7. ПРИНЯТИЕ РЕШЕНИЯ
    if (confidence < CONF_THRESHOLD) {
        result.language = "unknown";
        result.confidence = confidence;
        result.explanation = "Confidence too low (best: " + bestLang + "=" + std::to_string(bestDist) + ")";
        return result;
    }

    if (isAmbiguous) {
        result.language = "unknown";
        result.confidence = confidence;
        result.explanation = "Ambiguous: " + bestLang + " (" + std::to_string(bestDist) +
            ") and " + secondBestLang + " (" + std::to_string(secondBestDist) +
            ") are too close (diff < " + std::to_string(AMBIGUITY_THRESHOLD) + ")";
        return result;
    }

    // 8. УСПЕШНОЕ ОПРЕДЕЛЕНИЕ
    result.language = bestLang;
    result.confidence = confidence;

    // Формируем пояснение со всеми расстояниями
    std::string exp = "Manhattan distances (all): ";
    for (const auto& d : distanceList) {
        exp += d.first + "=" + std::to_string(d.second) + "; ";
    }
    exp += "Best: " + bestLang + " (conf=" + std::to_string(confidence) + ")";
    result.explanation = exp;

    return result;
}