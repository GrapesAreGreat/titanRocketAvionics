#ifndef PYRO_LOGIC_HPP_
#define PYRO_LOGIC_HPP_

#include "data_logging.hpp"

void pyro_logic_init();

void pyro_logic_tick();

void fire_drogue_signal_on(FlashLogger *logger);

void fire_chute_signal_on(FlashLogger *logger);

#endif
