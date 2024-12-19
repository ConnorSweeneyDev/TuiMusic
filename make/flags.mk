CXX := g++
ifeq ($(DEBUG), 1)
  CXX_FLAGS := -ggdb3 -Og -MD -MP -std=c++17 -DDEBUG -D_FORTIFY_SOURCE=2 -fno-common -fstack-protector-strong
else
  CXX_FLAGS := -s -O3 -std=c++17 -DNDEBUG -fno-common -fstack-protector-strong -ffunction-sections -fdata-sections -flto=auto -Wl,--gc-sections
endif

WARNINGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wundef -Wclobbered -Wdeprecated -Wmultichar -Wuninitialized -Wunreachable-code -Wstrict-aliasing -Wreturn-type -Wtype-limits -Wformat-security -Wpointer-arith -Wmaybe-uninitialized -Wempty-body -Wdouble-promotion -Wcast-qual -Wcast-align -Wfloat-equal -Wlogical-op -Wduplicated-cond -Wshift-overflow=2 -Wformat=2
INCLUDES := -Iprogram/include
ifeq ($(UNAME), Windows)
  SYSTEM_INCLUDES := -isystemexternal/include -isystemexternal/include/ftxui -isystemexternal/include/sdl/windows
  LIBRARIES := -Lexternal/library/ftxui/windows -Lexternal/library/sdl/windows -static -Wl,-Bstatic -lgcc -lstdc++ -lssp -lwinpthread -Wl,-Bdynamic -lftxui-component -lftxui-dom -lftxui-screen -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer
else ifeq ($(UNAME), Linux)
  LIBRARIES := -Lexternal/library/ftxui/linux -static -ldl -lm -lc -lgcc -lstdc++ -lftxui-component -lftxui-dom -lftxui-screen -Wl,-rpath,'$$ORIGIN'
endif
