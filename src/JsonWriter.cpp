#include "JsonWriter.hpp"
#include "Logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <system_error>
#include <regex>
#include <QString>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace acbo {

using json = nlohmann::json;

namespace {

inline std::filesystem::path utf8ToPath(const std::string& s) {
    return std::filesystem::path(QString::fromStdString(s).toStdWString());
}

inline std::string pathToUtf8(const std::filesystem::path& p) {
    return QString::fromStdWString(p.native()).toStdString();
}

} // namespace

bool JsonWriter::readFileWithoutBom(const std::filesystem::path& filePath, std::string& outContent, std::string& outError) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        outError = "Failed to open file for reading: " + pathToUtf8(filePath);
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Check and strip UTF-8 BOM (\xEF\xBB\xBF)
    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        content = content.substr(3);
    }

    outContent = std::move(content);
    return true;
}

bool JsonWriter::ensureBackup(const std::filesystem::path& jsonPath, std::string& outBackupPath, std::string& outError) {
    try {
        std::filesystem::path backupPath = jsonPath;
        backupPath += ".bak";
        outBackupPath = pathToUtf8(backupPath);

        // Create backup only if it does not already exist (preserving true pristine original)
        std::error_code ec;
        if (!std::filesystem::exists(backupPath, ec)) {
            std::filesystem::copy_file(jsonPath, backupPath, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec) {
                outError = "Failed to create backup at '" + pathToUtf8(backupPath) + "': " + ec.message();
                return false;
            }
            LOG_INFO("Created original backup file: " + pathToUtf8(backupPath));
        }
        return true;
    } catch (const std::exception& ex) {
        outError = std::string("Backup exception: ") + ex.what();
        return false;
    }
}

bool JsonWriter::atomicReplaceFile(const std::filesystem::path& tempPath, const std::filesystem::path& destPath, std::string& outError) {
#if defined(_WIN32) || defined(_WIN64)
    std::wstring wTemp = tempPath.native();
    std::wstring wDest = destPath.native();

    if (!MoveFileExW(wTemp.c_str(), wDest.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DWORD err = GetLastError();
        std::error_code ec;
        std::filesystem::remove(tempPath, ec);
        outError = "MoveFileEx failed with Windows error code: " + std::to_string(err);
        return false;
    }
#else
    std::error_code ec;
    std::filesystem::rename(tempPath, destPath, ec);
    if (ec) {
        std::filesystem::remove(tempPath, ec);
        outError = "Filesystem rename failed: " + ec.message();
        return false;
    }
#endif
    return true;
}

std::string JsonWriter::sanitizeJsonContent(const std::string& input) {
    std::string output;
    output.reserve(input.size() + 128);

    bool inString = false;
    bool escaped = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (inString) {
            if (escaped) {
                escaped = false;
                output.push_back(c);
            } else if (c == '\\') {
                escaped = true;
                output.push_back(c);
            } else if (c == '"') {
                inString = false;
                output.push_back(c);
            } else if (c == '\r') {
                // Escape raw carriage return inside string literal
                output += "\\r";
            } else if (c == '\n') {
                // Escape raw line feed inside string literal
                output += "\\n";
            } else if (c == '\t') {
                output += "\\t";
            } else if (static_cast<unsigned char>(c) < 0x20) {
                // Strip invalid control characters
            } else {
                output.push_back(c);
            }
        } else {
            if (c == '"') {
                inString = true;
                output.push_back(c);
            } else {
                output.push_back(c);
            }
        }
    }
    return output;
}

static std::string escapeForJson(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else res += c;
    }
    return res;
}

