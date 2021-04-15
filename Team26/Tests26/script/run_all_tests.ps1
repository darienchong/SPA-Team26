# autotester.exe
$autotester = ".\autotester.exe"

# analysis.xsl
$analysis = ".\analysis.xsl"

# Input directory name
$IN_DIR = ".."

# Output directory name
$OUT_DIR = ".\out"
$OUT_DIR_XML = "$OUT_DIR\xml"
$OUT_DIR_LOG = "$OUT_DIR\log"

#========#
# Script #
#========#

# Check for analysis.xsl
if(!(test-path $analysis)) {
  "[WARNING] analysis.xsl not found."
  "[WARNING] Generated results directory will not contain analysis.xsl."
}

# Check for autotester.exe
if(!(test-path $autotester)) {
  # Backup path in build folder
  $autotester = "..\..\build_win\x86-Release\src\autotester\autotester.exe"
  if (!(test-path $autotester)) {
    "[ERROR] autotester.exe not found."
    "Exiting..."
    pause
    exit
  }
  "[WARNING] autotester.exe not found in current directory."
  "[WARNING] Running autotester.exe in Release mode build folder..."
}

# Remove out dir contents
Remove-Item $OUT_DIR -Force -Recurse -ErrorAction SilentlyContinue

# Initialise new output dir for results
if (!(test-path $OUT_DIR)) {
  New-Item $OUT_DIR -ItemType Directory | Out-Null
  New-Item $OUT_DIR_XML -ItemType Directory | Out-Null
  New-Item $OUT_DIR_LOG -ItemType Directory | Out-Null
  Copy-Item $analysis -Destination $OUT_DIR_XML | Out-Null
}

# Get all source files
$source_files = Get-ChildItem $IN_DIR -File -Filter "*_source.txt"  -Name

# Run all test cases
"*************************"
"Running all test cases..."
"*************************"
""
""

foreach ($source_file in $source_files) {
  $test_name = ($source_file -split "_source.txt")[0]
  $queries_file = "${test_name}_queries.txt"

  $source_path = "$IN_DIR\$source_file"
  $queries_path = "$IN_DIR\$queries_file"
  $xml_path = "$OUT_DIR_XML\$test_name.xml"
  $log_path = "$OUT_DIR_LOG\$test_name.log"

  if(!(test-path $queries_path)) {
    "[WARNING] $source_file does not have a correspending $queries_file file."
    "[WARNING] Skipping test case..."
    ""
    continue
  }

  ""
  "Running $test_name..."
  $timing = Measure-Command {
      Invoke-Expression -Command "$autotester $source_path $queries_path $xml_path" > $log_path
  }
  "COMPLETED"
  "Time taken: ${timing}."
  ""
  ""
}

"***************************"
"Completed all test cases..."
"***************************"
pause
