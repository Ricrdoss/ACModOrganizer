#include "UpdateManager.hpp"
#include "Logger.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace acbo {

UpdateManager::UpdateManager(QObject* parent)
    : QObject(parent) {
}

static std::vector<int> parseVersionNumbers(const QString& verStr) {
    std::vector<int> parts;
    QString clean = verStr.trimmed();
    if (clean.startsWith('v', Qt::CaseInsensitive)) {
        clean = clean.mid(1);
    }
    QStringList tokens = clean.split('.', Qt::SkipEmptyParts);
    for (const QString& tok : tokens) {
        // Extract leading numeric part
        static QRegularExpression re(R"(^\d+)");
        auto match = re.match(tok);
        if (match.hasMatch()) {
            parts.push_back(match.captured(0).toInt());
        } else {
            parts.push_back(0);
        }
    }
    while (parts.size() < 3) {
        parts.push_back(0);
    }
    return parts;
}

bool UpdateManager::isVersionNewer(const QString& current, const QString& latest) {
    auto curParts = parseVersionNumbers(current);
    auto latParts = parseVersionNumbers(latest);

    for (size_t i = 0; i < std::min(curParts.size(), latParts.size()); ++i) {
        if (latParts[i] > curParts[i]) return true;
        if (latParts[i] < curParts[i]) return false;
    }
    return latParts.size() > curParts.size();
}

void UpdateManager::checkForUpdates(bool silent) {
    if (m_isChecking || m_isDownloading) return;

    m_isChecking = true;
    m_silentCheck = silent;
    emit isCheckingChanged();

    // Query GitHub Releases API for ACModOrganize
    QUrl url("https://api.github.com/repos/hrubcin/ACModOrganize/releases/latest");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ACBO-App-Updater/1.0");
    request.setRawHeader("Accept", "application/vnd.github.v3+json");

    m_currentReply = m_networkManager.get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UpdateManager::onCheckFinished);
}

void UpdateManager::onCheckFinished() {
    m_isChecking = false;
    emit isCheckingChanged();

    if (!m_currentReply) return;

    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString errStr = m_currentReply->errorString();
        LOG_INFO("Update check response: " + errStr.toStdString());
        if (!m_silentCheck) {
            emit updateError("Unable to reach GitHub update server: " + errStr);
        }
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    QByteArray response = m_currentReply->readAll();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isObject()) {
        if (!m_silentCheck) emit updateError("Invalid update response received.");
        return;
    }

    QJsonObject obj = doc.object();
    QString tagName = obj.value("tag_name").toString();
    QString title = obj.value("name").toString();
    QString body = obj.value("body").toString();

    if (tagName.isEmpty()) {
        if (!m_silentCheck) emit updateError("No release tag found in update metadata.");
        return;
    }

    // Extract asset download URL for ACBO.exe or zip
    QUrl downloadUrl;
    qint64 assetSize = 0;
    QJsonArray assets = obj.value("assets").toArray();
    for (const auto& assetVal : assets) {
        QJsonObject asset = assetVal.toObject();
        QString assetName = asset.value("name").toString();
        if (assetName.compare("ACBO.exe", Qt::CaseInsensitive) == 0 || assetName.endsWith(".exe", Qt::CaseInsensitive)) {
            downloadUrl = QUrl(asset.value("browser_download_url").toString());
            assetSize = asset.value("size").toInteger();
            break;
        }
    }

    if (!downloadUrl.isValid() && !assets.isEmpty()) {
        // Fallback to first available asset
        QJsonObject asset = assets.first().toObject();
        downloadUrl = QUrl(asset.value("browser_download_url").toString());
        assetSize = asset.value("size").toInteger();
    }

    m_latestVersion = tagName;
    m_releaseTitle = title.isEmpty() ? tagName : title;
    m_releaseNotes = body.isEmpty() ? "Bug fixes and performance improvements." : body;
    m_downloadUrl = downloadUrl;
    m_assetSize = assetSize;

    bool hasNewer = isVersionNewer(appVersion(), m_latestVersion);
    m_updateAvailable = hasNewer;

    emit updateDetailsChanged();
    emit updateAvailableChanged();
    emit updateCheckCompleted(hasNewer, m_latestVersion);
}

