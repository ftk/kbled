
kbled.exe: main.c keyboard.c
	gcc -Wall -O3 -flto -mwindows -s -std=c11 -fvisibility=hidden -fno-ident $^ -o $@
