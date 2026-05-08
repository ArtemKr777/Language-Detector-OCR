#ifndef LANGUAGEDETECTOR_H
#define LANGUAGEDETECTOR_H

#include <string>
#include <map>

struct DetectionResult {
    std::string language;
    double confidence;
    std::string explanation;
};

class LanguageDetector {
public:
    LanguageDetector();

    void loadProfiles();
    DetectionResult detectLanguage(const std::string& rawText);

    // Для получения дополнительной информации
    std::map<std::string, double> getLastDistances() const { return lastDistances; }

private:
    std::map<std::string, std::map<std::string, double>> profiles;
    std::map<std::string, double> lastDistances;

    double manhattanDistance(const std::map<std::string, double>& obs,
        const std::map<std::string, double>& model);
};

#endif // LANGUAGEDETECTOR_H