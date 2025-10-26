#ifndef CUSTOM_SYSCALLS_H
#define CUSTOM_SYSCALLS_H

// Incluye la definición de syscall()
#include <sys/syscall.h>
#include <unistd.h>

// Definiciones personalizadas de las syscalls

#define SYS_SCREEN_LIVE         556
#define SYS_MOUSE_TRACKING      557
#define SYS_MOUSE_ACTION        558
#define SYS_KEYBOARD_CAPTION    559
#define SYS_RESOURCES_PC        560

#endif
