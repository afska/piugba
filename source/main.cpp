#include <stdio.h>
#include <tonc.h>

int main()
{
	// Init interrupts and VBlank irq.
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	// Video mode 0, enable bg 0.
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;

	// Init 4bpp vwf text on bg 0.
	tte_init_chr4c(0,					   // BG 0
				   BG_CBB(0) | BG_SBB(31), // Charblock 0; screenblock 31
				   0xF000,				   // Screen-entry offset
				   bytes2word(1, 2, 0, 0), // Color attributes.
				   CLR_YELLOW,			   // Yellow text
				   &verdana9Font,		   // Verdana 9 font
				   NULL					   // Use default chr4 renderer
	);

	// Initialize use of stdio.
	tte_init_con();

	// Printf something at 96,72
	tte_printf("#{P:96,72}Hello World!");

	while (1)
	{
		VBlankIntrWait();
	}

	return 0;
}
