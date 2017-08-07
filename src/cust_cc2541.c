#include "user.h"
uint8_t CC2541_UpgradeTx(uint8_t Cmd, uint8_t *Buf, uint8_t *Data, u16 Addr)
{
	Buf[0] = CC2541_UPGRADE_SOF;
	Buf[2] = CC2541_UPGRADE_SUBSYS;
	Buf[3] = Cmd;
	switch (Cmd)
	{
	case CC2541_UPGRADE_CMD_SHAKEHAND:
		Buf[1] = 0;
		Buf[4] = XorCheck(Buf + 1, 3, 0);
		return 5;
		break;
	case CC2541_UPGRADE_CMD_WRITE:
		Buf[1] = 0x42;
		Buf[4] = (uint8_t)(Addr& 0x00ff);
		Buf[5] = (uint8_t)(Addr >> 8);
		memcpy(Buf + 6, Data, 64);
		Buf[70] = XorCheck(Buf + 1, 69, 0);
		return 71;
		break;
	case CC2541_UPGRADE_CMD_READ:
		Buf[1] = 2;
		Buf[4] = (uint8_t)(Addr& 0x00ff);
		Buf[5] = (uint8_t)(Addr >> 8);
		Buf[6] = XorCheck(Buf + 1, 5, 0);
		return 7;
		break;
	case CC2541_UPGRADE_CMD_CHECK:
		Buf[1] = 0;
		Buf[4] = XorCheck(Buf + 1, 3, 0);
		return 5;
		break;
	}
	return 0;
}

uint8_t CC2541_UpgradeRx(uint8_t *Buf, uint8_t Len, uint8_t *Data)
{
	uint8_t Cmd = Buf[3] & 0x0f;
	uint8_t Xor;
	if ( (Buf[0] != CC2541_UPGRADE_SOF) || (Buf[2] != CC2541_UPGRADE_SUBSYS))
	{
		DBG("%02x %02x", Buf[0], Buf[2]);
		return CC2541_UPGRADE_RX_ERROR;
	}


	switch (Cmd)
	{
	case CC2541_UPGRADE_CMD_SHAKEHAND:
	case CC2541_UPGRADE_CMD_WRITE:
		Xor = XorCheck(Buf + 1, Len - 2, 0);
		if (Xor != Buf[Len - 1])
		{
			return CC2541_UPGRADE_XOR_FAIL;
		}
		else
		{
			return Buf[4];
		}
		break;
	case CC2541_UPGRADE_CMD_CHECK:
		Xor = XorCheck(Buf + 1, 4, 0);
		if (Xor != Buf[5])
		{
			return CC2541_UPGRADE_XOR_FAIL;
		}
		else
		{
			return Buf[4];
		}
		break;
	case CC2541_UPGRADE_CMD_READ:
		if (memcmp(Buf + 7, Data, 64))
		{
			HexTrace(Buf + 7, 64);
			HexTrace(Data, 64);
			return CC2541_UPGRADE_DATA_ERROR;
		}
		else
		{
			return Buf[4];
		}
		break;
	}
	return CC2541_UPGRADE_RX_ERROR;
}
