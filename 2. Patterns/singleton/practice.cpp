#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <string>
#include <iomanip>
using namespace std;

enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger
{
private:
    // mutex mtx;
    LogLevel currentLevel;
    ofstream logFile;
    Logger()
    {
        currentLevel = LogLevel::DEBUG;
        logFile.open("app.log", ios::app);
    }

    string getTimestamp()
    {
        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << "[" << put_time(localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "] ";
        return ss.str();
    }

    // Internal logging
    void logInternal(LogLevel level, const string &message)
    {
        if (level < currentLevel)
            return;

        // lock_guard<mutex> lock(mtx); // It looks at the mtx. If the mutex is already locked by another thread, this thread pauses (blocks) and waits right here.

        string logLine = getTimestamp();

        switch (level)
        {
        case LogLevel::DEBUG:
            logLine += "[DEBUG]: ";
            break;
        case LogLevel::INFO:
            logLine += "[INFO]: ";
            break;
        case LogLevel::WARN:
            logLine += "[WARN]: ";
            break;
        case LogLevel::ERROR:
            logLine += "[ERROR]: ";
            break;
        }
        logLine += message;

        // Write to BOTH Console and File
        cout << logLine << endl;
        if (logFile.is_open())
        {
            logFile << logLine << endl;
        }
    }

    Logger(const Logger &l) = delete;
    Logger &operator=(const Logger &l) = delete;

public:
    static Logger &getInstance()
    {
        static Logger logger;
        return logger;
    }

    void setCurrentLevel(LogLevel level)
    {
        currentLevel = level;
    }

    void debug(const string &message)
    {
        logInternal(LogLevel::DEBUG, message);
    }

    void info(const std::string &message)
    {
        logInternal(LogLevel::INFO, message);
    }

    void warn(const std::string &message)
    {
        logInternal(LogLevel::WARN, message);
    }

    void error(const std::string &message)
    {
        logInternal(LogLevel::ERROR, message);
    }
    ~Logger()
    {
        if (logFile.is_open())
        {
            logFile.close();
        }
    }
};

int main()
{
    Logger &logger = Logger::getInstance();
    logger.setCurrentLevel(LogLevel::DEBUG);
    logger.debug("Debugging application");
    logger.info("Application started");
    logger.warn("Low memory warning");
    logger.error("Unhandled exception occurred");
    return 0;
}