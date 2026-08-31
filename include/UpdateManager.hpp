#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

namespace acbo {

class UpdateManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isChecking READ isChecking NOTIFY isCheckingChanged)
    Q_PROPERTY(bool isDownloading READ isDownloading NOTIFY isDownloadingChanged)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateAvailableChanged)
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateDetailsChanged)
    Q_PROPERTY(QString releaseTitle READ releaseTitle NOTIFY updateDetailsChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY updateDetailsChanged)
    Q_PROPERTY(double downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
    Q_PROPERTY(QString downloadStatusText READ downloadStatusText NOTIFY downloadStatusTextChanged)

public:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override = default;

    static QString appVersion() { return "1.0.0"; }
    static bool isVersionNewer(const QString& current, const QString& latest);

    [[nodiscard]] bool isChecking() const { return m_isChecking; }
    [[nodiscard]] bool isDownloading() const { return m_isDownloading; }
    [[nodiscard]] bool updateAvailable() const { return m_updateAvailable; }
    [[nodiscard]] QString currentVersion() const { return appVersion(); }
    [[nodiscard]] QString latestVersion() const { return m_latestVersion; }
    [[nodiscard]] QString releaseTitle() const { return m_releaseTitle; }
    [[nodiscard]] QString releaseNotes() const { return m_releaseNotes; }
    [[nodiscard]] double downloadProgress() const { return m_downloadProgress; }
    [[nodiscard]] QString downloadStatusText() const { return m_downloadStatusText; }

    Q_INVOKABLE void checkForUpdates(bool silent = false);
    Q_INVOKABLE void startDownloadAndInstall();
    Q_INVOKABLE void cancelDownload();

signals:
    void isCheckingChanged();
    void isDownloadingChanged();
    void updateAvailableChanged();
    void updateDetailsChanged();
    void downloadProgressChanged();
    void downloadStatusTextChanged();
    void updateCheckCompleted(bool hasUpdate, const QString& version);
    void updateError(const QString& message);

private slots:
    void onCheckFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    void applyUpdateAndRestart(const QString& downloadedFilePath);

    QNetworkAccessManager m_networkManager;
    QNetworkReply* m_currentReply = nullptr;
    QFile* m_downloadFile = nullptr;

    bool m_isChecking = false;
    bool m_isDownloading = false;
    bool m_updateAvailable = false;
    bool m_silentCheck = false;

    QString m_latestVersion;
    QString m_releaseTitle;
    QString m_releaseNotes;
    QUrl m_downloadUrl;
    qint64 m_assetSize = 0;

    double m_downloadProgress = 0.0;
    QString m_downloadStatusText;
    QString m_tempDownloadPath;
};

} // namespace acbo
