#include <QApplication>
#include <QDebug>

#include "ui/ConsoleWidget.h"
#include "engine/Engine.h"
#include "engine/ScriptLoader.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    auto* console = new ConsoleWidget();
    console->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    console->setCursor(Qt::BlankCursor);
    console->setFocusPolicy(Qt::StrongFocus);

    console->showFullScreen();
    console->activateWindow();
    console->raise();
    console->setFocus();

    pc::Script script;
    try {
        script = pc::ScriptLoader::loadFromResource(":/scripts/default.yaml");
        qDebug() << "PulseConsole: script loaded. steps =" << static_cast<int>(script.steps.size());
    } catch (const std::exception& e) {
        qDebug() << "PulseConsole: script load failed:" << e.what();
        console->appendLine("PulseConsole");
        console->appendLine("ERROR: Failed to load script.");
        console->appendLine(QString::fromUtf8(e.what()));
        console->appendLine("");
        console->appendLine("Press ESC or Q to exit (temporary).");
        return app.exec();
    }

    auto* engine = new pc::Engine(console, &app);
    engine->start(script);

    return app.exec();
}
