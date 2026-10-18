#ifndef BUTTON_H
#define BUTTON_H

// the KEY button on the Black Pill (PA0, connects to GND when pressed)
void button_init(void);

// 1 if the button was pressed since the last call, else 0
int button_was_pressed(void);

#endif
