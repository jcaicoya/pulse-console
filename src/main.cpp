#include <QApplication>
#include <QDebug>

#include "ui/ConsoleWidget.h"
#include "engine/Engine.h"
#include "engine/ScriptLoader.h"

static QString getScriptArg(const QStringList& args) {
    // Supported:
    //   --script <path>
    //   --script=<path>
    for (int i = 1; i < args.size(); ++i) {
        const QString a = args[i];

        if (a == "--script") {
            if (i + 1 < args.size()) {
                return args[i + 1];
            }
            return QString(); // missing value
        }

        const QString prefix = "--script=";
        if (a.startsWith(prefix)) {
            return a.mid(prefix.size());
        }
    }
    return QString();
}

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

    const QStringList args = QCoreApplication::arguments();
    const QString scriptPath = getScriptArg(args);

    pc::Script script;
    try {
        if (!scriptPath.isEmpty()) {
            qDebug() << "PulseConsole: loading script from file:" << scriptPath;
            script = pc::ScriptLoader::loadFromFile(scriptPath);
        } else {
            qDebug() << "PulseConsole: loading default script from resources";
            script = pc::ScriptLoader::loadFromResource(":/scripts/default.yaml");
        }

        qDebug() << "PulseConsole: script loaded. steps =" << static_cast<int>(script.steps.size());
    } catch (const std::exception& e) {
        qDebug() << "PulseConsole: script load failed:" << e.what();
        console->appendLine("PulseConsole");
        console->appendLine("ERROR: Failed to load script.");
        console->appendLine(QString::fromUtf8(e.what()));
        console->appendLine("");
        console->appendLine("Close the window using Alt+F4.");
        return app.exec();
    }

    auto* engine = new pc::Engine(console, &app);
    engine->start(script);

    return app.exec();
}