cmake_minimum_required(VERSION 3.20)
project(AtlantisMajordomo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ATLANTIS_BUILD_TESTS "Build Catch2 tests" OFF)

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
  src/Function/MapRegionDetailsUtils.cpp
  src/Function/FactionAttitudeUtils.cpp
  src/Function/HexDirectionUtils.cpp
  src/Function/JsonUtils.cpp
  src/Function/MonthUtils.cpp
  src/Function/MapNavigationUtils.cpp
  src/Function/ItemOrderingUtils.cpp
    src/Function/LocalEngineFactionUtils.cpp
    src/Function/LocalEngineRunUtils.cpp
    src/Function/LocalEngineSettingsUtils.cpp
  src/Function/OrderBusinessLogic.cpp
  src/Function/OrderChecksUtils.cpp
  src/Function/OrderItemTokenUtils.cpp
  src/Function/OrderParsingUtils.cpp
  src/Function/OrderWarningService.cpp
  src/Function/ReadOnlyPopupService.cpp
  src/Function/SkillFormattingUtils.cpp
  src/Function/StartupAutoLoadUtils.cpp
  src/Function/StringUtils.cpp
  src/Function/TabRefreshUtils.cpp
  src/Function/TradeTaxUtils.cpp
  src/Function/MapUtils.cpp
  src/Function/MainFactionExportUtils.cpp
  src/Function/UnitCapacityUtils.cpp
  src/Function/UnitsListSortUtils.cpp
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
    src/GUI/LocalEngineTabContent.cpp
    src/GUI/TradeTaxTabContent.cpp
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
    src/GUI/Linux/LocalEngineTabContentQt.cpp
    src/GUI/Linux/ReportsTabContentQt.cpp
    src/GUI/Linux/EventsTabContentQt.cpp
    src/GUI/Linux/BattlesTabContentQt.cpp
    src/GUI/Linux/TradeTaxTabContentQt.cpp
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
    include/GUI/LocalEngineTabContentQt.hpp
    include/GUI/ReportsTabContentQt.hpp
    include/GUI/EventsTabContentQt.hpp
    include/GUI/BattlesTabContentQt.hpp
    include/GUI/TradeTaxTabContentQt.hpp
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

if(ATLANTIS_BUILD_TESTS)
  enable_testing()

  find_package(Catch2 3 QUIET)
  if(NOT Catch2_FOUND)
    include(FetchContent)
    if(POLICY CMP0135)
      cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(
      Catch2
      URL https://codeload.github.com/catchorg/Catch2/tar.gz/refs/tags/v3.7.1
      URL_HASH SHA256=c991b247a1a0d7bb9c39aa35faf0fe9e19764213f28ffba3109388e62ee0269c
    )
    FetchContent_MakeAvailable(Catch2)
  endif()

  set(TEST_COMMON_SOURCES ${COMMON_SOURCES})
  list(REMOVE_ITEM TEST_COMMON_SOURCES src/Main.cpp)
  add_executable(AtlantisMajordomoTests
    tests/Data/AppDataTests.cpp
    tests/Data/BattleRepositoryTests.cpp
    tests/Data/BattleTests.cpp
    tests/Data/CommandsTests.cpp
    tests/Data/DataSerializerTests.cpp
    tests/Data/EventTests.cpp
    tests/Data/EventRepositoryTests.cpp
    tests/Data/FactionTests.cpp
    tests/Data/FactionRepositoryTests.cpp
    tests/Data/ItemTests.cpp
    tests/Data/ItemRepositoryTests.cpp
    tests/Data/OrderTests.cpp
    tests/Data/OrderRepositoryTests.cpp
    tests/Data/RegionTests.cpp
    tests/Data/RegionRepositoryTests.cpp
    tests/Data/ReportTests.cpp
    tests/Data/ReportRepositoryTests.cpp
    tests/Data/SkillTests.cpp
    tests/Data/SkillRepositoryTests.cpp
    tests/Data/StructInfoTests.cpp
    tests/Data/StructInfoRepositoryTests.cpp
    tests/Data/StructureTests.cpp
    tests/Data/StructureRepositoryTests.cpp
    tests/Data/UnitTests.cpp
    tests/Data/UnitRepositoryTests.cpp
    tests/Data/UnitNewTests.cpp
    tests/Data/UnitNewRepositoryTests.cpp
    tests/Function/AppDataUtilsTests.cpp
    tests/Function/BattleFormattingUtilsTests.cpp
    tests/Function/CommandSimulationServiceTests.cpp
    tests/Function/CoordinateFormattingUtilsTests.cpp
    tests/Function/MonthUtilsTests.cpp
    tests/Function/OrderParsingUtilsTests.cpp
    ${TEST_COMMON_SOURCES}
  )
  set_target_properties(AtlantisMajordomoTests PROPERTIES AUTOMOC OFF AUTORCC OFF)
  target_include_directories(AtlantisMajordomoTests PRIVATE include tests)
  target_link_libraries(AtlantisMajordomoTests PRIVATE Catch2::Catch2WithMain)
  if(WIN32)
    target_compile_definitions(AtlantisMajordomoTests PRIVATE
      UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
    )
  endif()

  include(Catch)
  catch_discover_tests(AtlantisMajordomoTests)
endif()

