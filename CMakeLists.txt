cmake_minimum_required(VERSION 3.20)
project(AtlantisMajordomo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Common sources (shared by all platforms)
# ---------------------------------------------------------------------------
set(COMMON_SOURCES
  src/DebugLog.cpp
  src/AppConfig.cpp
  src/Main.cpp
  src/Data/AppData.cpp
  src/Data/Commands.cpp
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
  src/Data/Report.cpp
  src/Data/ReportRepository.cpp
  src/Function/AppDataUtils.cpp
  src/Function/BattleFormattingUtils.cpp
  src/Function/CommandSimulationService.cpp
  src/Function/CoordinateFormattingUtils.cpp
  src/Function/CoordinateUtils.cpp
  src/Function/FactionAttitudeUtils.cpp
  src/Function/HexDirectionUtils.cpp
  src/Function/JsonUtils.cpp
  src/Function/MonthUtils.cpp
  src/Function/OrderBusinessLogic.cpp
  src/Function/OrderParsingUtils.cpp
  src/Function/OrderWarningService.cpp
  src/Function/SkillFormattingUtils.cpp
  src/Function/StringUtils.cpp
  src/Function/MapUtils.cpp
  src/Function/UnitCapacityUtils.cpp
)

# ---------------------------------------------------------------------------
# Windows build  (Win32 API + native GUI)
# ---------------------------------------------------------------------------
if(WIN32)

  set(GUI_SOURCES
    src/GUI/AtlantisMajordomo.rc
    src/GUI/UiSizeProfile.cpp
    src/GUI/UiScaleManagerWin.cpp
    src/GUI/MainWindowWin.cpp
    src/GUI/EventsTabContent.cpp
    src/GUI/WinGuiUtils.cpp
    src/GUI/WinSizingUtils.cpp
    src/GUI/ReportsTabContent.cpp
    src/GUI/MapTabContent.cpp
    src/GUI/MapTabContent_Layout.cpp
    src/GUI/MapTabContent_MapCanvas.cpp
    src/GUI/MapTabContent_UnitDetails.cpp
    src/GUI/MapTabContent_Orders.cpp
    src/GUI/MapTabContent_OrderChecks.cpp
    src/GUI/MapTabContent_RegionDetails.cpp
    src/GUI/MapTabContent_Navigation.cpp
    src/GUI/MapTabContent_Events.cpp
    src/GUI/OrdersEditorUtils.cpp
    src/GUI/ItemsTabContent.cpp
    src/GUI/SkillsTabContent.cpp
    src/GUI/FactionsTabContent.cpp
    src/GUI/BattlesTabContent.cpp
    src/GUI/WinTabView.cpp
    src/GUI/SettingsDialog.cpp
  )

  add_executable(AtlantisMajordomo WIN32
    ${COMMON_SOURCES}
    ${GUI_SOURCES}
  )

  target_link_libraries(AtlantisMajordomo PRIVATE comctl32)

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

# ---------------------------------------------------------------------------
# Linux build  (Qt6)
# ---------------------------------------------------------------------------
elseif(UNIX)

  find_package(Qt6 REQUIRED COMPONENTS Widgets)

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)

  set(GUI_SOURCES
    src/GUI/Linux/AtlantisMajordomo.qrc
    src/GUI/Linux/MainWindowQt.cpp
    src/GUI/Linux/ReportsTabContentQt.cpp
    src/GUI/Linux/EventsTabContentQt.cpp
    src/GUI/Linux/BattlesTabContentQt.cpp
    src/GUI/Linux/FactionsTabContentQt.cpp
    src/GUI/Linux/ItemsTabContentQt.cpp
    src/GUI/Linux/SkillsTabContentQt.cpp
    src/GUI/Linux/SettingsDialogQt.cpp
    src/GUI/Linux/MapTabContentQt_Layout.cpp
    src/GUI/Linux/MapTabContentQt_UnitDetails.cpp
    src/GUI/Linux/MapTabContentQt_Orders.cpp
    src/GUI/Linux/MapTabContentQt_OrderChecks.cpp
    src/GUI/Linux/MapTabContentQt_RegionDetails.cpp
    src/GUI/Linux/MapTabContentQt_Navigation.cpp
    src/GUI/Linux/MapTabContentQt_Events.cpp
    src/GUI/Linux/MapTabContentQt_MapCanvas.cpp
    include/GUI/MainWindowQt.hpp
    include/GUI/ReportsTabContentQt.hpp
    include/GUI/EventsTabContentQt.hpp
    include/GUI/BattlesTabContentQt.hpp
    include/GUI/FactionsTabContentQt.hpp
    include/GUI/ItemsTabContentQt.hpp
    include/GUI/SkillsTabContentQt.hpp
    include/GUI/SettingsDialogQt.hpp
    include/GUI/MapTabContentQt.hpp
    include/GUI/MapCanvasWidget.hpp
  )

  add_executable(AtlantisMajordomo
    ${COMMON_SOURCES}
    ${GUI_SOURCES}
  )

  target_link_libraries(AtlantisMajordomo PRIVATE Qt6::Widgets)

  target_compile_options(AtlantisMajordomo PRIVATE
    -Wall -Wextra -Wpedantic
  )

  install(TARGETS AtlantisMajordomo
    RUNTIME DESTINATION bin
  )

  install(FILES packaging/linux/AtlantisMajordomo.desktop
    DESTINATION share/applications
  )

  set(ICON_SIZES 16x16 24x24 32x32 48x48 64x64 128x128 256x256 512x512)
  foreach(ICON_SIZE IN LISTS ICON_SIZES)
    install(FILES packaging/linux/icons/hicolor/${ICON_SIZE}/apps/AtlantisMajordomo.png
      DESTINATION share/icons/hicolor/${ICON_SIZE}/apps
    )
  endforeach()

endif()

# ---------------------------------------------------------------------------
# Shared settings
# ---------------------------------------------------------------------------
target_include_directories(AtlantisMajordomo PRIVATE include)

