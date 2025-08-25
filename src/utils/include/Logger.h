//  ������ͷ�ļ�
#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>

//  ��־����ö��
enum class LogLevel {
	DEBUG,
	INFO,
	WARNING,
	ERROR
};

//  ��־������
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