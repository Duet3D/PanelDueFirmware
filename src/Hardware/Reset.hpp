/*
 * Reset.h
 *
 * Created: 07/11/2015 11:46:58
 *  Author: David
 */ 


#ifndef RESET_H_
#define RESET_H_

#include "asf.h"

#ifdef __cplusplus
extern "C" {
#endif

// Restart the hardware
inline void Reset()
{
	rstc_start_software_reset(RSTC);
}
void EraseAndReset() noexcept;

#ifdef __cplusplus
}
#endif

#endif /* RESET_H_ */
