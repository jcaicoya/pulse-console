#include "Engine.h"

#include "ui/ConsoleWidget.h"

#include <QTimer>

#include <algorithm>
#include <type_traits>

namespace pc {

static ConsoleWidget::AlignH toUiAlignH(AlignH a) {
    switch (a) {
        case AlignH::Left:   return ConsoleWidget::AlignH::Left;
        case AlignH::Center: return ConsoleWidget::AlignH::Center;
        case AlignH::Right:  return ConsoleWidget::AlignH::Right;
    }
    return ConsoleWidget::AlignH::Left;
}

static ConsoleWidget::AlignV toUiAlignV(AlignV a) {
    switch (a) {
        case AlignV::Top:    return ConsoleWidget::AlignV::Top;
        case AlignV::Middle: return ConsoleWidget::AlignV::Middle;
        case AlignV::Bottom: return ConsoleWidget::AlignV::Bottom;
    }
    return ConsoleWidget::AlignV::Top;
}

static int parseExpectedQtKey(const QString& keyName) {
    if (keyName.compare("Space", Qt::CaseInsensitive) == 0) return Qt::Key_Space;
    if (keyName.compare("Enter", Qt::CaseInsensitive) == 0) return Qt::Key_Return;
    if (keyName.compare("Return", Qt::CaseInsensitive) == 0) return Qt::Key_Return;
    if (keyName.compare("Any", Qt::CaseInsensitive) == 0) return 0;

    // Allow single characters: "A", "q", etc.
    if (keyName.size() == 1) {
        const QChar c = keyName[0].toUpper();
        if (c.isLetterOrNumber()) {
            return c.unicode();
        }
    }

    // Unknown key name -> treat as "any key"
    return 0;
}

Engine::Engine(ConsoleWidget* console, QObject* parent)
    : QObject(parent),
      m_console(console) {
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &Engine::onTick);

    if (m_console) {
        connect(m_console, &ConsoleWidget::keyPressed, this, &Engine::onKeyPressed);
    }
}

void Engine::start(const Script& script) {
    m_script = script;
    m_stepIndex = 0;

    // Apply defaults
    m_charDelaySeconds = std::max(0.0, m_script.defaults.type.charDelaySeconds);
    m_currentStyle = m_script.defaults.style;

    applyStyle(m_currentStyle);
    doClear();

    m_mode = Mode::Idle;
    m_timer.stop();

    // Reset transient state
    m_expectedQtKey = 0;
    m_writeText.clear();
    m_writePos = 0;
    m_waitRemainingMs = 0;

    startNextStep();
}

void Engine::applyStyle(const Style& style) {
    if (!m_console) return;

    ConsoleWidget::Style s;
    s.color = style.color;
    s.fontSizePx = style.sizePx;
    s.alignH = toUiAlignH(style.alignH);
    s.alignV = toUiAlignV(style.alignV);

    m_console->setStyle(s);
}

void Engine::doClear() {
    if (!m_console) return;
    m_console->clearBuffer();
}

void Engine::beginWrite(const QString& text) {
    m_mode = Mode::Writing;
    m_writeText = text;
    m_writePos = 0;

    // If delay is 0, write instantly
    if (m_charDelaySeconds <= 0.0) {
        if (m_console) {
            m_console->appendLine(m_writeText);
        }
        m_mode = Mode::Idle;
        startNextStep();
        return;
    }

    const int intervalMs = std::max(1, static_cast<int>(m_charDelaySeconds * 1000.0));
    m_timer.start(intervalMs);
}

void Engine::beginWait(double seconds) {
    m_mode = Mode::Waiting;

    const int ms = std::max(0, static_cast<int>(seconds * 1000.0));
    m_waitRemainingMs = ms;

    if (m_waitRemainingMs == 0) {
        m_mode = Mode::Idle;
        startNextStep();
        return;
    }

    // Tick at 50ms granularity for waits
    m_timer.start(50);
}

void Engine::startNextStep() {
    if (m_stepIndex >= m_script.steps.size()) {
        m_mode = Mode::Idle;
        m_timer.stop();
        return;
    }

    const Step& step = m_script.steps[m_stepIndex++];
    std::visit([this](auto&& s) {
        using T = std::decay_t<decltype(s)>;

        if constexpr (std::is_same_v<T, WriteStep>) {
            beginWrite(s.text);
        } else if constexpr (std::is_same_v<T, WaitStep>) {
            beginWait(s.seconds);
        } else if constexpr (std::is_same_v<T, ClearStep>) {
            doClear();
            startNextStep();
        } else if constexpr (std::is_same_v<T, SetStyleStep>) {
            m_currentStyle = s.style;
            applyStyle(m_currentStyle);
            startNextStep();
        } else if constexpr (std::is_same_v<T, PauseUntilKeyStep>) {
            // Pause until a key is pressed (or a specific key).
            m_timer.stop();
            m_mode = Mode::Paused;
            m_expectedQtKey = parseExpectedQtKey(s.key);
        }
    }, step);
}

void Engine::onTick() {
    if (!m_console) {
        m_timer.stop();
        m_mode = Mode::Idle;
        return;
    }

    if (m_mode == Mode::Paused) {
        m_timer.stop();
        return;
    }

    if (m_mode == Mode::Writing) {
        if (m_writePos < m_writeText.size()) {
            m_console->appendText(QString(m_writeText[m_writePos]));
            ++m_writePos;
            return;
        }

        // End of line
        m_console->appendText("\n");
        m_timer.stop();
        m_mode = Mode::Idle;
        startNextStep();
        return;
    }

    if (m_mode == Mode::Waiting) {
        m_waitRemainingMs -= 50;
        if (m_waitRemainingMs <= 0) {
            m_timer.stop();
            m_mode = Mode::Idle;
            startNextStep();
        }
        return;
    }

    // Idle
    m_timer.stop();
}

void Engine::onKeyPressed(int qtKey) {
    if (m_mode != Mode::Paused) {
        return;
    }

    // 0 means "any key"
    if (m_expectedQtKey == 0 || qtKey == m_expectedQtKey) {
        m_mode = Mode::Idle;
        m_expectedQtKey = 0;
        startNextStep();
    }
}

} // namespace pc
