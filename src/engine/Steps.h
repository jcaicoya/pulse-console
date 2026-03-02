#pragma once

#include <QString>
#include <QColor>
#include <variant>
#include <vector>

namespace pc {

    enum class AlignH { Left, Center, Right };
    enum class AlignV { Top, Middle, Bottom };

    struct Style {
        QColor color = QColor(0, 255, 0);
        int sizePx = 20;
        AlignH alignH = AlignH::Left;
        AlignV alignV = AlignV::Top;
    };

    struct TypeParams {
        double charDelaySeconds = 0.02;
    };

    struct WriteStep {
        QString text;
    };

    struct WaitStep {
        double seconds = 0.0;
    };

    struct ClearStep { };

    struct SetStyleStep {
        Style style;
    };

    struct PauseUntilKeyStep {
        QString key; // e.g. "Space", "Enter"
    };

    using Step = std::variant<WriteStep, WaitStep, ClearStep, SetStyleStep, PauseUntilKeyStep>;

    struct ScriptDefaults {
        Style style{};
        TypeParams type{};
    };

    struct Script {
        ScriptDefaults defaults{};
        std::vector<Step> steps{};
    };

} // namespace pc
