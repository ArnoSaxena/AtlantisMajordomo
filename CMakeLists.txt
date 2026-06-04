cmake_minimum_required(VERSION 3.20)
project(AtlantisMajordomo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(AtlantisMajordomo WIN32
  src/AppConfig.cpp
  src/Data/AppData.cpp
  src/Data/Commands.cpp
  src/Function/CommandSimulationService.cpp
  src/Function/CoordinateFormattingUtils.cpp
  src/Function/FactionAttitudeUtils.cpp
  src/Function/MonthUtils.cpp
  src/Function/OrderParsingUtils.cpp
  src/Function/OrderWarningService.cpp
  src/Function/OrderBusinessLogic.cpp
  src/Function/StringUtils.cpp
  src/Data/DataSerializer.cpp
  src/Data/Event.cpp
  src/Data/EventRepository.cpp
  src/Data/Battle.cpp
  src/Data/BattleRepository.cpp
  src/Data/Faction.cpp
  src/Data/FactionRepository.cpp
  src/Data/Item.cpp
  src/Data/ItemRepository.cpp
  src/Data/Skill.cpp
  src/Data/SkillRepository.cpp
  src/Data/Region.cpp
  src/Data/RegionRepository.cpp
  src/Data/Unit.cpp
  src/Data/UnitRepository.cpp
  src/Data/UnitNew.cpp
  src/Data/UnitNewRepository.cpp
  src/Data/Structure.cpp
  src/Data/StructureRepository.cpp
  src/Data/StructInfo.cpp
  src/Data/StructInfoRepository.cpp
  src/Data/Order.cpp
  src/Data/OrderRepository.cpp
  src/Main.cpp
  src/GUI/MainWindow.cpp
  src/GUI/EventsTabContent.cpp
  src/GUI/ReportsTabContent.cpp
  src/GUI/MapTabContent.cpp
  src/GUI/OrdersEditorUtils.cpp
  src/GUI/ItemsTabContent.cpp
  src/GUI/SkillsTabContent.cpp
  src/GUI/FactionsTabContent.cpp
  src/GUI/BattlesTabContent.cpp
  src/GUI/TabView.cpp
  src/GUI/SettingsDialog.cpp
  src/Data/Report.cpp
  src/Data/ReportRepository.cpp
)

target_include_directories(AtlantisMajordomo PRIVATE
  include
)

target_link_libraries(AtlantisMajordomo PRIVATE
  comctl32
)

target_compile_definitions(AtlantisMajordomo PRIVATE
  UNICODE
  _UNICODE
  WIN32_LEAN_AND_MEAN
  NOMINMAX
)

target_compile_options(AtlantisMajordomo PRIVATE
  /W4
  /permissive-
)
