This directory contains all the system test cases.

- Test cases that starts with `System` are system tests.
- Test cases that starts with `Stress` are stress tests.
- Test cases that contains `Iter1` in its name focuses on the Iteration 1 design abstractions.
- Test cases that contains `Iter2` in its name focuses on the Full SPA design abstractions.

For SPA Extension test cases, they can be found in the `test_extensions` sub-directory.
These test cases name start with `SystemIter3Ext` and they focuses on the SPA extension design abstractions.

#######################################
#      How to build AutoTester        #
#######################################
  1. Open Visual Studio 2019 in the current directory (Code26)
  2. Select the `x86-Release` configuration.
  3. In the Solution Explorer, Right-click `CMakeLists.txt` > `Build`
  4. Once the build is successful, the executable path can be found in `build_win\x86-Release\src\autotester\autotester.exe`

#######################################
# Running System Tests (Windows only) #
#######################################
A PowerShell test script in named `run_system_tests.ps1` is provided in the `script` sub-directory.
Follow the instructions below to use the test script. 

  1. Build the autotester.exe in Release mode. (Refer to How to build AutoTester)
  2. Place the autotester.exe in the same directory ad the script (in `script`)
  3. Ensure that `analysis.xsl` is also in the `script` sub-directory.
  4. Place all the test cases in the current directory 
    - Place the test cases in `test_extension` in the current directory if you want to test them.
    - Skip this step if you do not want to test the extension test cases.
  4. Right-click on `run_system_tests.ps1` > `Run with PowerShell`. 
    - Make sure that you enable the execution of PowerShell scripts on your machine. See NOTE below for more info.
  5. The results can be found in the `out` directory.

More info about ExecutionPolicy:
- https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_execution_policies?view=powershell-7.1&viewFallbackFrom=powershell-6
- https://stackoverflow.com/questions/54776324/powershell-bug-execution-of-scripts-is-disabled-on-this-system

