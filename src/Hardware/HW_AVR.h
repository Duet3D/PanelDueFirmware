#ifndef HW_AVR_h
#define HW_AVR_h

// *** Hardware specific functions ***
inline void UTFT::LCD_Write_Bus(uint16_t VHL)
{
#if 1
	// inline code for speed
# if SAM4S
	PIOA->PIO_ODSR = (uint32_t)VHL << 16;
# else
	PIOA->PIO_ODSR = VHL;
# endif
#else
	// original slow code
# if SAM4S
	pio_sync_output_write(PIOA, (uint32_t)VHL << 16);
# else
	pio_sync_output_write(PIOA, VHL);
# endif
#endif
	portWR.pulseLow();
}

// Write the previous 16-bit data again the specified number of times.
// Only supported in 9 and 16 bit modes. Used to speed up setting large blocks of pixels to the same colour. 
void UTFT::LCD_Write_Again(uint16_t num)
{   
	while (num != 0)
	{
		portWR.pulseLow();
		--num;
	}
}

void UTFT::_set_direction_registers()
{
#if SAM4S
	pio_configure(PIOA, PIO_OUTPUT_0, 0xFFFF0000, 0);
	pio_enable_output_write(PIOA, 0xFFFF0000);
#else
	pio_configure(PIOA, PIO_OUTPUT_0, 0x0000FFFF, 0);
	pio_enable_output_write(PIOA, 0x0000FFFF);
#endif
}

#endif
