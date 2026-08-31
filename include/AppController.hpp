#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include "BrandDetector.hpp"
#include "ScannerEngine.hpp"
#include "CarListModel.hpp"
#include "UpdateManager.hpp"

namespace acbo {

class AppController : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString carsDir READ carsDir WRITE setCarsDir NOTIFY carsDirChanged)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)
    Q_PROPERTY(int scanProgressPercent READ scanProgressPercent NOTIFY scanProgressChanged)
    Q_PROPERTY(QString scanStatusText READ scanStatusText NOTIFY scanStatusTextChanged)
    Q_PROPERTY(QStringList knownBrands READ knownBrands NOTIFY knownBrandsChanged)
    Q_PROPERTY(QStringList knownCountries READ knownCountries NOTIFY knownCountriesChanged)
    Q_PROPERTY(int filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(CarListModel* carModel READ carModel CONSTANT)
    Q_PROPERTY(UpdateManager* updateManager READ updateManager CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override = default;

    [[nodiscard]] QString carsDir() const;
    void setCarsDir(const QString& dir);

    [[nodiscard]] bool isScanning() const;
    [[nodiscard]] int scanProgressPercent() const;
    [[nodiscard]] QString scanStatusText() const;
    [[nodiscard]] QStringList knownBrands() const;
    [[nodiscard]] QStringList knownCountries() const;
    [[nodiscard]] int filterMode() const;
    void setFilterMode(int mode);
    [[nodiscard]] QString searchText() const;
    void setSearchText(const QString& text);
    [[nodiscard]] CarListModel* carModel();
    [[nodiscard]] UpdateManager* updateManager() const { return m_updateManager.get(); }
    [[nodiscard]] QString appVersion() const { return "v1.0.0"; }

    // QML Invocable actions
    Q_INVOKABLE void startScan();
    Q_INVOKABLE void startScanPath(const QString& path);
    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE void applyAllSuggestions();
    Q_INVOKABLE void saveAllPending();
    Q_INVOKABLE void saveAllPendingChanges() { saveAllPending(); }
    Q_INVOKABLE void copyToClipboard(const QString& text);
    Q_INVOKABLE QString detectAssettoCorsaCarsPath() const;
    Q_INVOKABLE QString getCountryForBrand(const QString& brand) const;
    Q_INVOKABLE QString getFlagForCountry(const QString& country) const;
    Q_INVOKABLE QUrl getBadgeForBrand(const QString& brand) const;

    // Updater invocables
    Q_INVOKABLE void checkForUpdates(bool silent = false);

signals:
    void carsDirChanged();
    void isScanningChanged();
    void scanProgressChanged();
    void scanStatusTextChanged();
    void filterModeChanged();
    void searchTextChanged();
    void knownBrandsChanged();
    void knownCountriesChanged();
    void showToast(const QString& type, const QString& title, const QString& message);

private:
    std::shared_ptr<BrandDetector> m_brandDetector;
    std::unique_ptr<ScannerEngine> m_scannerEngine;
    std::unique_ptr<CarListModel> m_carModel;
    std::unique_ptr<UpdateManager> m_updateManager;

    QString m_carsDir;
    bool m_isScanning{false};
    int m_scanProgressPercent{0};
    QString m_scanStatusText;
    int m_filterMode{0};
    QString m_searchText;

    QStringList m_knownBrandsList;
    QStringList m_knownCountriesList;

    void setupConnections();
};

} // namespace acbo
