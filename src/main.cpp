#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include "AppController.hpp"
#include "Logger.hpp"

#include <iostream>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg) {
        std::string text = msg.toStdString();
        if (context.file) {
            text += " (" + std::string(context.file) + ":" + std::to_string(context.line) + ")";
        }
        LOG_WARN(text);
    }
}

int main(int argc, char* argv[]) {
    qInstallMessageHandler(myMessageOutput);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    app.setApplicationName("Assetto Corsa Mod Organizer");
    app.setApplicationDisplayName("Assetto Corsa Mod Organizer");
    app.setOrganizationName("ACBO");
    app.setOrganizationDomain("github.com/hrubc/ACModOrganize");
    app.setWindowIcon(QIcon(":/qt/qml/ACBO/ui/icons/app_icon.ico"));

    // Use Basic or Fusion dark style for clean modern look
    QQuickStyle::setStyle("Basic");

    LOG_INFO("Initializing AC Brand & Country Organizer (ACBO)...");

    QQmlApplicationEngine engine;

    auto controller = std::make_unique<acbo::AppController>();
    engine.rootContext()->setContextProperty("appController", controller.get());
    engine.rootContext()->setContextProperty("carModel", controller->carModel());

    // Connect engine warnings
    QObject::connect(&engine, &QQmlApplicationEngine::warnings,
                     [](const QList<QQmlError>& warnings) {
        for (const auto& w : warnings) {
            std::cerr << "[QML ERROR] " << w.toString().toStdString() << std::endl;
        }
    });

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() {
        LOG_ERROR("Failed to load QML interface.");
        QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    const QUrl url(QStringLiteral("qrc:/qt/qml/ACBO/ui/Main.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        // Fallback for non-qt_add_qml_module resource path or direct file loading
        const QUrl fallbackUrl(QStringLiteral("qrc:/ui/Main.qml"));
        engine.load(fallbackUrl);
        if (engine.rootObjects().isEmpty()) {
            LOG_ERROR("Could not locate Main.qml in application resources.");
            return -1;
        }
    }

    LOG_SUCCESS("Application initialized successfully.");
    return app.exec();
}
