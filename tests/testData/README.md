# Test Data

This folder contains test data files required for unit tests. These files are copied from the project root's `testData` folder and are organized here to keep test data self-contained and portable.

## Contents

- `report_17_003.rep` - Sample Atlantis report file used by `ReportTests.cpp` and `ReportRepositoryTests.cpp`
- `report_17_004.rep` - Sample Atlantis report file used by `ReportTests.cpp`

## How Tests Access Test Data

Test files should use the `TestUtils::getTestDataPath()` function from `TestUtils.hpp` to locate test data files relative to the executable location. This approach:

1. **Portability**: Tests work regardless of where the project is checked out
2. **Independence**: Tests don't depend on a specific directory structure or absolute paths
3. **Automation**: CTest and CI/CD systems can run tests from any working directory
4. **Maintainability**: Adding test data requires only copying files to this folder

### Example Usage

```cpp
#include "TestUtils.hpp"

TEST_CASE("Example test with file loading", "[Example]")
{
  const std::string filePath = TestUtils::getTestDataPath("report_17_003.rep");
  REQUIRE_FALSE(filePath.empty());
  
  // Use filePath for your test...
}
```

## Adding New Test Data

When adding new test files:

1. Copy the file to this folder: `tests/testData/`
2. Update tests to use `TestUtils::getTestDataPath("filename.ext")`
3. Update this README if the file has a specific purpose

## Integration with Build System

The CMakeLists.txt includes the `tests` directory in the include path, making `TestUtils.hpp` accessible to all test files.
