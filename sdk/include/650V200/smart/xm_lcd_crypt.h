#ifndef  __XM_LCD_CRYPT__
#define  __XM_LCD_CRYPT__
#ifdef __cplusplus
extern "C"
{
#endif

/*
param used by user alloc         32 bytes
pucSrcId   device send to lcd    32 bytes
pucFlashId read from flash ID1   32 bytes
pucDstId   lcd send to device    32 bytes
len                              32 bytes 
*/
int XM_LCD_DECRYPT(unsigned char* pucSrcId, unsigned char *pucFlashId, unsigned char *pucDstId, int len);
	
#ifdef  __cplusplus
}
#endif  /* end of __cplusplus */
#endif

