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
 * File: Main.cpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Data/AppData.hpp"
#include "GUI/MainWindow.hpp"

#include <chrono>
#include <exception>
#include <fstream>
#include <iomanip>
#include <locale>
#include <mutex>
#include <sstream>
#include <string>
#include <windows.h>


// #define DEBUG
#ifdef DEBUG
static std::wofstream g_debugFile;
static std::mutex g_logMutex;
#endif

void InitDebug()
{
#ifdef DEBUG
    using namespace std::chrono;

    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);

    struct tm timeinfo;
    localtime_s(&timeinfo, &t);

    std::wostringstream filename;
    filename << L"debug_"
             << std::put_time(&timeinfo, L"%Y%m%d_%H%M%S")
             << L".txt";

    g_debugFile.open(filename.str().c_str(), std::ios::out | std::ios::app);
    g_debugFile.imbue(std::locale(""));
#endif
}

std::wstring GetTimestamp()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t t = system_clock::to_time_t(now);

    struct tm timeinfo;
    localtime_s(&timeinfo, &t);

    std::wostringstream oss;
    oss << std::put_time(&timeinfo, L"[%d.%m.%y %H:%M:%S")
        << L"." << std::setw(3) << std::setfill(L'0') << ms.count() << L"] ";

    return oss.str();
}

void DebugLog([[maybe_unused]] const std::wstring& text)
{
#ifdef DEBUG
    std::lock_guard<std::mutex> lock(g_logMutex);

    if (g_debugFile.is_open())
    {
        g_debugFile << GetTimestamp() << text << std::endl;
    }
#endif
}

void DebugLog([[maybe_unused]] LPCWSTR text)
{
#ifdef DEBUG
    std::lock_guard<std::mutex> lock(g_logMutex);

    if (g_debugFile.is_open())
    {
        if (text)
            g_debugFile << GetTimestamp() << text << std::endl;
        else
            g_debugFile << GetTimestamp() << L"(null)" << std::endl;
    }
#endif
}

void ShutdownDebug()
{
#ifdef DEBUG
    if (g_debugFile.is_open())
    {
        g_debugFile.flush();
        g_debugFile.close();
    }
#endif
}

namespace
{
std::wstring describeExceptionCode(DWORD code)
{
  switch (code)
  {
    case EXCEPTION_ACCESS_VIOLATION:
      return L"Access violation (invalid memory read/write).";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      return L"Array bounds exceeded.";
    case EXCEPTION_BREAKPOINT:
      return L"Breakpoint exception.";
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      return L"Datatype misalignment.";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      return L"Floating-point divide by zero.";
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      return L"Illegal instruction.";
    case EXCEPTION_IN_PAGE_ERROR:
      return L"In-page I/O error while accessing memory.";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      return L"Integer divide by zero.";
    case EXCEPTION_STACK_OVERFLOW:
      return L"Stack overflow.";
    default:
      return L"Unhandled structured exception.";
  }
}

std::wstring toHex(DWORD value)
{
  wchar_t buffer[16] {};
  swprintf_s(buffer, L"0x%08lX", value);
  return std::wstring(buffer);
}

std::wstring toHexPtr(std::uintptr_t value)
{
  wchar_t buffer[32] {};
  swprintf_s(buffer, L"0x%p", reinterpret_cast<void*>(value));
  return std::wstring(buffer);
}

LONG WINAPI topLevelExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
  if (exceptionInfo == nullptr || exceptionInfo->ExceptionRecord == nullptr)
  {
    MessageBoxW(nullptr,
                L"WindowsApp crashed with an unknown exception.",
                L"Fatal Error",
                MB_ICONERROR | MB_OK);
    return EXCEPTION_EXECUTE_HANDLER;
  }

  const DWORD code = exceptionInfo->ExceptionRecord->ExceptionCode;
  const auto address = reinterpret_cast<std::uintptr_t>(
    exceptionInfo->ExceptionRecord->ExceptionAddress);

  std::wostringstream message;
  message << L"WindowsApp encountered a fatal error and must close.\n\n"
          << L"Exception Code: " << toHex(code) << L"\n"
          << L"Address: " << toHexPtr(address) << L"\n"
          << L"Details: " << describeExceptionCode(code) << L"\n\n"
          << L"Please include this information when reporting the issue.";

  MessageBoxW(nullptr, message.str().c_str(), L"Fatal Error", MB_ICONERROR | MB_OK);
  return EXCEPTION_EXECUTE_HANDLER;
}

[[noreturn]] void terminateHandler()
{
  std::wstring reason = L"Unhandled C++ exception.";

  try
  {
    std::exception_ptr current = std::current_exception();
    if (current)
    {
      std::rethrow_exception(current);
    }
  }
  catch (const std::exception& ex)
  {
    const std::string whatText = ex.what();
    reason = std::wstring(whatText.begin(), whatText.end());
  }
  catch (...)
  {
    reason = L"Unhandled non-standard C++ exception.";
  }

  std::wstring message =
    L"WindowsApp terminated unexpectedly.\n\nReason: " + reason +
    L"\n\nPlease include this information when reporting the issue.";
  MessageBoxW(nullptr, message.c_str(), L"Fatal Error", MB_ICONERROR | MB_OK);
  std::abort();
}
}

int WINAPI wWinMain(
  _In_     HINSTANCE instance,
  _In_opt_ HINSTANCE /*prevInstance*/,
  _In_     LPWSTR    /*cmdLine*/,
  _In_     int       showCmd)
{
  InitDebug();
  DebugLog(L"Application started");
  int appResult = 0;

  SetUnhandledExceptionFilter(topLevelExceptionFilter);
  std::set_terminate(terminateHandler);

  try
  {
    AppData&  appData = AppData::getInstance();
    MainWindow window;

    if (!window.create(instance, appData))
    {
      LPCWSTR errorMessage {L"Failed to create the main window."};
      DebugLog(std::wstring(errorMessage));
      MessageBoxW(nullptr, errorMessage, L"Error", MB_ICONERROR | MB_OK);
      appResult = 1;
    }
    else
    {
      appResult = window.run(showCmd);
      DebugLog(L"Application finished gracefully");
    }
  }
  catch (const std::exception& ex)
  {
    const std::string whatText = ex.what();
    const std::wstring message =
      L"WindowsApp failed due to an unhandled C++ exception.\n\n"
      L"Details: " + std::wstring(whatText.begin(), whatText.end());
    DebugLog(message);
    MessageBoxW(nullptr, message.c_str(), L"Fatal Error", MB_ICONERROR | MB_OK);
    appResult = 1;
  }
  catch (...)
  {
    const std::wstring errorMessage{L"WindowsApp failed due to an unknown unhandled exception."};
    DebugLog(errorMessage);
    MessageBoxW(nullptr,
                errorMessage.c_str(),
                L"Fatal Error",
                MB_ICONERROR | MB_OK);
    appResult = 1;
  }
  
  ShutdownDebug();
  return appResult;
}
