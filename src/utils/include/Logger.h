//  工具类头文件
#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>

//  日志级别枚举
enum class LogLevel {
	DEBUG,
	INFO,
	WARNING,
	ERROR
};

//  日志类声明
class Logger {
public:
	static Logger& GetInstance();

	void SetLogFile(const std::string& filename);
	void SetLogLevel(LogLevel level);

	void Debug(const std::string& message);
	void Info(const std::string& message);
	void Warning(const std::string& message);
	void Error(const std::string& message);

private:
	Logger();
	~Logger();

	std::ofstream logFile;
	LogLevel currentLevel;
	std::mutex logMutex;

	void Log(LogLevel level, const std::string& message);
	std::string LevelToString(LogLevel level);
};//Logger