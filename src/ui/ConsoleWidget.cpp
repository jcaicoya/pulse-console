
#include "ConsoleWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>

ConsoleWidget::ConsoleWidget(QWidget* parent)
    : QWidget(parent) {
    // Console look
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);

    m_font = QFont(QStringLiteral("Consolas"));
    m_font.setStyleHint(QFont::Monospace);
    m_font.setPixelSize(m_style.fontSizePx);
}

void ConsoleWidget::clearBuffer() {
    m_buffer.clear();
    update();
}

void ConsoleWidget::setStyle(const Style& style) {
    m_style = style;
    m_font.setPixelSize(m_style.fontSizePx);
    update();
}

void ConsoleWidget::setText(const QString& text) {
    m_buffer = text;
    update();
}

void ConsoleWidget::appendText(const QString& text) {
    m_buffer += text;
    update();
}

void ConsoleWidget::appendLine(const QString& line) {
    m_buffer += line;
    m_buffer += QLatin1Char('\n');
    update();
}

QRect ConsoleWidget::computeTextRect(const QSize& textSize) const {
    const QRect area = rect();

    int x = area.left();
    int y = area.top();

    // Horizontal alignment
    switch (m_style.alignH) {
        case AlignH::Left:
            x = area.left();
            break;
        case AlignH::Center:
            x = area.left() + (area.width() - textSize.width()) / 2;
            break;
        case AlignH::Right:
            x = area.right() - textSize.width();
            break;
    }

    // Vertical alignment
    switch (m_style.alignV) {
        case AlignV::Top:
            y = area.top();
            break;
        case AlignV::Middle:
            y = area.top() + (area.height() - textSize.height()) / 2;
            break;
        case AlignV::Bottom:
            y = area.bottom() - textSize.height();
            break;
    }

    return QRect(QPoint(x, y), textSize);
}

void ConsoleWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // Background
    painter.fillRect(rect(), Qt::black);

    painter.setFont(m_font);
    painter.setPen(m_style.color);

    if (m_buffer.isEmpty()) {
        return;
    }

    // Measure text block size
    const QFontMetrics fm(m_font);
    const QRect textBounds = fm.boundingRect(QRect(0, 0, width(), height()),
                                            Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop,
                                            m_buffer);

    const QSize textSize(textBounds.width(), textBounds.height());
    const QRect targetRect = computeTextRect(textSize);

    painter.drawText(targetRect,
                     Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop,
                     m_buffer);
}
