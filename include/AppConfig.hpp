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
 * File: AppConfig.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

/**
* @brief Stores and persists application-level configuration.
*
* The configuration file is always located next to the executable.
* Unknown JSON entries are ignored. Missing entries fall back to
* deterministic internal defaults.
*/
class AppConfig
{
public:
  AppConfig();

  /**
  * @brief Loads configuration from disk or creates defaults when missing.
  * @return true when load/create succeeded.
  */
  bool load();

  /**
  * @brief Persists the current configuration in stable key order.
  * @return true when save succeeded.
  */
  bool save() const;

  /**
  * @brief Gets the configured default save file path.
  */
  const std::wstring& getSaveFilePath() const;

  /**
  * @brief Sets the configured default save file path.
  */
  void setSaveFilePath(const std::wstring& saveFilePath);

  /**
  * @brief Gets the configured default report import folder.
  */
  const std::wstring& getReportImportFolder() const;

  /**
  * @brief Sets the configured default report import folder.
  */
  void setReportImportFolder(const std::wstring& reportImportFolder);

  /**
  * @brief Gets the configured default export orders folder.
  */
  const std::wstring& getExportOrdersFolder() const;

  /**
  * @brief Sets the configured default export orders folder.
  */
  void setExportOrdersFolder(const std::wstring& exportOrdersFolder);

    /**
    * @brief Gets the configured main window width.
    */
    int getMainWindowWidth() const;

    /**
    * @brief Gets the configured main window height.
    */
    int getMainWindowHeight() const;

    /**
    * @brief Sets the configured main window width.
    */
    void setMainWindowWidth(int width);

    /**
    * @brief Sets the configured main window height.
    */
    void setMainWindowHeight(int height);

    /**
    * @brief Gets the configured map hex width.
    */
    int getMapHexWidth() const;

    /**
    * @brief Sets the configured map hex width.
    */
    void setMapHexWidth(int mapHexWidth);

    /**
    * @brief Gets whether only leaders may teach.
    */
    bool getOnlyLeaderCanTeach() const;

    /**
    * @brief Gets whether magic study requires leader mages.
    */
    bool getLeaderMages() const;

    /**
    * @brief Gets configured flying ship tokens as CSV.
    */
    const std::wstring& getFlyingShipsCsv() const;

    /**
    * @brief Sets configured flying ship tokens as CSV.
    */
    void setFlyingShipsCsv(const std::wstring& flyingShipsCsv);

    /**
    * @brief Gets configured full-month order keywords as CSV.
    */
    const std::wstring& getFullMonthOrdersCsv() const;

    /**
    * @brief Sets configured full-month order keywords as CSV.
    */
    void setFullMonthOrdersCsv(const std::wstring& fullMonthOrdersCsv);

    /**
    * @brief Gets configured magic skill trigger phrases as CSV.
    */
    const std::wstring& getMagicSkillTriggersCsv() const;

    /**
    * @brief Gets the configured initial data file path.
    */
    const std::wstring& getDataFilePath() const;

    /**
    * @brief Sets the configured initial data file path.
    */
    void setDataFilePath(const std::wstring& dataFilePath);

    /**
    * @brief Sets configured magic skill trigger phrases as CSV.
    */
    void setMagicSkillTriggersCsv(const std::wstring& magicSkillTriggersCsv);

    /**
    * @brief Sets whether only leaders may teach.
    */
    void setOnlyLeaderCanTeach(bool onlyLeaderCanTeach);

    /**
    * @brief Sets whether magic study requires leader mages.
    */
    void setLeaderMages(bool leaderMages);

    /**
    * @brief Returns configured RGB color for a region type.
    *
    * Falls back to the configured color for "unknown" when the type is not found.
    */
    std::array<int, 3> getRegionColor(const std::wstring& regionType) const;

    /**
    * @brief Gets the configured RGB color for the selected region's border.
    */
    std::array<int, 3> getSelectedRegionBorderColor() const;

    /**
    * @brief Gets the configured RGB color for road overlays.
    */
    std::array<int, 3> getRoadColor() const;

    /**
    * @brief Gets the configured RGB color for structure marker overlays.
    */
    std::array<int, 3> getStructureMarkerColor() const;

private:
  std::wstring configFilePath_;
  std::wstring saveFilePath_;
  std::wstring reportImportFolder_;
  std::wstring exportOrdersFolder_;
  std::wstring dataFilePath_;
    int mainWindowWidth_ { 900 };
    int mainWindowHeight_ { 600 };
    int mapHexWidth_ { 40 };
    bool onlyLeaderCanTeach_ { false };
    bool leaderMages_ { true };
  std::wstring flyingShipsCsv_;
  std::wstring fullMonthOrdersCsv_;
  std::wstring magicSkillTriggersCsv_;

  static std::wstring getExecutableDirectory();
  static std::wstring getDefaultSaveFilePath();
  static std::wstring getDefaultDataFilePath();
  static std::wstring getDefaultReportImportFolder();
  static std::wstring getDefaultExportOrdersFolder();
  /*
  static bool extractJsonFieldValue(const std::wstring& json,
                                    const std::wstring& fieldName,
                                    std::wstring& value);
  static bool extractJsonStringField(const std::wstring& json,
                                    const std::wstring& fieldName,
                                    std::wstring& value);
  static bool extractJsonObjectField(const std::wstring& json,
                                    const std::wstring& fieldName,
                                    std::wstring& objectContent);
  static bool parseJsonInteger(const std::wstring& jsonValue, int& parsedValue);
  static bool parseRgbColorArray(const std::wstring& jsonArray,
                                std::array<int, 3>& rgbColor);
*/

  void applyDefaults();

  std::vector<std::pair<std::wstring, std::array<int, 3>>> regionColors_;
  std::array<int, 3> roadColor_ { 0, 0, 0 };
  std::array<int, 3> structureMarkerColor_ { 0, 0, 0 };
  std::array<int, 3> selectedRegionBorderColor_ { 173, 216, 230 };
};
