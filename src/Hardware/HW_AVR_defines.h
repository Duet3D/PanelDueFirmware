// *** Hardwarespecific defines ***

#ifndef HW_AVR_DEFINES_h
#define HW_AVR_DEFINES_h

//#define swap(type, i, j) {type t = i; i = j; j = t;}

#define fontbyte(x) *(uint8_t*)(&cfont.font[x])  

#define regtype volatile uint8_t
#define regsize uint8_t

#endif
