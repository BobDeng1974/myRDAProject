#ifndef _UNICODEGBTRANS_H_
#define _UNICODEGBTRANS_H_

unsigned long __UCS2ToGB2312(const unsigned char * src, unsigned char * dst,unsigned long srclen);
unsigned long __GB2312ToUCS2(const unsigned char* src, unsigned char* dst, unsigned long srclen);

/* end */
 
#endif
