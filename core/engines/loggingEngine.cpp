///// Vermilion Logging

#include "loggingEngine.h"
#include <string>

Vermilion::LogEngine::LogEngine(unsigned int startState, unsigned int nLogLevel) : state(startState), logLevel(nLogLevel)
{
    // Open logging file
    this->logOut.open("VermilionLog.txt", std::ios::app);
}

Vermilion::LogEngine::LogEngine(std::string fName, unsigned int startState, unsigned int nLogLevel) : state(startState), logLevel(nLogLevel)
{
	// Open logging file
	this->logOut.open(fName, std::ios::app);
}

Vermilion::LogEngine::~LogEngine()
{
	logOut << std::endl;
    this->logOut.close();
}

//#pragma region C Style Logging
///////// 
//// C Style Logging
////
//
///// Custom C Style + Base for all other calls
//void Vermilion::LogEngine::log(char* level, char* message)
//{
//    switch(this->state)
//    {
//    case VermiLogFile:
//        this->logOut << "[" << level << "] " << message << std::endl;
//        break;
//    case VermiLogStd:
//        std::cout << "[" << level << "] " << message << std::endl;
//        break;
//    case VermiLogBoth:
//        this->logOut << "[" << level << "] " << message << std::endl;
//        std::cout << "[" << level << "] " << message << std::endl;
//        break;
//    default:
//        break;
//    }
//}
//
///// Log Info
//void Vermilion::LogEngine::logInfo(char* message)
//{
//	if(this->logLevel < VermiLogLevelWarn) log("INFO",message);
//}
//
///// Log Warning
//void Vermilion::LogEngine::logWarn(char* message)
//{
//	if (this->logLevel < VermiLogLevelError) log("WARN", message);
//}
//
///// Log Error
//void Vermilion::LogEngine::logError(char* message)
//{
//	if (this->logLevel < VermiLogLevelFatal) log("ERROR", message);
//}
//
///// Log Fatal
//void Vermilion::LogEngine::logFatal(char* message)
//{
//	if(this->logLevel < VermiLogLevelNone) log("FATAL", message);
//}
//#pragma endregion

#pragma region C++ Style Logging

/// Custom C++ Style
void Vermilion::LogEngine::log(std::string level, std::string message)
{
	// Call our C Style equivalent
	switch (this->state)
	{
	case VermiLogFile:
		this->logOut << "[" << level << "] " << message << std::endl;
		break;
	case VermiLogStd:
		std::cout << "[" << level << "] " << message << std::endl;
		break;
	case VermiLogBoth:
		this->logOut << "[" << level << "] " << message << std::endl;
		std::cout << "[" << level << "] " << message << std::endl;
		break;
	default:
		break;
	}
}

/// Log Info
void Vermilion::LogEngine::logInfo(std::string message)
{
	if (this->logLevel < VermiLogLevelWarn) log("INFO", message);
}

/// Log Warning
void Vermilion::LogEngine::logWarn(std::string message)
{
	if (this->logLevel < VermiLogLevelError) log("WARN", message);
}

/// Log Error
void Vermilion::LogEngine::logError(std::string message)
{
	if (this->logLevel < VermiLogLevelFatal) log("ERROR", message);
}

/// Log Fatal
void Vermilion::LogEngine::logFatal(std::string message)
{
	if (this->logLevel < VermiLogLevelNone) log("FATAL", message);
}
#pragma endregion

void Vermilion::LogEngine::updateState(unsigned int newState)
{
    this->state = newState;
}

void Vermilion::LogEngine::updateLogLevel(unsigned nLogLevel)
{
	this->logLevel = nLogLevel;
}

