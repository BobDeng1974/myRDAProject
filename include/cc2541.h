#ifndef __CC2541_H__
#define __CC2541_H__
#include "cs_types.h"
#define CC2541_UPGRADE_SOF (0xFE)
#define CC2541_UPGRADE_SUBSYS (0x0D)

#define CC2541_UPGRADE_STATUS_OK (0x00)
#define CC2541_UPGRADE_DATA_ERROR (0xFF)
#define CC2541_UPGRADE_XOR_FAIL (0xEE)
#define CC2541_UPGRADE_RX_ERROR (0xDD)
#define CC2541_UPGRADE_CMD_SHAKEHAND (0x04)
#define CC2541_UPGRADE_CMD_WRITE (0x01)
#define CC2541_UPGRADE_CMD_READ (0x02)
#define CC2541_UPGRADE_CMD_CHECK (0x03)
#define CC2541_UPGRADE_BIN_LEN	(248 * 1024)
#define CC2541_UPGRADE_DATA_PACK_LEN (64)
uint8_t CC2541_UpgradeTx(uint8_t Cmd, uint8_t *Buf, uint8_t *Data, uint16_t Addr);
uint8_t CC2541_UpgradeRx(uint8_t *Buf, uint8_t Len, uint8_t *Data);
#endif
