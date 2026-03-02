#include "ConsoleWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>

#include <algorithm>

ConsoleWidget::ConsoleWidget(QWidget* parent)
    : QWidget(parent) {
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);

    m_font = QFont(QStringLiteral("Consolas"));
    m_font.setStyleHint(QFont::Monospace);
    m_font.setPixelSize(m_style.fontSizePx);
}

void ConsoleWidget::clearBuffer() {
    m_lines.clear();
    m_currentLine.clear();
    update();
}

void ConsoleWidget::setStyle(const Style& style) {
    m_style = style;
    m_font.setPixelSize(m_style.fontSizePx);
    update();
}

void ConsoleWidget::setText(const QString& text) {
    m_lines = text.split('\n');
    m_currentLine.clear();

    // Trim trailing empty produced by split if text ends with '\n'
    while (!m_lines.isEmpty() && m_lines.last().isEmpty()) {
        m_lines.removeLast();
    }

    // Cap stored lines
    while (m_lines.size() > m_maxStoredLines) {
        m_lines.removeFirst();
    }

    update();
}

void ConsoleWidget::appendText(const QString& text) {
    // Handle embedded newlines so the Engine can do appendText("\n")
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n')) {
            m_lines.push_back(m_currentLine);
            m_currentLine.clear();

            while (m_lines.size() > m_maxStoredLines) {
                m_lines.removeFirst();
            }
        } else if (ch != QLatin1Char('\r')) {
            // Ignore CR to be safe with CRLF inputs
            m_currentLine += ch;
        }
    }
    update();
}

void ConsoleWidget::appendLine(const QString& line) {
    appendText(line);
    appendText("\n");
}

QRect ConsoleWidget::computeTextRect(const QSize& textSize) const {
    const QRect area = rect();

    int x = area.left();
    int y = area.top();

    switch (m_style.alignH) {
        case AlignH::Left:   x = area.left(); break;
        case AlignH::Center: x = area.left() + (area.width() - textSize.width()) / 2; break;
        case AlignH::Right:  x = area.right() - textSize.width(); break;
    }

    switch (m_style.alignV) {
        case AlignV::Top:    y = area.top(); break;
        case AlignV::Middle: y = area.top() + (area.height() - textSize.height()) / 2; break;
        case AlignV::Bottom: y = area.bottom() - textSize.height(); break;
    }

    return QRect(QPoint(x, y), textSize);
}

void ConsoleWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    painter.fillRect(rect(), Qt::black);

    painter.setFont(m_font);
    painter.setPen(m_style.color);

    const QFontMetrics fm(m_font);

    // How many lines fit?
    const int lineHeight = std::max(1, fm.height());
    const int availableHeight = height();
    const int maxVisibleLines = std::max(1, availableHeight / lineHeight);

    // Build visible lines: last N from m_lines + (optional) currentLine as last
    QStringList all = m_lines;
    if (!m_currentLine.isEmpty()) {
        all.push_back(m_currentLine);
    }

    if (all.isEmpty()) {
        return;
    }

    const qsizetype maxVisible = static_cast<qsizetype>(maxVisibleLines);
    const qsizetype startIndex = std::max<qsizetype>(0, all.size() - maxVisible);
    const QStringList visible = all.mid(startIndex);

    // Compute text block width (max line width) and height
    int maxWidth = 0;
    for (const QString& l : visible) {
        maxWidth = std::max(maxWidth, fm.horizontalAdvance(l));
    }
    const int blockHeight = visible.size() * lineHeight;

    const QSize textSize(maxWidth, blockHeight);
    const QRect targetRect = computeTextRect(textSize);

    // Draw line by line for precise spacing (and no word-wrap surprises)
    int y = targetRect.top() + fm.ascent();
    for (const QString& l : visible) {
        painter.drawText(targetRect.left(), y, l);
        y += lineHeight;
    }
}

void ConsoleWidget::keyPressEvent(QKeyEvent* event) {
    if (!event) {
        return;
    }

    emit keyPressed(event->key());

    // Stage safety: do not close on any key. Only Alt+F4 should close the app.
    event->accept();
}
