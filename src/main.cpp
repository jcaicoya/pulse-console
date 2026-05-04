#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QStringList>
#include <iostream>

#include "CyberAppMode.h"
#include "ui/ConsoleWidget.h"
#include "engine/Engine.h"
#include "engine/ScriptLoader.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("pulse-console");

    // Extract --file before parseAppLaunchOptions (unknown flags are hard errors).
    QStringList args = app.arguments();
    QString scriptFile;
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == "--file" && i + 1 < args.size()) {
            scriptFile = args[i + 1];
            args.removeAt(i + 1);
            args.removeAt(i);
            break;
        }
    }

    // Normalize Orchestrator vocabulary to app vocabulary.
    for (auto& a : args) {
        if (a == "--show")   a = "--live";
        if (a == "--design") a = "--demo";
    }

    const cybershow::ParseResult parseResult = cybershow::parseAppLaunchOptions(args);
    if (!parseResult.ok) {
        std::cerr << "pulse-console: " << parseResult.error.toStdString() << std::endl;
        return 1;
    }

    std::cout << "CYBERSHOW_STATUS READY" << std::endl;

    const cybershow::AppLaunchOptions& options = parseResult.options;

    const auto screens = QGuiApplication::screens();
    QScreen* targetScreen = (options.screenIndex >= 0 && options.screenIndex < screens.size())
                            ? screens.at(options.screenIndex)
                            : QGuiApplication::primaryScreen();

    auto* console = new ConsoleWidget();

    if (options.fullscreen) {
        console->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        if (targetScreen)
            console->move(targetScreen->geometry().topLeft());
        console->showFullScreen();
    } else if (options.windowed) {
        if (targetScreen)
            console->setGeometry(targetScreen->availableGeometry());
        console->show();
    } else {
        if (targetScreen)
            console->setGeometry(targetScreen->availableGeometry());
        console->showMaximized();
    }

    console->activateWindow();
    console->raise();
    console->setFocus();
    console->setCursor(Qt::BlankCursor);

    pc::Script script;
    try {
        if (!scriptFile.isEmpty()) {
            script = pc::ScriptLoader::loadFromFile(scriptFile);
        } else {
            script = pc::ScriptLoader::loadFromResource(":/scripts/default.yaml");
        }
    } catch (const std::exception& e) {
        std::cerr << "pulse-console: script load failed: " << e.what() << std::endl;
        console->appendLine("pulse-console");
        console->appendLine("ERROR: Failed to load script.");
        console->appendLine(QString::fromUtf8(e.what()));
        console->appendLine("");
        console->appendLine("Close the window using Alt+F4.");
        return app.exec();
    }

    auto* engine = new pc::Engine(console, &app);

    std::cout << "CYBERSHOW_STATUS RUNNING" << std::endl;

    engine->start(script);
    return app.exec();
}
