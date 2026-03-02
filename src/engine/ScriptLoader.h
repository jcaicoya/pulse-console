#pragma once

#include <QString>

#include "Steps.h"

namespace pc {

    class ScriptLoader final {
    public:
        // Loads and parses a YAML script embedded as a Qt resource, e.g. ":/scripts/default.yaml".
        // Throws std::runtime_error on failure.
        static Script loadFromResource(const QString& resourcePath);

        // Loads and parses a YAML script from disk.
        // Throws std::runtime_error on failure.
        static Script loadFromFile(const QString& filePath);
    };

} // namespace pc
