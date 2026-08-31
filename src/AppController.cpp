#include "AppController.hpp"
#include "Logger.hpp"
#include <QDir>
#include <QGuiApplication>
#include <QClipboard>
#include <filesystem>
#include <QStandardPaths>
#include <QProcess>
#include <QDesktopServices>

namespace acbo {

AppController::AppController(QObject* parent)
    : QObject(parent),
      m_brandDetector(std::make_shared<BrandDetector>()),
      m_scannerEngine(std::make_unique<ScannerEngine>(m_brandDetector, this)),
      m_carModel(std::make_unique<CarListModel>(this)),
      m_updateManager(std::make_unique<UpdateManager>(this)) {

    // Pre-populate known brands and countries for QML autocomplete
    for (const auto& brand : m_brandDetector->getKnownBrands()) {
        m_knownBrandsList.append(QString::fromStdString(brand));
    }
    for (const auto& country : m_brandDetector->getKnownCountries()) {
        m_knownCountriesList.append(QString::fromStdString(country));
    }

    // Check if we can auto-detect AC path
    QString detected = detectAssettoCorsaCarsPath();
    if (!detected.isEmpty()) {
        m_carsDir = detected;
    }

    setupConnections();

    // Check for updates silently on startup (after a slight delay in background)
    QMetaObject::invokeMethod(this, [this]() {
        if (m_updateManager) {
            m_updateManager->checkForUpdates(true);
        }
    }, Qt::QueuedConnection);
}

void AppController::setupConnections() {
    connect(m_scannerEngine.get(), &ScannerEngine::scanStarted, this, [this](int estimatedTotal) {
        m_isScanning = true;
        m_scanProgressPercent = 0;
        m_scanStatusText = QString("Scanning %1 car directories...").arg(estimatedTotal);
        emit isScanningChanged();
        emit scanProgressChanged();
        emit scanStatusTextChanged();
    });

    connect(m_scannerEngine.get(), &ScannerEngine::scanProgress, this, [this](int current, int total, const QString& carName) {
        m_scanProgressPercent = (total > 0) ? static_cast<int>((current * 100.0) / total) : 0;
        m_scanStatusText = QString("Analyzing [%1/%2]: %3").arg(current).arg(total).arg(carName);
        emit scanProgressChanged();
        emit scanStatusTextChanged();
    });

    connect(m_scannerEngine.get(), &ScannerEngine::scanFinished, this, [this](const std::vector<CarItem>& cars) {
        m_isScanning = false;
        m_scanProgressPercent = 100;
        m_scanStatusText = QString("Successfully loaded %1 cars").arg(cars.size());
        m_carModel->setCars(cars);

        // Dynamically discover and register any brands and countries from scanned cars
        bool brandsChanged = false;
        bool countriesChanged = false;

        for (const auto& brand : m_brandDetector->getKnownBrands()) {
            QString b = QString::fromStdString(brand);
            if (!m_knownBrandsList.contains(b, Qt::CaseInsensitive)) {
                m_knownBrandsList.append(b);
                brandsChanged = true;
            }
        }
        for (const auto& country : m_brandDetector->getKnownCountries()) {
            QString c = QString::fromStdString(country);
            if (!m_knownCountriesList.contains(c, Qt::CaseInsensitive)) {
                m_knownCountriesList.append(c);
                countriesChanged = true;
            }
        }

        if (brandsChanged) {
            m_knownBrandsList.sort(Qt::CaseInsensitive);
            emit knownBrandsChanged();
        }
        if (countriesChanged) {
            m_knownCountriesList.sort(Qt::CaseInsensitive);
            emit knownCountriesChanged();
        }

        emit isScanningChanged();
        emit scanProgressChanged();
        emit scanStatusTextChanged();
        emit showToast("success", "Scan Complete", QString("Loaded %1 cars (%2 verified, %3 suggestions, %4 missing)").arg(cars.size()).arg(m_carModel->verifiedCount()).arg(m_carModel->detectedCount()).arg(m_carModel->missingCount()));
    });

    // Update Manager error toast connection
    connect(m_updateManager.get(), &UpdateManager::updateError, this, [this](const QString& err) {
        emit showToast("warning", "Update Check", err);
    });

    connect(m_updateManager.get(), &UpdateManager::updateCheckCompleted, this, [this](bool hasUpdate, const QString& ver) {
        if (!hasUpdate) {
            emit showToast("info", "Up to Date", QString("You are running the latest version of ACBO (%1).").arg(appVersion()));
        } else {
            emit showToast("info", "Update Available", QString("Version %1 is now available!").arg(ver));
        }
    });
}

QString AppController::carsDir() const {
    return m_carsDir;
}

void AppController::setCarsDir(const QString& dir) {
    QString cleanDir = dir;
    if (cleanDir.startsWith("file:///")) {
        cleanDir = QUrl(cleanDir).toLocalFile();
    }
    if (m_carsDir != cleanDir) {
        m_carsDir = cleanDir;
        emit carsDirChanged();
    }
}

bool AppController::isScanning() const {
    return m_isScanning;
}

int AppController::scanProgressPercent() const {
    return m_scanProgressPercent;
}

QString AppController::scanStatusText() const {
    return m_scanStatusText;
}

QStringList AppController::knownBrands() const {
    return m_knownBrandsList;
}

QStringList AppController::knownCountries() const {
    return m_knownCountriesList;
}

int AppController::filterMode() const {
    return m_filterMode;
}

void AppController::setFilterMode(int mode) {
    if (m_filterMode != mode) {
        m_filterMode = mode;
        m_carModel->setFilterMode(static_cast<FilterMode>(mode));
        emit filterModeChanged();
    }
}

QString AppController::searchText() const {
    return m_searchText;
}

void AppController::setSearchText(const QString& text) {
    if (m_searchText != text) {
        m_searchText = text;
        m_carModel->setSearchFilter(text);
        emit searchTextChanged();
    }
}

CarListModel* AppController::carModel() {
    return m_carModel.get();
}

void AppController::startScan() {
    if (m_carsDir.trimmed().isEmpty()) {
        emit showToast("error", "Invalid Path", "Please select a valid Assetto Corsa content/cars directory.");
        return;
    }
    startScanPath(m_carsDir.trimmed());
}

void AppController::startScanPath(const QString& path) {
    setCarsDir(path);
    m_scannerEngine->startScan(path);
}

void AppController::cancelScan() {
    m_scannerEngine->cancelScan();
}

void AppController::applyAllSuggestions() {
    const int count = m_carModel->detectedCount();
    if (count == 0) {
        emit showToast("info", "No Suggestions", "There are no pending auto-detected suggestions to apply.");
        return;
    }
    m_carModel->applyAllSuggestions();
    emit showToast("success", "Suggestions Applied", QString("Applied %1 auto-detections. Review and click 'Save All Pending' to write to disk.").arg(count));
}

void AppController::saveAllPending() {
    const int count = m_carModel->pendingCount();
    if (count == 0) {
        emit showToast("info", "No Changes", "There are no pending changes to save.");
        return;
    }
    const int saved = m_carModel->saveAllPending();
    emit showToast("success", "Saved Changes", QString("Successfully updated %1 car JSON files with backups.").arg(saved));
}

void AppController::copyToClipboard(const QString& text) {
    if (auto* clipboard = QGuiApplication::clipboard()) {
        clipboard->setText(text);
        emit showToast("info", "Copied", QString("'%1' copied to clipboard").arg(text));
    }
}

QString AppController::detectAssettoCorsaCarsPath() const {
    static const std::vector<std::string> candidatePaths = {
        "C:/Program Files (x86)/Steam/steamapps/common/assettocorsa/content/cars",
        "C:/Program Files/Steam/steamapps/common/assettocorsa/content/cars",
        "D:/SteamLibrary/steamapps/common/assettocorsa/content/cars",
        "D:/Games/SteamLibrary/steamapps/common/assettocorsa/content/cars",
        "E:/SteamLibrary/steamapps/common/assettocorsa/content/cars",
        "E:/Games/SteamLibrary/steamapps/common/assettocorsa/content/cars",
        "F:/SteamLibrary/steamapps/common/assettocorsa/content/cars",
        "G:/SteamLibrary/steamapps/common/assettocorsa/content/cars"
    };

    for (const auto& path : candidatePaths) {
        if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
            return QString::fromStdString(path);
        }
    }
    return QString();
}

QString AppController::getCountryForBrand(const QString& brand) const {
    if (m_brandDetector && !brand.isEmpty()) {
        auto c = m_brandDetector->getCountryForBrand(brand.toStdString());
        if (c.has_value()) {
            return QString::fromStdString(*c);
        }
    }
    return QString();
}

QString AppController::getFlagForCountry(const QString& country) const {
    return CarItem::getCountryFlag(country.toStdString());
}

QUrl AppController::getBadgeForBrand(const QString& brand) const {
    if (m_brandDetector && !brand.isEmpty()) {
        std::string badge = m_brandDetector->getBadgeForBrand(brand.toStdString());
        if (!badge.empty()) {
            return QUrl::fromLocalFile(QString::fromStdString(badge));
        }
    }
    return QUrl();
}

void AppController::checkForUpdates(bool silent) {
    if (m_updateManager) {
        m_updateManager->checkForUpdates(silent);
    }
}

} // namespace acbo
