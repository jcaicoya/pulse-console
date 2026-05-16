#pragma once

#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QWidget>
#include <QKeyEvent>

class ConsoleWidget final : public QWidget {
    Q_OBJECT

public:
    enum class AlignH { Left, Center, Right };
    enum class AlignV { Top, Middle, Bottom };

    struct Style {
        QColor color = QColor(0, 255, 0);
        int fontSizePx = 20;
        AlignH alignH = AlignH::Left;
        AlignV alignV = AlignV::Top;
    };

    struct Effects {
        bool   glow            = true;
        bool   scanlines       = true;
        bool   vignette        = true;
        double flickerStrength = 0.03;
    };

    explicit ConsoleWidget(QWidget* parent = nullptr);

    void clearBuffer();
    void setStyle(const Style& style);
    const Style&   style()   const { return m_style; }
    void setEffects(const Effects& effects);
    const Effects& effects() const { return m_effects; }

    void setText(const QString& text);
    void appendText(const QString& text);
    void appendLine(const QString& line);

signals:
    void keyPressed(int qtKey);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    QRect computeTextRect(const QSize& textSize) const;
    void  rebuildOverlays();

    Style   m_style{};
    Effects m_effects{};

    QTimer  m_flickerTimer;
    double  m_flickerFactor = 1.0;

    QStringList m_lines{};
    QString     m_currentLine{};
    int         m_maxStoredLines = 2000;
    QFont       m_font{};

    QPixmap m_scanlinesCache;
    QPixmap m_vignetteCache;
};
