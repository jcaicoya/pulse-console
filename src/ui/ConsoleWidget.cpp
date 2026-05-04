#include "ConsoleWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QRadialGradient>
#include <QResizeEvent>

#include <algorithm>
#include <random>

ConsoleWidget::ConsoleWidget(QWidget* parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);

    m_font = QFont(QStringLiteral("Consolas"));
    m_font.setStyleHint(QFont::Monospace);
    m_font.setPixelSize(m_style.fontSizePx);

    m_flickerTimer.setSingleShot(false);
    connect(&m_flickerTimer, &QTimer::timeout, this, [this]() {
        if (m_effects.flickerStrength <= 0.0) {
            m_flickerFactor = 1.0;
            return;
        }
        static thread_local std::mt19937 rng{std::random_device{}()};
        const double s = std::clamp(m_effects.flickerStrength, 0.0, 0.2);
        std::uniform_real_distribution<double> dist(1.0 - s, 1.0 + s);
        m_flickerFactor = dist(rng);
        update();
    });

    if (m_effects.flickerStrength > 0.0)
        m_flickerTimer.start(150);
}

void ConsoleWidget::setEffects(const Effects& effects) {
    m_effects = effects;
    if (m_effects.flickerStrength > 0.0) {
        if (!m_flickerTimer.isActive()) m_flickerTimer.start(150);
    } else {
        m_flickerTimer.stop();
        m_flickerFactor = 1.0;
    }
    update();
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
    while (!m_lines.isEmpty() && m_lines.last().isEmpty())
        m_lines.removeLast();
    while (m_lines.size() > m_maxStoredLines)
        m_lines.removeFirst();
    update();
}

void ConsoleWidget::appendText(const QString& text) {
    for (const QChar ch : text) {
        if (ch == QLatin1Char('\n')) {
            m_lines.push_back(m_currentLine);
            m_currentLine.clear();
            while (m_lines.size() > m_maxStoredLines)
                m_lines.removeFirst();
        } else if (ch != QLatin1Char('\r')) {
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
    int x = area.left(), y = area.top();
    switch (m_style.alignH) {
        case AlignH::Left:   x = area.left(); break;
        case AlignH::Center: x = area.left() + (area.width()  - textSize.width())  / 2; break;
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

    const QFontMetrics fm(m_font);
    const int lineHeight      = std::max(1, fm.height());
    const int maxVisibleLines = std::max(1, height() / lineHeight);

    const bool     hasCurrent   = !m_currentLine.isEmpty();
    const qsizetype storedCount = m_lines.size();
    const qsizetype totalCount  = storedCount + (hasCurrent ? 1 : 0);

    if (totalCount == 0) {
        if (m_effects.scanlines && !m_scanlinesCache.isNull())
            painter.drawPixmap(0, 0, m_scanlinesCache);
        if (m_effects.vignette && !m_vignetteCache.isNull())
            painter.drawPixmap(0, 0, m_vignetteCache);
        return;
    }

    const qsizetype visibleCount       = std::min<qsizetype>(totalCount, maxVisibleLines);
    const qsizetype startIndex         = totalCount - visibleCount;
    const qsizetype endIndexExclusive  = startIndex + visibleCount;

    int maxWidth = 0;
    for (qsizetype i = startIndex; i < endIndexExclusive; ++i) {
        const QString& line = (i < storedCount) ? m_lines[static_cast<int>(i)] : m_currentLine;
        maxWidth = std::max(maxWidth, fm.horizontalAdvance(line));
    }

    const int   blockHeight = static_cast<int>(visibleCount) * lineHeight;
    const QRect targetRect  = computeTextRect(QSize(maxWidth, blockHeight));

    QColor baseColor = m_style.color;
    baseColor.setAlpha(std::clamp(static_cast<int>(baseColor.alpha() * m_flickerFactor), 0, 255));

    const int x0 = targetRect.left();
    int y = targetRect.top() + fm.ascent();

    if (m_effects.glow) {
        QColor glow = baseColor;
        glow.setAlpha(std::min(70, baseColor.alpha()));
        constexpr int offsets[][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
        painter.setPen(glow);
        for (const auto& off : offsets) {
            int yy = y;
            for (qsizetype i = startIndex; i < endIndexExclusive; ++i) {
                const QString& line = (i < storedCount) ? m_lines[static_cast<int>(i)] : m_currentLine;
                painter.drawText(x0 + off[0], yy + off[1], line);
                yy += lineHeight;
            }
        }
        painter.setPen(baseColor);
        for (qsizetype i = startIndex; i < endIndexExclusive; ++i) {
            const QString& line = (i < storedCount) ? m_lines[static_cast<int>(i)] : m_currentLine;
            painter.drawText(x0, y, line);
            y += lineHeight;
        }
    } else {
        painter.setPen(baseColor);
        for (qsizetype i = startIndex; i < endIndexExclusive; ++i) {
            const QString& line = (i < storedCount) ? m_lines[static_cast<int>(i)] : m_currentLine;
            painter.drawText(x0, y, line);
            y += lineHeight;
        }
    }

    if (m_effects.scanlines && !m_scanlinesCache.isNull())
        painter.drawPixmap(0, 0, m_scanlinesCache);
    if (m_effects.vignette && !m_vignetteCache.isNull())
        painter.drawPixmap(0, 0, m_vignetteCache);
}

void ConsoleWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    rebuildOverlays();
}

void ConsoleWidget::rebuildOverlays() {
    const QRect r = rect();
    if (r.width() <= 0 || r.height() <= 0) {
        m_scanlinesCache = QPixmap();
        m_vignetteCache  = QPixmap();
        return;
    }
    {
        QPixmap pm(r.size());
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setPen(Qt::NoPen);
        const QColor c(0, 0, 0, 40);
        for (int y = 0; y < r.height(); y += 3)
            p.fillRect(QRect(0, y, r.width(), 1), c);
        m_scanlinesCache = std::move(pm);
    }
    {
        QPixmap pm(r.size());
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        const QPointF center(r.width() / 2.0, r.height() / 2.0);
        const double  radius = std::max(r.width(), r.height()) * 0.65;
        QRadialGradient grad(center, radius);
        grad.setColorAt(0.0, QColor(0, 0, 0,   0));
        grad.setColorAt(0.7, QColor(0, 0, 0,   0));
        grad.setColorAt(1.0, QColor(0, 0, 0, 160));
        p.fillRect(QRect(0, 0, r.width(), r.height()), grad);
        m_vignetteCache = std::move(pm);
    }
}

void ConsoleWidget::keyPressEvent(QKeyEvent* event) {
    if (!event) return;
    emit keyPressed(event->key());
    QWidget::keyPressEvent(event);
}
