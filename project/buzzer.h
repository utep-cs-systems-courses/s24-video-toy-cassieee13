#ifndef buzzer_included
#define buzzer_included

#define LITTLE_C 1915
#define D 1700
#define E 1519
#define F 1432
#define G 1275
#define A 1136
#define B 1014
#define BIG_C 956

void buzzer_init();
void blueWon();
void redWon();
void buzzer_set_period(short cycles);
#endif // included
