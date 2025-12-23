#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// TTP229 PIN MAPPING
#define TTP229_SCL 18   // D8  (GPIO17)
#define TTP229_SDO 17   // D9  (GPIO18)

// PASSWORD
extern String currentPassword;
extern String inputPassword;

// FUNCTION PROTOTYPES
byte ttp229_read_key();
void password_task(void *param);

#endif
