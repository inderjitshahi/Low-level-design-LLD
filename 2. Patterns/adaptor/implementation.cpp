#include <bits/stdc++.h>
using namespace std;

/*
Adaptor Pattern (structural pattern because creates new structure: Client ---> Adapter ---> XmlLogger)
Intent: Convert the interface of a class into another interface that the client expects.
so no modification in existing class/interface, preserving ocp principle

Key: Inherit the interface you want to use, and Compose (wrap) the object you actually have.
*/

// target interface, required
class JsonLogger
{
public:
    virtual void logJson(const string &message) = 0;
    virtual ~JsonLogger() {};
};

// existing incompatible class, Adaptee
class XmlLogger
{
public:
    void logXml(const string &message)
    {
        cout << "<log>" << message << "</log>" << endl;
    }
};

// Adaptor
class XmlToJsonLoggerAdaptor : public JsonLogger  //using JsonLogger as interface
{
    // the goal is to make two incompatible things talk to each other by "wrapping" the old one in a way that fits the new system.
    // So, JsonLogger inherited and XmlLogger composed
    // Inherit the interface you want to use, and Compose (wrap) the object you actually have.
private:
    XmlLogger *xmlLogger;

public:
    XmlToJsonLoggerAdaptor(XmlLogger *x) : xmlLogger(x) {} //copy constructor
    void logJson(const string &message) override
    {
        // Translation logic
        string convertedMessage = "{ \"log\": \"" + message + "\" }";
        xmlLogger->logXml(convertedMessage);
    }
};

int main()
{

    XmlLogger xmlLogger;
    JsonLogger *logger = new XmlToJsonLoggerAdaptor(&xmlLogger);  
    logger->logJson("Adapter Pattern Applied");

    delete logger;
    return 0;
}