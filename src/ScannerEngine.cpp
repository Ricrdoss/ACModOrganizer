#include "ScannerEngine.hpp"
#include "JsonWriter.hpp"
#include "Logger.hpp"
#include <QtConcurrent/QtConcurrent>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <regex>

namespace acbo {

using json = nlohmann::json;

namespace {

// Safely converts std::filesystem::path to UTF-8 std::string on Windows without throwing code-page exceptions
inline std::string pathToUtf8(const std::filesystem::path& p) {
    return QString::fromStdWString(p.native()).toStdString();
}

inline std::filesystem::path utf8ToPath(const std::string& s) {
    return std::filesystem::path(QString::fromStdString(s).toStdWString());
}

std::string findCarPreview(const std::filesystem::path& carFolder) {
    static const std::vector<std::string> exts = {
        ".png", ".jpg", ".jpeg", ".PNG", ".JPG", ".JPEG"
    };

    std::error_code ec;

    // 1. Direct ui preview (e.g. ui/preview.png)
    std::filesystem::path uiDir = carFolder / "ui";
    if (std::filesystem::exists(uiDir, ec)) {
        for (const auto& ext : exts) {
            std::filesystem::path p = uiDir / ("preview" + ext);
            if (std::filesystem::exists(p, ec) && !ec) {
                return pathToUtf8(p);
            }
        }
    }

    // 2. Look in skins subdirectories (e.g. skins/<skin_name>/preview.png)
    std::filesystem::path skinsDir = carFolder / "skins";
    if (std::filesystem::exists(skinsDir, ec) && std::filesystem::is_directory(skinsDir, ec)) {
        for (const auto& skinEntry : std::filesystem::directory_iterator(skinsDir, ec)) {
            if (ec) break;
            if (skinEntry.is_directory(ec)) {
                for (const auto& ext : exts) {
                    std::filesystem::path p = skinEntry.path() / ("preview" + ext);
                    if (std::filesystem::exists(p, ec) && !ec) {
                        return pathToUtf8(p);
                    }
                }
            }
        }
    }

    // 3. Root car directory preview (e.g. preview.png)
    for (const auto& ext : exts) {
        std::filesystem::path p = carFolder / ("preview" + ext);
        if (std::filesystem::exists(p, ec) && !ec) {
            return pathToUtf8(p);
        }
    }

    // 4. Fallback to badge image if present
    std::filesystem::path badge = uiDir / "badge.png";
    if (std::filesystem::exists(badge, ec) && !ec) {
        return pathToUtf8(badge);
    }

    return "";
}

std::string findCarBadge(const std::filesystem::path& carFolder) {
    static const std::vector<std::string> exts = {
        ".png", ".PNG", ".jpg", ".jpeg", ".JPG", ".JPEG"
    };
    std::error_code ec;

    std::filesystem::path uiDir = carFolder / "ui";
    if (std::filesystem::exists(uiDir, ec)) {
        for (const auto& ext : exts) {
            std::filesystem::path p = uiDir / ("badge" + ext);
            if (std::filesystem::exists(p, ec) && !ec) {
                return pathToUtf8(p);
            }
        }
        for (const auto& ext : exts) {
            std::filesystem::path p = uiDir / ("logo" + ext);
            if (std::filesystem::exists(p, ec) && !ec) {
                return pathToUtf8(p);
            }
        }
    }

    for (const auto& ext : exts) {
        std::filesystem::path p = carFolder / ("badge" + ext);
        if (std::filesystem::exists(p, ec) && !ec) {
            return pathToUtf8(p);
        }
    }

    return "";
}

} // namespace

ScannerEngine::ScannerEngine(std::shared_ptr<BrandDetector> detector, QObject* parent)
    : QObject(parent), m_detector(std::move(detector)) {
    connect(&m_watcher, &QFutureWatcher<std::vector<CarItem>>::finished, this, [this]() {
        m_isScanning = false;
        if (!m_cancelRequested) {
            emit scanFinished(m_watcher.result());
        }
    });
}

ScannerEngine::~ScannerEngine() {
    cancelScan();
    if (m_watcher.isRunning()) {
        m_watcher.waitForFinished();
    }
}

bool ScannerEngine::isScanning() const {
    return m_isScanning;
}

void ScannerEngine::cancelScan() {
    if (m_isScanning) {
        m_cancelRequested = true;
        m_watcher.cancel();
    }
}

void ScannerEngine::startScan(const QString& directoryPath) {
    if (m_isScanning) {
        cancelScan();
        m_watcher.waitForFinished();
    }

    m_isScanning = true;
    m_cancelRequested = false;

    QFuture<std::vector<CarItem>> future = QtConcurrent::run([this, directoryPath]() {
        return doScan(directoryPath);
    });

    m_watcher.setFuture(future);
}

bool ScannerEngine::parseCarJson(const std::filesystem::path& jsonPath, CarItem& car) {
    std::ifstream file(jsonPath, std::ios::binary);
    if (!file.is_open()) {
        car.isValidJson = false;
        car.parseError = "Could not open file";
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Strip BOM if present
    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        content = content.substr(3);
    }

    std::string sanitized = JsonWriter::sanitizeJsonContent(content);
    json j = json::parse(sanitized, nullptr, false, true);
    if (j.is_discarded()) {
        // Resilient regex extraction fallback for malformed mod JSONs
        std::regex nameRegex(R"RAW("name"\s*:\s*"((?:[^"\\]|\\.)*)")RAW");
        std::regex brandRegex(R"RAW("brand"\s*:\s*"((?:[^"\\]|\\.)*)")RAW");
        std::regex countryRegex(R"RAW("country"\s*:\s*"((?:[^"\\]|\\.)*)")RAW");
        std::regex authorRegex(R"RAW("author"\s*:\s*"((?:[^"\\]|\\.)*)")RAW");
        std::regex yearRegex(R"RAW("year"\s*:\s*"?([0-9]{4})"?)RAW");
        std::smatch m;

        if (std::regex_search(content, m, nameRegex)) car.name = m[1].str();
        else car.name = car.folderName;
        if (std::regex_search(content, m, brandRegex)) car.brand = m[1].str();
        if (std::regex_search(content, m, countryRegex)) car.country = m[1].str();
        if (std::regex_search(content, m, authorRegex)) car.author = m[1].str();
        if (std::regex_search(content, m, yearRegex)) {
            try { car.year = std::stoi(m[1].str()); } catch (...) { car.year = 0; }
        }

        car.isValidJson = true;
        return true;
    }

    // Safely extract string / number fields
    if (j.contains("name") && j["name"].is_string()) {
        car.name = j["name"].get<std::string>();
    } else {
        car.name = car.folderName;
    }

    if (j.contains("brand") && j["brand"].is_string()) {
        car.brand = j["brand"].get<std::string>();
    }

    if (j.contains("country") && j["country"].is_string()) {
        car.country = j["country"].get<std::string>();
    }

    if (j.contains("author") && j["author"].is_string()) {
        car.author = j["author"].get<std::string>();
    }

    if (j.contains("year")) {
        if (j["year"].is_number_integer()) {
            car.year = j["year"].get<int>();
        } else if (j["year"].is_string()) {
            try {
                car.year = std::stoi(j["year"].get<std::string>());
            } catch (...) {
                car.year = 0;
            }
        }
    }

    if (j.contains("tags") && j["tags"].is_array()) {
        for (const auto& tag : j["tags"]) {
            if (tag.is_string()) {
                car.tags.push_back(tag.get<std::string>());
            }
        }
    }

    return true;
}

std::vector<CarItem> ScannerEngine::doScan(const QString& directoryPath) {
    std::vector<CarItem> results;
    std::error_code ec;
    std::filesystem::path carsDir(directoryPath.toStdWString());

    if (!std::filesystem::exists(carsDir, ec) || !std::filesystem::is_directory(carsDir, ec)) {
        const QString err = "Specified directory does not exist or is not a directory: " + directoryPath;
        LOG_ERROR(err.toStdString());
        emit scanFailed(err);
        return results;
    }

    // Pass 1: Count candidate car subdirectories safely
    std::vector<std::filesystem::path> subdirs;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(carsDir, ec)) {
            if (m_cancelRequested || ec) break;
            if (entry.is_directory(ec)) {
                subdirs.push_back(entry.path());
            }
        }
    } catch (const std::exception& ex) {
        const QString err = QString("Filesystem error reading directory: ") + ex.what();
        LOG_ERROR(err.toStdString());
        emit scanFailed(err);
        return results;
    }

    const int total = static_cast<int>(subdirs.size());
    emit scanStarted(total);
    LOG_INFO("Discovered " + std::to_string(total) + " candidate car folders in " + directoryPath.toStdString());

    // Pass 2: Inspect each folder
    results.reserve(subdirs.size());
    int current = 0;

    for (const auto& carFolder : subdirs) {
        if (m_cancelRequested) break;
        current++;

        try {
            CarItem car;
            car.folderName = pathToUtf8(carFolder.filename());
            car.folderPath = pathToUtf8(carFolder);

            // Check ui subdirectory and parse JSON
            std::filesystem::path uiDir = carFolder / "ui";
            std::filesystem::path jsonPath = uiDir / "ui_car.json";

            if (std::filesystem::exists(jsonPath, ec) && !ec) {
                car.jsonPath = pathToUtf8(jsonPath);
                parseCarJson(jsonPath, car);
            } else {
                car.name = car.folderName;
                car.brand = "";
                car.country = "";
                car.isValidJson = false;
                car.parseError = "Missing ui/ui_car.json";
            }

            // Comprehensive search for preview thumbnail (ui, skins, root, etc.)
            car.previewPath = findCarPreview(carFolder);
            car.badgePath = findCarBadge(carFolder);

            // Initialize staging edit values
            car.editedBrand = car.brand;
            car.editedCountry = car.country;

            // If car already has a valid brand, auto-populate origin country in memory without listing as a problem!
            if (!car.isBrandMissing() && car.isCountryMissing() && m_detector) {
                auto countryOpt = m_detector->getCountryForBrand(car.brand);
                if (countryOpt.has_value()) {
                    car.country = *countryOpt;
                    car.editedCountry = *countryOpt;
                }
            }

            // Perform smart brand and country detection
            if (m_detector) {
                DetectionResult det = m_detector->detect(car);
                if (det.matched) {
                    car.suggestedBrand = det.brand;
                    car.suggestedCountry = det.country;
                    car.detectionConfidence = det.confidence;
                    car.detectionReason = det.reason;
                    car.hasSuggestion = true;

                    // Auto-stage suggestions so the card immediately reflects the detected brand and country!
                    car.editedBrand = det.brand;
                    car.editedCountry = det.country;
                    car.isPendingSave = true;
                } else if (car.isBrandMissing()) {
                    // When no brand or model was detected in name or files, mark as "Brand Not Found"
                    car.editedBrand = "Brand Not Found";
                    if (car.isCountryMissing()) {
                        car.editedCountry = "Unknown";
                    }
                    car.isPendingSave = false;
                }
            } else if (car.isBrandMissing()) {
                car.editedBrand = "Brand Not Found";
                if (car.isCountryMissing()) {
                    car.editedCountry = "Unknown";
                }
                car.isPendingSave = false;
            }

            // Dynamically register discovered brand and badge into BrandDetector
            if (m_detector && !car.isBrandMissing()) {
                m_detector->registerDiscoveredBrand(car.brand, car.country, car.badgePath);
            }

            results.push_back(std::move(car));
        } catch (const std::exception& ex) {
            LOG_WARN("Error inspecting car folder: " + std::string(ex.what()));
        } catch (...) {
            LOG_WARN("Unknown error inspecting car folder.");
        }

        if (current % 25 == 0 || current == total) {
            QString displayName = results.empty() ? "" : QString::fromStdString(results.back().name);
            emit scanProgress(current, total, displayName);
        }
    }

    // Dynamic Badge Resolution: If a car mod did not bundle badge.png, inherit learned badge for its brand
    if (m_detector) {
        for (auto& car : results) {
            if (car.badgePath.empty() && !car.isBrandMissing()) {
                car.badgePath = m_detector->getBadgeForBrand(car.brand);
            }
            if (car.badgePath.empty() && !car.editedBrand.empty() && car.editedBrand != "Brand Not Found") {
                car.badgePath = m_detector->getBadgeForBrand(car.editedBrand);
            }
            if (car.badgePath.empty() && !car.suggestedBrand.empty()) {
                car.badgePath = m_detector->getBadgeForBrand(car.suggestedBrand);
            }
        }
    }

    LOG_SUCCESS("Scan completed. Loaded and inspected " + std::to_string(results.size()) + " cars.");
    return results;
}

} // namespace acbo
