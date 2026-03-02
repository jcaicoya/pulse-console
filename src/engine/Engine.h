#pragma once

#include <QObject>
#include <QTimer>

#include <cstddef>

#include "Steps.h"

class ConsoleWidget;

namespace pc {

    class Engine final : public QObject {
        Q_OBJECT

    public:
        explicit Engine(ConsoleWidget* console, QObject* parent = nullptr);

        void start(const Script& script);

    private slots:
        void onTick();
        void onKeyPressed(int qtKey);

    private:
        void startNextStep();
        void beginWrite(const QString& text);
        void beginWait(double seconds);
        void applyStyle(const Style& style);
        void doClear();

    private:
        ConsoleWidget* m_console = nullptr;

        Script m_script{};
        std::size_t m_stepIndex = 0;

        // Execution state
        QTimer m_timer;
        enum class Mode { Idle, Writing, Waiting, Paused } m_mode = Mode::Idle;

        // Writing state
        QString m_writeText;
        int m_writePos = 0;

        // Waiting state
        int m_waitRemainingMs = 0;

        // Pause state
        int m_expectedQtKey = 0; // 0 means "any key"

        // Current params
        double m_charDelaySeconds = 0.02;
        Style m_currentStyle{};
    };

} // namespace pc
