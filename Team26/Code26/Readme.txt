==========================================================
 NUS CS3203 Static Program Analyzer (SPA) project - CMake
==========================================================
The directory contains all the source code and relavent files to the SPA project for the NUS module CS3203.

#######################################
#               Info                  #
#######################################
Visual Studio 2019 version: 16.8.4
We use the Cross platform Startup SPA solution which uses Cmake.
Testing is done on a Windows machine

How to get started:

  https://github.com/nus-cs3203/project-wiki/wiki/Cross-platform-Startup-SPA-Solution

Folders info:
  - `src` Contains all the source code of the project.
  - `tests` Contains the system test cases.

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
A set of System Tests is provided in the `tests` directory with a PowerShell test script named `run_system_tests.ps1`.
Follow the instructions below to use the test script. 

  1. Build the autotester.exe in Release mode. (Refer to How to build AutoTester)
  2. Go to the `test` directory.
  3. Ensure that the test cases are written in the write format. (Skip this step if you are using the provided test cases)
    - The `analysis.xsl` XML sytlesheet should be present
    - Each test case should be stored in a directory named `test<ID>`
    - Each test case should only contain ONE source program named `source.txt`
    - Each test case can have multiple queries files named `queries_<ID>.txt`
  4. Right-click on `run_system_tests.ps1` > `Run with PowerShell`. 
    - Make sure that you enable the execution of PowerShell scripts on your machine. See NOTE below for more info.
  5. The results can be found in the `out` directory.
    - The results of the test cases are grouped in a directory with the same test case directory name. 
      - e.g. `out/test1`
    - The `.xml` files will be generated in the directory with the name `result_<ID>.xml`. 
      - e.g. `out/test1/result_1.txt`
    - The log files will also be regenerated in the same directory with the name `result_<ID>.log`. 
      -e.g. `out/test1/result_1.log`

NOTE:
If you encountered an error message that mentions "execution of scripts is disabled on this system",
you can open PowerShell as administrator and run the following command

  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope LocalMachine

More info about ExecutionPolicy:
- https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_execution_policies?view=powershell-7.1&viewFallbackFrom=powershell-6
- https://stackoverflow.com/questions/54776324/powershell-bug-execution-of-scripts-is-disabled-on-this-system
