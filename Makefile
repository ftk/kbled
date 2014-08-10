
# compile without c runtime library to make binary super small (~3k)
kbled.exe: kbled2.c
	gcc -Dmain=mainCRTStartup -std=c11 -nostdlib -mwindows $< -o $@ -luser32 -lkernel32 -Ofast -march=native -s -fvisibility=hidden -fno-ident
