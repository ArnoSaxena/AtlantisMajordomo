/*
 * Copyright (C) 2026 Arno Saxena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3 or later.
 */
#pragma once

#include <filesystem>
#include <string>

namespace TestUtils
{
  /**
   * @brief Gets the absolute path to a test data file using relative path resolution.
   * 
   * This function resolves test data files relative to the executable's location.
   * It searches for the testData folder by walking up from the build directory.
   * 
   * @param relativeFileName Filename relative to the testData folder
   * @return Absolute path to the test data file
   */
  inline std::string getTestDataPath(const std::string& relativeFileName)
  {
    using namespace std::filesystem;
    
    // Get the directory of the running executable
    std::error_code ec;
    path exePath = canonical(current_path(), ec);
    
    if (ec)
    {
      return "";
    }
    
    // Search upward for testData folder (up to 5 levels)
    for (int i = 0; i < 5; ++i)
    {
      path testDataPath = exePath / "testData" / relativeFileName;
      if (exists(testDataPath, ec))
      {
        return testDataPath.string();
      }
      
      exePath = exePath.parent_path();
      if (exePath == exePath.parent_path())
      {
        break; // Reached filesystem root
      }
    }
    
    return "";
  }

  /**
   * @brief Gets the absolute path to the testData directory using relative path resolution.
   * 
   * @return Absolute path to the testData directory, or empty string if not found
   */
  inline std::string getTestDataDir()
  {
    using namespace std::filesystem;
    
    std::error_code ec;
    path exePath = canonical(current_path(), ec);
    
    if (ec)
    {
      return "";
    }
    
    // Search upward for testData folder (up to 5 levels)
    for (int i = 0; i < 5; ++i)
    {
      path testDataPath = exePath / "testData";
      if (exists(testDataPath, ec) && is_directory(testDataPath, ec))
      {
        return testDataPath.string();
      }
      
      exePath = exePath.parent_path();
      if (exePath == exePath.parent_path())
      {
        break; // Reached filesystem root
      }
    }
    
    return "";
  }
}
