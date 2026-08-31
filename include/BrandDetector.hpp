#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <optional>
#include "CarItem.hpp"

namespace acbo {

struct DetectionResult {
    std::string brand;
    std::string country;
    std::string confidence; // "High", "Medium", "Low"
    std::string reason;
    bool matched{false};
};

class BrandDetector {
public:
    BrandDetector();

    // Analyzes a CarItem and returns suggestions if missing or mismatched
    [[nodiscard]] DetectionResult detect(const CarItem& car) const;

    // Direct lookup of country by brand name (canonical or synonym)
    [[nodiscard]] std::optional<std::string> getCountryForBrand(std::string_view brand) const;

    // Get list of all known canonical brands (sorted alphabetically)
    [[nodiscard]] const std::vector<std::string>& getKnownBrands() const;

    // Get list of all distinct countries (sorted alphabetically)
    [[nodiscard]] const std::vector<std::string>& getKnownCountries() const;

    // Helper: strip common AC modder prefixes from folder names
    [[nodiscard]] static std::string cleanFolderName(std::string_view folder);

    // Dynamically register a brand, its country, and optional badge path discovered from scanned cars
    void registerDiscoveredBrand(const std::string& brand, const std::string& country, const std::string& badgePath = "");

    // Get badge path for a brand (learned dynamically from cars in memory)
    [[nodiscard]] std::string getBadgeForBrand(std::string_view brand) const;

public:
    struct BrandInfo {
        std::string canonicalName;
        std::string country;
    };

private:
    void initDatabase();
    void loadBrandBadgesFromSystem();
    void addBrand(std::string canonicalBrand, std::string country,
                  std::vector<std::string> brandAliases = {},
                  std::vector<std::string> modelSynonyms = {});

    // Normalized lowercase lookup -> BrandInfo
    std::unordered_map<std::string, BrandInfo> m_brandLookup;
    // Model-name-only synonyms (fallback, lower priority than brand names)
    std::unordered_map<std::string, BrandInfo> m_modelSynonyms;
    // Mod lore alias mapping (lowercase -> canonical brand)
    std::unordered_map<std::string, std::string> m_modLoreAliases;
    // Brand badges discovered dynamically during car scanning (lowercase brand -> badge file path)
    std::unordered_map<std::string, std::string> m_brandBadges;

    // Brand rules sorted by pattern length descending for greedy matching
    std::vector<std::pair<std::string, BrandInfo>> m_sortedBrandRules;
    // Model synonym rules sorted by pattern length descending (fallback)
    std::vector<std::pair<std::string, BrandInfo>> m_sortedModelRules;

    std::vector<std::string> m_allBrandsSorted;
    std::vector<std::string> m_allCountriesSorted;

    // Match only brand names (primary tier)
    [[nodiscard]] std::optional<BrandInfo> matchBrandName(std::string_view text) const;
    // Match model synonyms (fallback tier)
    [[nodiscard]] std::optional<BrandInfo> matchModelName(std::string_view text) const;
    // Match both (brand names first, then model synonyms) — legacy behavior
    [[nodiscard]] std::optional<BrandInfo> matchText(std::string_view text) const;
    [[nodiscard]] std::optional<BrandInfo> matchFuzzy(std::string_view token) const;
    [[nodiscard]] bool isBrandInText(std::string_view brand, std::string_view text) const;
    [[nodiscard]] static int levenshteinDistance(std::string_view s1, std::string_view s2);
};

} // namespace acbo
