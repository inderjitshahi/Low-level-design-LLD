#include <iostream>
#include <string>
#include <mutex>
#include <fstream> // For file output
#include <chrono>  // For high-resolution time
#include <ctime>   // For converting time to string
#include <iomanip> // For formatting the time output put_time

using namespace std;

// This Logger is a centralized communication center for your application.
// Enum Class, also known as a Scoped Enumeration, introduced in C++11.
// Enums are used when a variable needs to be one of a fixed set of possible values.
// Traditional enum is leaky, unsafe, and implicitly converts to integers. Modern enum class is strongly typed, safely scoped, and prevents a lot of hidden bugs. enum Color { RED, GREEN, BLUE };
enum class LogLevel
{
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
};

class Logger
{
private:
    // mutex mtx; // std::mutex is a synchronization tool used to prevent Race Conditions.
    LogLevel currentLevel;
    ofstream logFile; // File stream object

    // private constructor, prevents object creation
    Logger() : currentLevel(LogLevel::INFO)
    {
        // Open file in "Append" mode so we don't delete old logs
        logFile.open("app.log", ios::app);
        if (!logFile.is_open())
        {
            cerr << "Failed to open log file!" << endl;
        }
    }

    // Helper to get current timestamp as a string
    string getTimestamp()
    {
        auto now = chrono::system_clock::now();                // a template class,  A "Duration" (a count of ticks) since a specific "Clock's" epoch.
        auto in_time_t = chrono::system_clock::to_time_t(now); // return time_t, which is basically number of seconds elapsed since unix epoch

        stringstream ss;
        // Formats time as [YYYY-MM-DD HH:MM:SS]
        ss << "[" << put_time(localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "] ";
        return ss.str();
    }

    // prevent copy
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

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

public:
    static Logger &getInstance() // return by reference only, otherwise copy constructor is called that will cause error
    {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level)
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

    // Destructor to close the file properly
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
    // singleton for logger, otherwise each file / module creates its own Logger object. Single instance give app wise single logging system
    Logger &logger = Logger::getInstance(); // Logger logger=Logger::getInstance(); will create copy, any modification will not modify the global object

    logger.setLogLevel(LogLevel::DEBUG);
    logger.debug("Debugging application");
    logger.info("Application started");
    logger.warn("Low memory warning");
    logger.error("Unhandled exception occurred");
    return 0;
}