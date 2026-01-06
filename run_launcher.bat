@echo off
if not exist launcher.exe (
  echo launcher.exe not found. Compile launcher.cpp first.
  pause
  exit /b 1
)
launcher.exe %*
