/*
 * Icons.h
 *
 * Created: 29/11/2015 12:11:34
 *  Author: David
 */ 


#ifndef ICONS_H_
#define ICONS_H_

extern const uint16_t IconNozzle1[], IconNozzle2[], IconNozzle3[], IconNozzle4[];

#if DISPLAY_X == 800
extern const uint16_t IconNozzle5[], IconNozzle6[];
#endif

extern const uint16_t IconHomeAll[], IconHomeX[], IconHomeY[], IconHomeZ[], IconHomeU[], IconBedComp[];

#if DISPLAY_X == 800
extern const uint16_t IconHomeV[], IconHomeW[];
#endif

extern const uint16_t IconBed[], IconOk[], IconCancel[], IconFiles[], IconKeyboard[], IconTrash[];
extern const uint16_t IconBackspace[], IconUp[], IconDown[], IconEnter[];

#endif /* ICONS_H_ */