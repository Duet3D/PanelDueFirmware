#ifndef HW_AVR_h
#define HW_AVR_h

// *** Hardware specific functions ***
void UTFT::LCD_Write_Bus(uint16_t VHL)
{
#if 1
	// inline code for speed
	PIOA->PIO_ODSR = VHL;
#else
	pio_sync_output_write(PIOA, VHL);
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
	pio_configure(PIOA, PIO_OUTPUT_0, (displayTransferMode == TMode16bit) ? 0x0000FFFF : 0x000000FF, 0);
	pio_enable_output_write(PIOA, (displayTransferMode == TMode16bit) ? 0x0000FFFF : 0x000000FF);
}

#endif
