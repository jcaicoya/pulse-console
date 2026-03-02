#pragma once

#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QKeyEvent>

class ConsoleWidget final : public QWidget {
    Q_OBJECT

public:
    enum class AlignH { Left, Center, Right };
    enum class AlignV { Top, Middle, Bottom };

    struct Style {
        QColor color = QColor(0, 255, 0); // Default: green
        int fontSizePx = 20;              // Pixel size for consistent rendering
        AlignH alignH = AlignH::Left;
        AlignV alignV = AlignV::Top;
    };

    explicit ConsoleWidget(QWidget* parent = nullptr);

    // Console-like API (minimal for now)
    void clearBuffer();
    void setStyle(const Style& style);
    const Style& style() const { return m_style; }

    void setText(const QString& text);     // Replaces entire buffer (splits into lines)
    void appendText(const QString& text);  // Appends to current line
    void appendLine(const QString& line);  // Appends a new line

signals:
    void keyPressed(int qtKey);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QRect computeTextRect(const QSize& textSize) const;

    Style m_style{};
    QStringList m_lines{};
    QString m_currentLine{};
    int m_maxStoredLines = 2000; // safety cap; tweak later
    QFont m_font{};
};
