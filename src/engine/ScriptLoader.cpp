#include "ScriptLoader.h"

#include <QFile>

#include <yaml-cpp/yaml.h>

#include <sstream>
#include <stdexcept>

namespace pc {

static std::runtime_error makeError(const QString& where, const std::string& what) {
    std::ostringstream oss;
    oss << where.toStdString() << ": " << what;
    return std::runtime_error(oss.str());
}

static AlignH parseAlignH(const std::string& s) {
    if (s == "left") return AlignH::Left;
    if (s == "center") return AlignH::Center;
    if (s == "right") return AlignH::Right;
    throw std::runtime_error("Invalid alignH value: " + s + " (expected: left|center|right)");
}

static AlignV parseAlignV(const std::string& s) {
    if (s == "top") return AlignV::Top;
    if (s == "middle") return AlignV::Middle;
    if (s == "bottom") return AlignV::Bottom;
    throw std::runtime_error("Invalid alignV value: " + s + " (expected: top|middle|bottom)");
}

static QColor parseColor(const std::string& s) {
    // Expect "#RRGGBB"
    const QString qs = QString::fromStdString(s);
    if (!QColor::isValidColor(qs)) {
        throw std::runtime_error("Invalid color: " + s + " (expected a Qt-valid color like #RRGGBB)");
    }
    return QColor(qs);
}

static void parseDefaults(const YAML::Node& root, Script& out) {
    const YAML::Node defaults = root["defaults"];
    if (!defaults) {
        // defaults are optional; we keep hardcoded defaults
        return;
    }

    // style
    if (const YAML::Node style = defaults["style"]) {
        if (const YAML::Node color = style["color"]) {
            out.defaults.style.color = parseColor(color.as<std::string>());
        }
        if (const YAML::Node size = style["size"]) {
            out.defaults.style.sizePx = size.as<int>();
        }
        if (const YAML::Node alignH = style["alignH"]) {
            out.defaults.style.alignH = parseAlignH(alignH.as<std::string>());
        }
        if (const YAML::Node alignV = style["alignV"]) {
            out.defaults.style.alignV = parseAlignV(alignV.as<std::string>());
        }
    }

    // type
    if (const YAML::Node type = defaults["type"]) {
        if (const YAML::Node cd = type["charDelay"]) {
            out.defaults.type.charDelaySeconds = cd.as<double>();
        }
    }
}

static Step parseOneStep(const YAML::Node& stepNode) {
    if (!stepNode || !stepNode.IsMap() || stepNode.size() != 1) {
        throw std::runtime_error("Each step must be a map with exactly one key (e.g. {write: \"...\"})");
    }

    const auto it = stepNode.begin();
    const std::string key = it->first.as<std::string>();
    const YAML::Node value = it->second;

    if (key == "write") {
        if (!value || !value.IsScalar()) {
            throw std::runtime_error("write step must be a string");
        }
        return WriteStep{QString::fromStdString(value.as<std::string>())};
    }

    if (key == "wait") {
        if (!value || !value.IsScalar()) {
            throw std::runtime_error("wait step must be a number (seconds)");
        }
        return WaitStep{value.as<double>()};
    }

    if (key == "clear") {
        // We accept: clear: {} or clear: null
        return ClearStep{};
    }

    if (key == "set_style") {
        if (!value || !value.IsMap()) {
            throw std::runtime_error("set_style step must be a map (e.g. {color: \"#ff0000\", size: 50, ...})");
        }

        Style s{};
        if (const YAML::Node color = value["color"]) {
            s.color = parseColor(color.as<std::string>());
        }
        if (const YAML::Node size = value["size"]) {
            s.sizePx = size.as<int>();
        }
        if (const YAML::Node alignH = value["alignH"]) {
            s.alignH = parseAlignH(alignH.as<std::string>());
        }
        if (const YAML::Node alignV = value["alignV"]) {
            s.alignV = parseAlignV(alignV.as<std::string>());
        }
        return SetStyleStep{s};
    }

    if (key == "pause_until_key") {
        if (!value || !value.IsMap()) {
            throw std::runtime_error("pause_until_key step must be a map (e.g. {key: \"Space\"})");
        }
        const YAML::Node k = value["key"];
        if (!k || !k.IsScalar()) {
            throw std::runtime_error("pause_until_key requires a string field: key");
        }
        return PauseUntilKeyStep{QString::fromStdString(k.as<std::string>())};
    }

    throw std::runtime_error("Unknown step type: " + key);
}

Script ScriptLoader::loadFromResource(const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(("Failed to open resource: " + resourcePath).toStdString());
    }

    const QByteArray bytes = file.readAll();
    if (bytes.isEmpty()) {
        throw std::runtime_error(("Resource is empty: " + resourcePath).toStdString());
    }

    Script script{};

    try {
        const YAML::Node root = YAML::Load(bytes.constData());

        parseDefaults(root, script);

        const YAML::Node steps = root["steps"];
        if (!steps || !steps.IsSequence()) {
            throw std::runtime_error("Root field 'steps' must be a YAML sequence");
        }

        script.steps.reserve(steps.size());
        for (const auto& stepNode : steps) {
            script.steps.push_back(parseOneStep(stepNode));
        }
    } catch (const YAML::ParserException& e) {
        throw makeError(resourcePath, std::string("YAML parse error: ") + e.what());
    } catch (const YAML::Exception& e) {
        throw makeError(resourcePath, std::string("YAML error: ") + e.what());
    } catch (const std::exception& e) {
        throw makeError(resourcePath, e.what());
    }

    return script;
}

} // namespace pc
