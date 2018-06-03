#pragma once

#define VGABIOS_ADDR	0xC0000

void load_bios(int vmfd, char *bios_path);
