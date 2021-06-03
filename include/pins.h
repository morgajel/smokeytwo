
// A total of six wires are needed for  type-k thermocouples
#define thermo1S0 5   // s0 is yellow wire
#define thermo1CS 4   // cs is blue wire
#define thermo1CLK 2  // clk is green wire
#define thermo2S0 12  // s0 is yellow wire
#define thermo2CS 13  // cs is blue wire
#define thermo2CLK 15 // clk is green wire

// LCD needs 2 pins; the first pin is not actually used.
#define lcdPin 14
#define deadpin 10 //not actually used by the LCD

#define power_control_pin 16   // used to control relay