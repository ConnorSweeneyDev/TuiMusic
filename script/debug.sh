#!/bin/bash

if [ "$OS" == "Windows_NT" ]; then
  gdb -tui ./binary/windows/TuiMusic.exe
elif [ "$(uname)" == "Linux" ]; then
  gdb -tui ./binary/linux/TuiMusic.out
else
  echo "Unsupported OS"
fi
