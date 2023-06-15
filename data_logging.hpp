#ifndef DATA_LOGGING_HPP_
#define DATA_LOGGING_HPP_

#include "W25N512GVEIG.h"
#include <stdint.h>

class FlashLogger {
public:
  FlashLogger();

  void init();

  void erase_all_and_reset();

  void log_string(char *s);
  void log_string(char *s, const size_t s_len);
  
  void flush();

  void write_all_to_serial();
private:
  W25N512 flash;
  uint16_t currentPageAddress;
  uint16_t currentInternalBufferLength;
};

#endif
