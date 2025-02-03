CC = gcc
CFLAGS = -march=native -O3 -ffast-math -flto -fno-exceptions -funroll-loops -Wall -Wextra
LIBS = -lxcb -lxcb-util -lxcb-icccm -lxcb-keysyms -lxkbcommon -lxcb-cursor 
C_FILES = absent.c clients.c events.c keycallbacks.c keys.c

BIN_DIR = /usr/bin
EXEC_FILE = absent
AUTOSTART_FILE = autostartabsent
SESSION_FILE = $(EXEC_FILE).desktop
SESSION_DIR = /usr/share/xsessions

all: copy install

copy:
	cp $(SESSION_FILE) $(SESSION_DIR)
	cp $(AUTOSTART_FILE) $(BIN_DIR)

install: absent.c
	$(CC) -o $(BIN_DIR)/$(EXEC_FILE) $(C_FILES) $(CFLAGS) $(LIBS)

clean:
	rm -f $(BIN_DIR)/$(EXEC_FILE)
	rm -f $(SESSION_DIR)/$(SESSION_FILE)
	rm -f $(BIN_DIR)/$(AUTOSTART_FILE)

.PHONY: all copy install clean
