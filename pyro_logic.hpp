#ifndef PYRO_LOGIC_HPP_
#define PYRO_LOGIC_HPP_

#include <SD.h>

void pyro_logic_init();

void pyro_logic_tick();

void fire_drogue_signal_on(File *file);

void fire_chute_signal_on(File *file);

#endif
