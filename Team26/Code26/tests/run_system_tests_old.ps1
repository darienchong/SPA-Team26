# autotester.exe
$autotester = "..\build_win\x86-Release\src\autotester\autotester.exe"

$OUT_DIR = ".\out"

# Remove out dir contents
Remove-Item $OUT_DIR -Force -Recurse -ErrorAction SilentlyContinue

# Make out dir for results
if (!(test-path $OUT_DIR)) {
  New-Item $OUT_DIR -ItemType Directory | Out-Null
}

# Get all test case folders
$test_cases = Get-ChildItem -Directory -Filter test*  -Name

# Run all test cases
"Running all test cases..."

foreach ($test_folder in $test_cases) {
  # Make folders for results of each test case
  if (!(test-path $OUT_DIR\$test_folder)) {
    New-Item $OUT_DIR\$test_folder -ItemType Directory | Out-Null
  }

  if (!(test-path $OUT_DIR\$test_folder\analysis.xsl)) {
    Copy-Item .\analysis.xsl -Destination $OUT_DIR\$test_folder
  }

  # Get all quries files to test
  $test_queries = Get-ChildItem -Path $test_folder -File -Filter queries_*.txt -Name

  # For each quries file, run autotester and save result in out folder
  $i = 1
  foreach($test_query in $test_queries) {
    Measure-Command {
      Invoke-Expression -Command "$autotester .\$test_folder\source.txt .\$test_folder\$test_query $OUT_DIR\$test_folder\result_$i.xml" > $OUT_DIR\$test_folder\result_$i.log
    }
    "[$test_folder] [Queries $i] Done"
    $i += 1
  }
}

pause

# # For debugging
# Invoke-Expression -Command "$autotester .\test1\source.txt .\test1\queries_1.txt out.xml"
# "Done"
