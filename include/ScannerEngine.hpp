#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include <atomic>
#include <memory>
#include <filesystem>
#include <QFutureWatcher>
#include "CarItem.hpp"
#include "BrandDetector.hpp"

namespace acbo {

class ScannerEngine : public QObject {
    Q_OBJECT

public:
    explicit ScannerEngine(std::shared_ptr<BrandDetector> detector, QObject* parent = nullptr);
    ~ScannerEngine() override;

    // Starts scanning directory asynchronously
    void startScan(const QString& directoryPath);

    // Requests cooperative cancellation
    void cancelScan();

    [[nodiscard]] bool isScanning() const;

    // Direct synchronous scan for testing and processing
    [[nodiscard]] std::vector<CarItem> doScan(const QString& directoryPath);

signals:
    void scanStarted(int estimatedTotal);
    void scanProgress(int current, int total, const QString& currentCarName);
    void scanFinished(const std::vector<CarItem>& cars);
    void scanFailed(const QString& errorMessage);

private:
    std::shared_ptr<BrandDetector> m_detector;
    std::atomic<bool> m_isScanning{false};
    std::atomic<bool> m_cancelRequested{false};
    QFutureWatcher<std::vector<CarItem>> m_watcher;

    static bool parseCarJson(const std::filesystem::path& jsonPath, CarItem& car);
};

} // namespace acbo
