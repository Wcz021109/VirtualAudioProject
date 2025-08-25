//  工具类实现文件
#include "Logger.h"
#include <chrono>
#include <iomanip>

Logger&Logger::GetInstance() {
	static Logger instance;
	return instance;
}

Logger:Logger() : currentLevel(LogLevel::INFO) {}

Logger::~Logger() {
	if (logFile.is_open()) {
		logFile.close();
	}
}

void Logger::SetLogFile(const std::string& filename) {
	std::lock_guard<std::mutex> lock(logMutex);
	if (logFile.is_open()) {
		logFile.close();
	}
	logFile.open(filename, std::ios::out | std::ios::app);
}

void Logger::SetLogLevel(LogLevel level) {
	std::lock_guard<std::mutex> lock(logMutex);
	currentLevel = level;
}

void Logger::Debug(const std::string& message) {
	if (currentLevel <= LogLevel::DEBUG) {
		Log(LogLevel::DEBUG, message);
	}
}

void Logger::Info(const std::string& message) {
	if (currentLevel <= LogLevel::INFO) {
		Log(LogLevel::INFO, message);
	}
}

void Logger::Warning(const std::string& message) {
	if (currentLevel <= LogLevel::WARNING) {
		Log(LogLevel::WARNING, message);
	}
}

void Logger::Error(const std::string& message) {
	if (currentLevel <= LogLevel::ERROR) {
		Log(LogLevel::ERROR, message);
	}
}

void Logger::Log(LogLevel level, const std::string& message) {
	std::lock_guard<std::mutex> lock(logMutex);

	//  获取当前时间
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	//  格式化当前时间
	std::stringstream oss;
	oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
		<< "." << std::setfill('0') << std::setw(3) << ms.count();

	//  构建日志信息
	std::string logMessage = "[" + oss.str() + "] [" + LevelToString(level) + "] " + message;

	//  输出到控制台
	std::cout << logMessage << std::endl;

	//  输出到日志文件（如果已设置）
	if (logFile.is_open()) {
		logFile << logMessage << std::endl;
	}
}//Log

std::string Logger::LevelToString(LogLevel level) {
	switch (level) {
	case LogLevel::DEBUG: return "DEBUG";
	case LogLevel::INFO: return "INFO";
	case LogLevel::WARNING: return "WARNING";
	case LogLevel::ERROR: return "ERROR";
	default: return "UNKNOWN";
	}
}