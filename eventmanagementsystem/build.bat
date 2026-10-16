@echo off
echo Building Secure Marksheet Verifier using g++...
g++ -std=c++17 -Wall -Wextra main.cpp MerkleTree.cpp MarksheetVerifier.cpp -o verifier.exe
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilation successful! Run 'verifier.exe' to start the application.
) else (
    echo.
    echo Compilation failed. Please check errors above.
)
