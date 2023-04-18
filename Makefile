# flags = -O2 -H -DBlackWhite
flags = -DBlackWhite -O2
sdl_linkers_flags = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
lib_path = src/include
linkers_path = src/lib

file_path = src
file_name = test
file_ext = .cpp
proc_ext = .exe

all:
	g++ $(flags) -I $(lib_path) -L $(linkers_path) $(file_path)/*$(file_ext) -o $(file_name)$(proc_ext) $(sdl_linkers_flags)
	./$(file_name)$(proc_ext)

compile:
	g++ $(flags) -I $(lib_path) -L $(linkers_path) $(file_path)/*$(file_ext) $(sdl_linkers_flags)
	# ./$(file_name)$(proc_ext)

run:
	./$(file_name)$(proc_ext)