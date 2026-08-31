#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include "CarItem.hpp"

namespace acbo {

struct WriteResult {
    bool success{false};
    std::string errorMessage;
    std::string backupPath;
};

class JsonWriter {
public:
    // Safely saves edited brand and country into ui_car.json with .bak backup
    static WriteResult saveCar(CarItem& car);

    // Batch save multiple cars; returns number of successful saves
    static size_t batchSave(std::vector<CarItem*>& cars, std::vector<std::string>& outErrors);

    // Creates a .bak backup file if one does not already exist
    static bool ensureBackup(const std::filesystem::path& jsonPath, std::string& outBackupPath, std::string& outError);

    // Sanitizes JSON string escaping raw CR/LF inside string literals
    static std::string sanitizeJsonContent(const std::string& input);

    // Resilient fallback: replaces or inserts "brand" and "country" directly in raw text without failing on corrupt JSON
    static bool updateBrandAndCountryInRawText(std::string& content, const std::string& newBrand, const std::string& newCountry);

private:
    // Cleans BOM and reads full file content into string
    static bool readFileWithoutBom(const std::filesystem::path& filePath, std::string& outContent, std::string& outError);
    
    // Atomically replaces destination file with temporary file
    static bool atomicReplaceFile(const std::filesystem::path& tempPath, const std::filesystem::path& destPath, std::string& outError);
};

} // namespace acbo
