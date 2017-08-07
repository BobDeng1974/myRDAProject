#ifndef _UNICODEGBTRANS_H_
#define _UNICODEGBTRANS_H_

uint32_t __UCS2ToGB2312(const uint8_t * src, uint8_t * dst,uint32_t srclen);
uint32_t __GB2312ToUCS2(const uint8_t* src, uint8_t* dst, uint32_t srclen);

/* end */
 
#endif
