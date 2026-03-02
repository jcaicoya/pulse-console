#include <QApplication>
#include <QDebug>

#include "ui/ConsoleWidget.h"
#include "engine/ScriptLoader.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Create a console-like window (no frame, no title bar).
    auto* console = new ConsoleWidget();
    console->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    console->setCursor(Qt::BlankCursor);
    console->setFocusPolicy(Qt::StrongFocus);

    // Load script from Qt resources (YAML).
    try {
        const pc::Script script = pc::ScriptLoader::loadFromResource(":/scripts/default.yaml");
        qDebug() << "PulseConsole: script loaded. steps =" << static_cast<int>(script.steps.size());

        // For now, just show something so we know rendering works:
        console->appendLine("PulseConsole");
        console->appendLine("Script loaded successfully.");
        console->appendLine(QString("Steps: %1").arg(static_cast<int>(script.steps.size())));
        console->appendLine("");
        console->appendLine("Press ESC or Q to exit (temporary).");
    } catch (const std::exception& e) {
        qDebug() << "PulseConsole: script load failed:" << e.what();
        console->appendLine("PulseConsole");
        console->appendLine("ERROR: Failed to load script.");
        console->appendLine(QString::fromUtf8(e.what()));
        console->appendLine("");
        console->appendLine("Press ESC or Q to exit (temporary).");
    }

    // Show fullscreen for stage use
    console->showFullScreen();

    // Ensure keyboard focus (important for ESC/Q)
    console->activateWindow();
    console->raise();
    console->setFocus();

    return app.exec();
}
