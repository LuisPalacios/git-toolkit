//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <mutex>

namespace gittoolkit {

/// Log severity levels.
enum class GtkLogLevel {
    Debug,
    Info,
    Warn,
    Error
};

/// Thread-safe leveled logger. Outputs to stderr.
/// Set the level once at startup; all messages below that level are suppressed.
class GtkLog {
public:
    /// Get the global logger instance.
    static GtkLog& Instance();

    /// Set minimum log level. Messages below this are suppressed.
    void SetLevel(GtkLogLevel level);

    /// Get current log level.
    GtkLogLevel GetLevel() const;

    /// Enable or disable colored output.
    void SetColorEnabled(bool enabled);

    /// Log a message at the given level.
    void Log(GtkLogLevel level, const std::string& message);

    /// Convenience methods.
    void Debug(const std::string& message);
    void Info(const std::string& message);
    void Warn(const std::string& message);
    void Error(const std::string& message);

private:
    GtkLog();
    ~GtkLog() = default;
    GtkLog(const GtkLog&) = delete;
    GtkLog& operator=(const GtkLog&) = delete;

    static const char* LevelToString(GtkLogLevel level);
    static const char* LevelToColor(GtkLogLevel level);
    bool IsTerminal() const;

    mutable std::mutex _mutex;
    GtkLogLevel _level = GtkLogLevel::Info;
    bool _colorEnabled = true;
};

// Global convenience macros-like inline functions
inline void LogDebug(const std::string& msg) { GtkLog::Instance().Debug(msg); }
inline void LogInfo(const std::string& msg) { GtkLog::Instance().Info(msg); }
inline void LogWarn(const std::string& msg) { GtkLog::Instance().Warn(msg); }
inline void LogError(const std::string& msg) { GtkLog::Instance().Error(msg); }

} // namespace gittoolkit
