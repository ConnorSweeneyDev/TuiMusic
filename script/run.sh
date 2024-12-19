#!/bin/bash

if [ "$OS" == "Windows_NT" ]; then
  ./binary/windows/TuiMusic.exe
elif [ "$(uname)" == "Linux" ]; then
  ./binary/linux/TuiMusic.out
else
  echo "Unsupported OS"
fi
