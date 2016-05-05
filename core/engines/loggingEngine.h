///// Vermilion Logging
#ifndef VERMILION_LOGGING_ENGINE_H
#define VERMILION_LOGGING_ENGINE_H

#include <fstream>
#include <iostream>
#include <string.h>

namespace Vermilion
{

	enum LogLevel
	{
		// Show all logs
		VermiLogLevelAll = 0,

		// Suppress Info
		VermiLogLevelWarn = 1,

		//Suppress below Warn
		VermiLogLevelError = 2,
		
		// Suppress below Error
		VermiLogLevelFatal = 3,

		// Don't Log
		VermiLogLevelNone = 4,

		// Total levels
		VermiLogLevelCount
	};

	enum LogState
	{
		VermiLogFile = 0,
		VermiLogStd = 1,
		VermiLogBoth = 2,

		// Total States
		VermiLogTotalStates
	};

	class LogEngine
	{
		unsigned int state;
		unsigned int logLevel;
		std::ofstream logOut;


		//private:
		//void internLog();

	public:
		LogEngine(unsigned int startState = VermiLogFile, unsigned int nLogLevel = VermiLogLevelAll);
		LogEngine(std::string logName, unsigned int startState = VermiLogFile, unsigned int nLogLevel = VermiLogLevelAll);
		~LogEngine();

		// Logging methods for C style *char's
		//void log(char* level, char* message); // Custom Levels
		//void logInfo(char* message);
		//void logWarn(char* message);
		//void logError(char* message);
		//void logFatal(char* message);

		// Logging methods for C++ style string
		void log(std::string level, std::string message); // Custom Levels
		void logInfo(std::string message);
		void logWarn(std::string message);
		void logError(std::string message);
		void logFatal(std::string message);

		void updateState(unsigned int newState);
		void updateLogLevel(unsigned int nLogLevel);
	};
}

#endif // End VERMILION_LOGGING_ENGINE_H
