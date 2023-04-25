#pragma once

#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

char *exe_dir(void);
