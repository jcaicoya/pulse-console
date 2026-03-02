#include <QApplication>
#include <QFile>
#include <QDebug>

#include "ui/ConsoleWidget.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Create a console-like window (no frame, no title bar).
    auto* console = new ConsoleWidget();
    console->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    console->setCursor(Qt::BlankCursor);
    console->setFocusPolicy(Qt::StrongFocus);

    // Optional: ensure it grabs focus (useful for future key handling)
    console->activateWindow();
    console->raise();

    // Temporary resource check (can be removed later)
    QFile f(":/scripts/default.yaml");
    qDebug() << "Resource exists:" << f.exists();

    // Show fullscreen for stage use
    console->showFullScreen();

    return app.exec();
}