bool JsonWriter::updateBrandAndCountryInRawText(std::string& content, const std::string& newBrand, const std::string& newCountry) {
    try {
        std::string escapedBrand = escapeForJson(newBrand);
        std::string escapedCountry = escapeForJson(newCountry);

        // 1. Update "brand": "..."
        std::regex brandRegex(R"RAW(("brand"\s*:\s*)"(?:[^"\\]|\\.)*("))RAW");
        if (std::regex_search(content, brandRegex)) {
            content = std::regex_replace(content, brandRegex, "$1" + escapedBrand + "$2");
        } else {
            // If "brand" key does not exist, insert before last '}'
            size_t lastBrace = content.rfind('}');
            if (lastBrace != std::string::npos) {
                std::string toInsert = ",\n    \"brand\": \"" + escapedBrand + "\"";
                content.insert(lastBrace, toInsert);
            }
        }

        // 2. Update "country": "..."
        std::regex countryRegex(R"RAW(("country"\s*:\s*)"(?:[^"\\]|\\.)*("))RAW");
        if (std::regex_search(content, countryRegex)) {
            content = std::regex_replace(content, countryRegex, "$1" + escapedCountry + "$2");
        } else {
            // If "country" key does not exist, insert before last '}'
            size_t lastBrace = content.rfind('}');
            if (lastBrace != std::string::npos) {
                std::string toInsert = ",\n    \"country\": \"" + escapedCountry + "\"";
                content.insert(lastBrace, toInsert);
            }
        }
        return true;
    } catch (const std::exception& ex) {
        LOG_ERROR(std::string("Raw text update exception: ") + ex.what());
        return false;
    }
}

WriteResult JsonWriter::saveCar(CarItem& car) {
    WriteResult result;

    std::filesystem::path jsonPath;
    if (!car.jsonPath.empty()) {
        jsonPath = utf8ToPath(car.jsonPath);
    } else if (!car.folderPath.empty()) {
        std::filesystem::path carDir = utf8ToPath(car.folderPath);
        std::filesystem::path uiDir = carDir / "ui";
        std::error_code ec;
        std::filesystem::create_directories(uiDir, ec);
        jsonPath = uiDir / "ui_car.json";
        car.jsonPath = pathToUtf8(jsonPath);
    } else {
        result.errorMessage = "No ui_car.json or folder path associated with car: " + car.folderName;
        LOG_ERROR(result.errorMessage);
        return result;
    }

    std::error_code ec;
    // If ui_car.json does not exist yet (e.g. 2015_season), automatically generate and initialize it!
    if (!std::filesystem::exists(jsonPath, ec)) {
        json j;
        j["name"] = car.name.empty() ? car.folderName : car.name;
        j["brand"] = car.editedBrand;
        j["country"] = car.editedCountry;
        j["description"] = "";
        j["tags"] = json::array();
        j["year"] = car.year > 0 ? car.year : 2020;

        std::string serialized;
        try {
            serialized = j.dump(4) + "\n";
        } catch (...) {
            serialized = "{\n    \"name\": \"" + (car.name.empty() ? car.folderName : car.name) + "\",\n    \"brand\": \"" + car.editedBrand + "\",\n    \"country\": \"" + car.editedCountry + "\"\n}\n";
        }

        std::filesystem::create_directories(jsonPath.parent_path(), ec);
        std::ofstream file(jsonPath, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            result.errorMessage = "Failed to create new ui_car.json: " + pathToUtf8(jsonPath);
            LOG_ERROR(result.errorMessage);
            return result;
        }
        file.write(serialized.data(), static_cast<std::streamsize>(serialized.size()));
        file.close();

        car.brand = car.editedBrand;
        car.country = car.editedCountry;
        car.isPendingSave = false;
        car.isSaved = true;
        car.hasSuggestion = false;
        result.success = true;
        LOG_SUCCESS("Created and populated new ui_car.json for '" + car.folderName + "' -> Brand: '" + car.brand + "', Country: '" + car.country + "'");
        return result;
    }

    // Step 1: Ensure pristine original backup
    std::string backupPath;
    std::string backupError;
    if (!ensureBackup(jsonPath, backupPath, backupError)) {
        result.errorMessage = "Failed to safeguard backup before write: " + backupError;
        LOG_ERROR(result.errorMessage);
        return result;
    }
    result.backupPath = backupPath;

    // Step 2: Read original JSON
    std::string originalContent;
    std::string readError;
    if (!readFileWithoutBom(jsonPath, originalContent, readError)) {
        result.errorMessage = readError;
        LOG_ERROR(result.errorMessage);
        return result;
    }

    // Step 3: Try parsing sanitized JSON with nlohmann::json
    std::string serialized;
    bool parsedSuccessfully = false;

    try {
        std::string sanitized = sanitizeJsonContent(originalContent);
        json j = json::parse(sanitized, nullptr, true, true);
        j["brand"] = car.editedBrand;
        j["country"] = car.editedCountry;
        serialized = j.dump(4) + "\n";
        parsedSuccessfully = true;
    } catch (const std::exception& ex) {
        LOG_WARN(std::string("Standard JSON parsing failed on '") + car.folderName + "': " + ex.what() + ". Using resilient raw text updater.");
    }

    // Step 4: Resilient Fallback to direct raw text editing if JSON structure had syntax errors
    if (!parsedSuccessfully) {
        std::string rawContent = originalContent;
        if (!updateBrandAndCountryInRawText(rawContent, car.editedBrand, car.editedCountry)) {
            result.errorMessage = "Failed to update brand and country in file: " + car.jsonPath;
            LOG_ERROR(result.errorMessage);
            return result;
        }
        serialized = rawContent;
    }

    // Step 6: Atomic file write via temporary file
    std::filesystem::path tempPath = jsonPath;
    tempPath += ".tmp";
    {
        std::ofstream file(tempPath, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            result.errorMessage = "Failed to open temp file for writing: " + pathToUtf8(tempPath);
            LOG_ERROR(result.errorMessage);
            return result;
        }
        file.write(serialized.data(), static_cast<std::streamsize>(serialized.size()));
        file.flush();
        if (!file.good()) {
            result.errorMessage = "Write error during flush to temp file: " + pathToUtf8(tempPath);
            LOG_ERROR(result.errorMessage);
            return result;
        }
        file.close();
    }

    std::string replaceError;
    if (!atomicReplaceFile(tempPath, jsonPath, replaceError)) {
        result.errorMessage = replaceError;
        LOG_ERROR(result.errorMessage);
        return result;
    }

    // Step 7: Update in-memory car model state
    car.brand = car.editedBrand;
    car.country = car.editedCountry;
    car.isPendingSave = false;
    car.isSaved = true;
    car.hasSuggestion = false;

    result.success = true;
    LOG_SUCCESS("Successfully updated ui_car.json for '" + car.folderName + "' -> Brand: '" + car.brand + "', Country: '" + car.country + "'");
    return result;
}

size_t JsonWriter::batchSave(std::vector<CarItem*>& cars, std::vector<std::string>& outErrors) {
    size_t successCount = 0;
    outErrors.clear();

    for (auto* car : cars) {
        if (!car || !car->isPendingSave) continue;

        WriteResult res = saveCar(*car);
        if (res.success) {
            successCount++;
        } else {
            outErrors.push_back(car->folderName + ": " + res.errorMessage);
        }
    }

    return successCount;
}

} // namespace acbo
