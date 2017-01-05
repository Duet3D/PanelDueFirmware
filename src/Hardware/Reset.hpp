/*
 * Reset.h
 *
 * Created: 07/11/2015 11:46:58
 *  Author: David
 */ 


#ifndef RESET_H_
#define RESET_H_

// Restart the hardware
void Restart()
{
	rstc_start_software_reset(RSTC);
}

#endif /* RESET_H_ */