void UpdateManager::startDownloadAndInstall() {
    if (m_isDownloading || !m_downloadUrl.isValid()) {
        emit updateError("No valid download URL available for this update.");
        return;
    }

    m_isDownloading = true;
    m_downloadProgress = 0.0;
    m_downloadStatusText = "Connecting to download server...";
    emit isDownloadingChanged();
    emit downloadProgressChanged();
    emit downloadStatusTextChanged();

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_tempDownloadPath = tempDir + "/ACBO_update.exe";

    if (QFile::exists(m_tempDownloadPath)) {
        QFile::remove(m_tempDownloadPath);
    }

    m_downloadFile = new QFile(m_tempDownloadPath, this);
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
        m_isDownloading = false;
        emit isDownloadingChanged();
        emit updateError("Failed to create temporary update file on disk.");
        delete m_downloadFile;
        m_downloadFile = nullptr;
        return;
    }

    QNetworkRequest request(m_downloadUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ACBO-App-Updater/1.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_currentReply = m_networkManager.get(request);
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &UpdateManager::onDownloadProgress);
    connect(m_currentReply, &QNetworkReply::finished, this, &UpdateManager::onDownloadFinished);
    connect(m_currentReply, &QNetworkReply::readyRead, this, [this]() {
        if (m_downloadFile && m_currentReply) {
            m_downloadFile->write(m_currentReply->readAll());
        }
    });
}

void UpdateManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        m_downloadProgress = static_cast<double>(bytesReceived) / bytesTotal;
        double mbRec = bytesReceived / (1024.0 * 1024.0);
        double mbTot = bytesTotal / (1024.0 * 1024.0);
        m_downloadStatusText = QString("Downloading update: %1 MB / %2 MB (%3%)")
                                   .arg(mbRec, 0, 'f', 1)
                                   .arg(mbTot, 0, 'f', 1)
                                   .arg(static_cast<int>(m_downloadProgress * 100));
    } else {
        double mbRec = bytesReceived / (1024.0 * 1024.0);
        m_downloadStatusText = QString("Downloading update: %1 MB...").arg(mbRec, 0, 'f', 1);
    }
    emit downloadProgressChanged();
    emit downloadStatusTextChanged();
}

void UpdateManager::onDownloadFinished() {
    m_isDownloading = false;
    emit isDownloadingChanged();

    if (m_downloadFile) {
        m_downloadFile->flush();
        m_downloadFile->close();
        delete m_downloadFile;
        m_downloadFile = nullptr;
    }

    if (!m_currentReply) return;

    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString err = m_currentReply->errorString();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        emit updateError("Download failed: " + err);
        return;
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    m_downloadStatusText = "Download complete! Restarting application...";
    emit downloadStatusTextChanged();

    applyUpdateAndRestart(m_tempDownloadPath);
}

void UpdateManager::cancelDownload() {
    if (m_currentReply && m_isDownloading) {
        m_currentReply->abort();
    }
    if (m_downloadFile) {
        m_downloadFile->close();
        m_downloadFile->remove();
        delete m_downloadFile;
        m_downloadFile = nullptr;
    }
    m_isDownloading = false;
    emit isDownloadingChanged();
    emit downloadStatusTextChanged();
}

void UpdateManager::applyUpdateAndRestart(const QString& downloadedFilePath) {
    QString currentExePath = QCoreApplication::applicationFilePath();
    qint64 currentPid = QCoreApplication::applicationPid();

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString scriptPath = tempDir + "/update_acbo.bat";

    // Create a self-deleting batch script that waits for ACBO to close, replaces the EXE, and relaunches it
    QString scriptContent = QString(
        "@echo off\n"
        "title Updating ACBO...\n"
        "echo Waiting for ACBO to close (PID: %1)...\n"
        ":wait_loop\n"
        "tasklist /fi \"PID eq %1\" 2>nul | find \"%1\" >nul\n"
        "if not errorlevel 1 (\n"
        "    timeout /t 1 /nobreak >nul\n"
        "    goto wait_loop\n"
        ")\n"
        "echo Applying updated ACBO executable...\n"
        "copy /y \"%2\" \"%3\" >nul\n"
        "del \"%2\" >nul 2>&1\n"
        "echo Launching ACBO...\n"
        "start \"\" \"%3\"\n"
        "del \"%~f0\" >nul 2>&1\n"
        "exit\n"
    ).arg(QString::number(currentPid), QDir::toNativeSeparators(downloadedFilePath), QDir::toNativeSeparators(currentExePath));

    QFile scriptFile(scriptPath);
    if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&scriptFile);
        out << scriptContent;
        scriptFile.close();

        // Launch the updater batch script silently
        QProcess::startDetached("cmd.exe", QStringList() << "/c" << QDir::toNativeSeparators(scriptPath));

        // Exit current app cleanly
        QCoreApplication::quit();
    } else {
        emit updateError("Failed to create updater script.");
    }
}

} // namespace acbo
