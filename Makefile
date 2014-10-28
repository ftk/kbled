
all: kbled.exe kbled3.exe

# compile without c runtime library to make binary super small (~3k)
kbled.exe: kbled2.c
	gcc -Dmain=mainCRTStartup -std=c11 -nostdlib -mwindows $< -o $@ -luser32 -lkernel32 -Ofast -march=native -s -fvisibility=hidden -fno-ident

kbled3.exe: kbled3.c
	gcc -O3 -march=native -flto -mwindows -s -std=c11 -fvisibility=hidden -fno-ident $< -o $@
