/* 
 * Copyright (C) 2026 Arno Saxena
 *
 * Atlantis Majordomo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * File: DebugLog.cpp
 */

// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65

#include "DebugLog.hpp"

// #define DEBUG
#ifdef DEBUG

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <locale>
#include <mutex>
#include <sstream>

namespace
{
std::wofstream g_debugFile;
std::mutex g_logMutex;

std::tm toLocalTime(const std::time_t t)
{
	std::tm timeInfo {};
#ifdef _WIN32
	localtime_s(&timeInfo, &t);
#else
	localtime_r(&t, &timeInfo);
#endif
	return timeInfo;
}

std::wstring getTimestamp()
{
	using namespace std::chrono;

	const auto now = system_clock::now();
	const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

	const std::time_t t = system_clock::to_time_t(now);
	const std::tm timeInfo = toLocalTime(t);

	std::wostringstream oss;
	oss << std::put_time(&timeInfo, L"[%d.%m.%y %H:%M:%S")
			<< L"." << std::setw(3) << std::setfill(L'0') << ms.count() << L"] ";
	return oss.str();
}
}

#endif

void InitDebug()
{
#ifdef DEBUG
	using namespace std::chrono;

	const auto now = system_clock::now();
	const std::time_t t = system_clock::to_time_t(now);
	const std::tm timeInfo = toLocalTime(t);

	std::wostringstream filename;
	filename << L"debug_"
					 << std::put_time(&timeInfo, L"%Y%m%d_%H%M%S")
					 << L".txt";

	std::lock_guard<std::mutex> lock(g_logMutex);
	g_debugFile.open(filename.str().c_str(), std::ios::out | std::ios::app);
	g_debugFile.imbue(std::locale(""));
#endif
}

void DebugLog([[maybe_unused]] const std::wstring& text)
{
#ifdef DEBUG
	std::lock_guard<std::mutex> lock(g_logMutex);
	if (g_debugFile.is_open())
	{
		g_debugFile << getTimestamp() << text << std::endl;
	}
#endif
}

void DebugLog([[maybe_unused]] const wchar_t* text)
{
#ifdef DEBUG
	std::lock_guard<std::mutex> lock(g_logMutex);
	if (g_debugFile.is_open())
	{
		if (text)
		{
			g_debugFile << getTimestamp() << text << std::endl;
		}
		else
		{
			g_debugFile << getTimestamp() << L"(null)" << std::endl;
		}
	}
#endif
}

void ShutdownDebug()
{
#ifdef DEBUG
	std::lock_guard<std::mutex> lock(g_logMutex);
	if (g_debugFile.is_open())
	{
		g_debugFile.flush();
		g_debugFile.close();
	}
#endif
}
