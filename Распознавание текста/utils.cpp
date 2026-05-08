#include "utils.h"
#include <cctype>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <windows.h>

namespace Utils {

    // ============== КОНВЕРТАЦИЯ UTF-8 → WSTRING ==============
    std::wstring stringToWstring(const std::string& str) {
        if (str.empty()) return std::wstring();

        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
        if (sizeNeeded <= 0) return std::wstring();

        std::wstring result(sizeNeeded, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], sizeNeeded);

        return result;
    }

    // ============== ТРАНСЛИТЕРАЦИЯ КИРИЛЛИЦЫ ==============
    static std::string transliterateRussian(const std::string& text) {
        static const std::unordered_map<char, std::string> translitMap = {
            {'а', "a"}, {'б', "b"}, {'в', "v"}, {'г', "g"}, {'д', "d"},
            {'е', "e"}, {'ё', "e"}, {'ж', "zh"}, {'з', "z"}, {'и', "i"},
            {'й', "y"}, {'к', "k"}, {'л', "l"}, {'м', "m"}, {'н', "n"},
            {'о', "o"}, {'п', "p"}, {'р', "r"}, {'с', "s"}, {'т', "t"},
            {'у', "u"}, {'ф', "f"}, {'х', "kh"}, {'ц', "ts"}, {'ч', "ch"},
            {'ш', "sh"}, {'щ', "shch"}, {'ъ', ""}, {'ы', "y"}, {'ь', ""},
            {'э', "e"}, {'ю', "yu"}, {'я', "ya"},
            {'А', "a"}, {'Б', "b"}, {'В', "v"}, {'Г', "g"}, {'Д', "d"},
            {'Е', "e"}, {'Ё', "e"}, {'Ж', "zh"}, {'З', "z"}, {'И', "i"},
            {'Й', "y"}, {'К', "k"}, {'Л', "l"}, {'М', "m"}, {'Н', "n"},
            {'О', "o"}, {'П', "p"}, {'Р', "r"}, {'С', "s"}, {'Т', "t"},
            {'У', "u"}, {'Ф', "f"}, {'Х', "kh"}, {'Ц', "ts"}, {'Ч', "ch"},
            {'Ш', "sh"}, {'Щ', "shch"}, {'Ъ', ""}, {'Ы', "y"}, {'Ь', ""},
            {'Э', "e"}, {'Ю', "yu"}, {'Я', "ya"}
        };

        std::string result;
        for (unsigned char ch : text) {
            auto it = translitMap.find(ch);
            if (it != translitMap.end()) {
                result += it->second;
            }
            else {
                result += ch;
            }
        }
        return result;
    }

    // ============== УДАЛЕНИЕ ДИАКРИТИКИ (немецкий, французский, итальянский, польский) ==============
    std::string removeDiacritics(const std::string& text) {
        static const std::unordered_map<std::string, std::string> diacriticMap = {
            // НЕМЕЦКИЕ
            {"ä", "a"}, {"ö", "o"}, {"ü", "u"}, {"ß", "ss"},
            {"Ä", "A"}, {"Ö", "O"}, {"Ü", "U"},

            // ФРАНЦУЗСКИЕ (и общие для итальянского)
            {"é", "e"}, {"è", "e"}, {"ê", "e"}, {"ë", "e"},
            {"à", "a"}, {"â", "a"}, {"ç", "c"}, {"ô", "o"},
            {"ù", "u"}, {"û", "u"}, {"î", "i"}, {"ï", "i"},
            {"É", "E"}, {"È", "E"}, {"Ê", "E"}, {"Ë", "E"},
            {"À", "A"}, {"Â", "A"}, {"Ç", "C"}, {"Ô", "O"},
            {"Ù", "U"}, {"Û", "U"}, {"Î", "I"}, {"Ï", "I"},

            // ИТАЛЬЯНСКИЕ (дополнительные)
            {"ì", "i"}, {"ò", "o"},
            {"Ì", "I"}, {"Ò", "O"},

            // ПОЛЬСКИЕ
            {"ą", "a"}, {"ć", "c"}, {"ę", "e"}, {"ł", "l"},
            {"ń", "n"}, {"ó", "o"}, {"ś", "s"}, {"ź", "z"}, {"ż", "z"},
            {"Ą", "A"}, {"Ć", "C"}, {"Ę", "E"}, {"Ł", "L"},
            {"Ń", "N"}, {"Ó", "O"}, {"Ś", "S"}, {"Ź", "Z"}, {"Ż", "Z"}
        };

        std::string result;
        size_t i = 0;
        while (i < text.length()) {
            bool replaced = false;
            if (i + 1 < text.length()) {
                std::string twoBytes = text.substr(i, 2);
                auto it = diacriticMap.find(twoBytes);
                if (it != diacriticMap.end()) {
                    result += it->second;
                    i += 2;
                    replaced = true;
                }
            }
            if (!replaced) {
                result += text[i];
                i++;
            }
        }
        return result;
    }

    // ============== НОРМАЛИЗАЦИЯ ТЕКСТА ==============
    std::string normalizeText(const std::string& text) {
        if (text.empty()) return "";

        std::string lowerText;
        for (char c : text) {
            lowerText += std::tolower(static_cast<unsigned char>(c));
        }

        std::string translitText = transliterateRussian(lowerText);
        std::string noDiacritics = removeDiacritics(translitText);

        std::string result;
        for (char c : noDiacritics) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                result += std::tolower(static_cast<unsigned char>(c));
            }
        }
        return result;
    }

    // ============== БИГРАММЫ ==============
    std::map<std::string, int> extractBigrams(const std::string& text) {
        std::map<std::string, int> bigrams;
        if (text.length() < 2) return bigrams;
        for (size_t i = 0; i <= text.length() - 2; ++i) {
            std::string bg = text.substr(i, 2);
            bigrams[bg]++;
        }
        return bigrams;
    }

    // ============== ЧАСТОТЫ → ВЕРОЯТНОСТИ ==============
    std::map<std::string, double> frequenciesToProbabilities(const std::map<std::string, int>& freqs) {
        std::map<std::string, double> probs;
        int total = 0;
        for (const auto& p : freqs) total += p.second;
        if (total == 0) return probs;
        for (const auto& p : freqs) {
            probs[p.first] = static_cast<double>(p.second) / total;
        }
        return probs;
    }

    // ============== ЭТАЛОННЫЕ ПРОФИЛИ ДЛЯ 6 ЯЗЫКОВ ==============
    std::map<std::string, std::map<std::string, double>> loadLanguageProfiles() {
        std::map<std::string, std::map<std::string, double>> profiles;

        // АНГЛИЙСКИЙ
        profiles["english"] = {
            {"th", 0.0278}, {"he", 0.0236}, {"in", 0.0204}, {"er", 0.0189},
            {"an", 0.0173}, {"re", 0.0162}, {"nd", 0.0151}, {"on", 0.0144},
            {"en", 0.0138}, {"at", 0.0132}, {"ou", 0.0127}, {"ed", 0.0122},
            {"ha", 0.0118}, {"to", 0.0114}, {"or", 0.0110}, {"it", 0.0106},
            {"is", 0.0103}, {"hi", 0.0100}, {"es", 0.0097}, {"ng", 0.0094},
            {"ea", 0.0091}, {"ti", 0.0088}, {"se", 0.0085}, {"ic", 0.0082},
            {"al", 0.0079}, {"ar", 0.0076}, {"te", 0.0073}, {"ra", 0.0070},
            {"ro", 0.0067}, {"st", 0.0064}
        };

        // РУССКИЙ (после транслитерации)
        profiles["russian"] = {
            {"st", 0.0265}, {"no", 0.0238}, {"to", 0.0212}, {"na", 0.0195},
            {"en", 0.0181}, {"ov", 0.0168}, {"ko", 0.0156}, {"ro", 0.0147},
            {"po", 0.0139}, {"pr", 0.0132}, {"ra", 0.0125}, {"ve", 0.0119},
            {"ol", 0.0113}, {"ka", 0.0108}, {"od", 0.0103}, {"ni", 0.0098},
            {"ne", 0.0094}, {"ta", 0.0090}, {"vo", 0.0086}, {"or", 0.0082},
            {"al", 0.0078}, {"za", 0.0074}, {"li", 0.0071}, {"va", 0.0068},
            {"re", 0.0065}, {"lo", 0.0062}, {"ch", 0.0059}, {"ya", 0.0056},
            {"tr", 0.0053}, {"ve", 0.0050}
        };

        // НЕМЕЦКИЙ
        profiles["german"] = {
            {"en", 0.0325}, {"er", 0.0312}, {"ch", 0.0281}, {"ei", 0.0254},
            {"ie", 0.0237}, {"de", 0.0223}, {"nd", 0.0211}, {"te", 0.0200},
            {"ge", 0.0190}, {"in", 0.0181}, {"sc", 0.0172}, {"au", 0.0164},
            {"he", 0.0156}, {"ic", 0.0149}, {"be", 0.0142}, {"un", 0.0135},
            {"st", 0.0128}, {"ne", 0.0122}, {"an", 0.0116}, {"re", 0.0110},
            {"ei", 0.0105}, {"ng", 0.0100}, {"se", 0.0095}, {"ie", 0.0091},
            {"ar", 0.0087}, {"el", 0.0083}, {"es", 0.0080}, {"au", 0.0077},
            {"is", 0.0074}, {"nd", 0.0071}
        };

        // ФРАНЦУЗСКИЙ
        profiles["french"] = {
            {"es", 0.0315}, {"en", 0.0298}, {"le", 0.0276}, {"de", 0.0261},
            {"re", 0.0248}, {"nt", 0.0235}, {"on", 0.0224}, {"te", 0.0213},
            {"et", 0.0204}, {"an", 0.0195}, {"ai", 0.0187}, {"ou", 0.0179},
            {"ui", 0.0172}, {"er", 0.0165}, {"ne", 0.0158}, {"la", 0.0152},
            {"me", 0.0146}, {"ce", 0.0140}, {"it", 0.0134}, {"ue", 0.0129},
            {"se", 0.0124}, {"ns", 0.0119}, {"pa", 0.0114}, {"el", 0.0109},
            {"ie", 0.0105}, {"ur", 0.0101}, {"ta", 0.0097}, {"un", 0.0093},
            {"qu", 0.0089}, {"ve", 0.0085}
        };

        // ИТАЛЬЯНСКИЙ
        profiles["italian"] = {
            {"ti", 0.0302}, {"ta", 0.0285}, {"to", 0.0269}, {"an", 0.0254},
            {"on", 0.0240}, {"no", 0.0228}, {"en", 0.0217}, {"re", 0.0207},
            {"ra", 0.0198}, {"ri", 0.0190}, {"ar", 0.0182}, {"am", 0.0175},
            {"te", 0.0168}, {"nt", 0.0161}, {"ia", 0.0155}, {"ic", 0.0149},
            {"io", 0.0143}, {"er", 0.0138}, {"in", 0.0133}, {"al", 0.0128},
            {"co", 0.0123}, {"li", 0.0118}, {"or", 0.0114}, {"tt", 0.0110},
            {"ez", 0.0106}, {"me", 0.0102}, {"ai", 0.0098}, {"le", 0.0094},
            {"at", 0.0090}, {"ma", 0.0086}
        };

        // ПОЛЬСКИЙ
        profiles["polish"] = {
            {"cz", 0.0310}, {"sz", 0.0292}, {"dz", 0.0275}, {"ch", 0.0259},
            {"pr", 0.0244}, {"tr", 0.0230}, {"na", 0.0217}, {"po", 0.0205},
            {"ow", 0.0194}, {"za", 0.0184}, {"ie", 0.0175}, {"er", 0.0167},
            {"on", 0.0159}, {"ny", 0.0152}, {"ec", 0.0145}, {"wa", 0.0139},
            {"lo", 0.0133}, {"yc", 0.0127}, {"go", 0.0122}, {"st", 0.0117},
            {"ki", 0.0112}, {"ko", 0.0108}, {"wi", 0.0104}, {"ci", 0.0100},
            {"ni", 0.0096}, {"cz", 0.0092}, {"zy", 0.0088}, {"sc", 0.0084},
            {"dz", 0.0080}, {"ki", 0.0076}
        };

        return profiles;
    }

} // namespace Utils