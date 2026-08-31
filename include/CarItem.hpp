#pragma once

#include <string>
#include <vector>
#include <QString>
#include <QUrl>

namespace acbo {

enum class CarStatus {
    Verified,       // Both brand & country are set and consistent
    AutoDetected,   // Has a suggestion ready to apply
    MissingBrand,   // Brand is missing or unknown
    MissingCountry, // Country is missing or unknown
    Mismatched,     // Country does not match brand origin
    PendingSave     // User or batch has modified fields, not yet written to disk
};

struct CarItem {
    std::string folderName;
    std::string folderPath;
    std::string jsonPath;
    std::string previewPath;
    std::string badgePath;

    // Data parsed from ui/ui_car.json
    std::string name;
    std::string brand;
    std::string country;
    std::string author;
    int year{0};
    std::vector<std::string> tags;

    // Smart auto-detection fields
    std::string suggestedBrand;
    std::string suggestedCountry;
    std::string detectionConfidence; // "High", "Medium", "Low"
    std::string detectionReason;
    bool hasSuggestion{false};

    // Staging / Edit fields
    std::string editedBrand;
    std::string editedCountry;
    bool isPendingSave{false};
    bool isSaved{false};
    bool isValidJson{true};
    std::string parseError;

    // Helper status queries
    [[nodiscard]] bool isBrandMissing() const;
    [[nodiscard]] bool isCountryMissing() const;
    [[nodiscard]] bool isCountryMismatched() const;
    [[nodiscard]] CarStatus status() const;
    [[nodiscard]] QString statusString() const;
    [[nodiscard]] QUrl previewUrl() const;
    [[nodiscard]] QUrl badgeUrl() const;
    [[nodiscard]] QString countryFlag() const;
    [[nodiscard]] static QString getCountryFlag(std::string_view countryName);
    [[nodiscard]] QString tagsString() const;

    void applySuggestion();
    void setCustomBrand(const std::string& newBrand);
    void setCustomCountry(const std::string& newCountry);
    void markSaved();
};

} // namespace acbo
