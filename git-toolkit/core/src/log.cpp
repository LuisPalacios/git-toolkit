//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "gittoolkit/log.h"

#include <iostream>
#include <format>

#ifdef _WIN32
#include <io.h>
#define IS_TTY(fd) _isatty(fd)
#define STDERR_FD _fileno(stderr)
#else
#include <unistd.h>
#define IS_TTY(fd) isatty(fd)
#define STDERR_FD fileno(stderr)
#endif

namespace gittoolkit {

GtkLog& GtkLog::Instance() {
    static GtkLog instance;
    return instance;
}

GtkLog::GtkLog() {
    _colorEnabled = IsTerminal();
}

void GtkLog::SetLevel(GtkLogLevel level) {
    std::scoped_lock lock(_mutex);
    _level = level;
}

GtkLogLevel GtkLog::GetLevel() const {
    std::scoped_lock lock(_mutex);
    return _level;
}

void GtkLog::SetColorEnabled(bool enabled) {
    std::scoped_lock lock(_mutex);
    _colorEnabled = enabled;
}

void GtkLog::Log(GtkLogLevel level, const std::string& message) {
    std::scoped_lock lock(_mutex);
    if (level < _level) {
        return;
    }

    if (_colorEnabled) {
        std::cerr << std::format("{}[{}]\033[0m {}\n",
            LevelToColor(level), LevelToString(level), message);
    } else {
        std::cerr << std::format("[{}] {}\n", LevelToString(level), message);
    }
}

void GtkLog::Debug(const std::string& message) { Log(GtkLogLevel::Debug, message); }
void GtkLog::Info(const std::string& message) { Log(GtkLogLevel::Info, message); }
void GtkLog::Warn(const std::string& message) { Log(GtkLogLevel::Warn, message); }
void GtkLog::Error(const std::string& message) { Log(GtkLogLevel::Error, message); }

const char* GtkLog::LevelToString(GtkLogLevel level) {
    switch (level) {
        case GtkLogLevel::Debug: return "DEBUG";
        case GtkLogLevel::Info:  return "INFO";
        case GtkLogLevel::Warn:  return "WARN";
        case GtkLogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

const char* GtkLog::LevelToColor(GtkLogLevel level) {
    switch (level) {
        case GtkLogLevel::Debug: return "\033[36m";  // cyan
        case GtkLogLevel::Info:  return "\033[32m";  // green
        case GtkLogLevel::Warn:  return "\033[33m";  // yellow
        case GtkLogLevel::Error: return "\033[31m";  // red
    }
    return "\033[0m";
}

bool GtkLog::IsTerminal() const {
    return IS_TTY(STDERR_FD) != 0;
}

} // namespace gittoolkit
