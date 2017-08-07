#ifndef __LED_H__
#define __LED_H__
enum
{
	LED_OFF,
	LED_ON,
	LED_FLUSH_SLOW,
	LED_FLUSH_FAST,

	LED_TYPE_GSM = 0,
	LED_TYPE_GPS,
	LED_TYPE_MAX,
};
void Led_Flush(uint8_t Led, uint8_t NewType);
#endif
