
#define CTRLEPROM_s_nZERO                   (0U)

#define CTRLEPROM_s_nWITH_OUT_REQS          (1U)
#define CTRLEPROM_s_nREAD                   (2U)
#define CTRLEPROM_s_nWRITE                  (3U)
#define CTRLEPROM_s_nWAIT_STATE_ANS_READ    (4U)
#define CTRLEPROM_s_nWAIT_STATE_ANS_WRITE   (5U)

#define CTRLEPROM_s_nFIRST_VALUE           (0xAA)
#define CTRLEPROM_s_nSECOND_VALUE          (0x55)

#define CTRLEPROM_s_nADDR                   (1)
#define CTRLEPROM_s_nFIRST_DATA             (2)
#define CTRLEPROM_s_nCURR_DATA              (3)
#define CTRLEPROM_s_nSECOND_DATA            (4)
#define CTRLEPROM_s_nCRC                    (5)

#define CTRLEPROM_s_nDATA1                  (0)
#define CTRLEPROM_s_nONE_DATA               (1)
#define CTRLEPROM_s_nDATA2                  (2)

typedef struct{
	uint32 u32MsgId;
	uint32 u32Flag;
	uint32 u32Addr;
	uint32 u32BufferSize;
	uint32* pu32Data1;
	uint32* pu32Data;
	uint32* pu32Data2;
	uint32 u32CRC;
}CTRLEPROM_s_tstMsg;


 static uint8 CTRLEPROM_s_u8Tries = 0U;
 static uint32 CTRLEPROM_s_u32KernelTime = 0U;
 static uint8 CTRLEPROM_s_u8FMS= 0U;
 static uint32* CTRLEPROM_s_pu32ReadMsg = 0U;
 static uint32* CTRLEPROM_s_pu32WriteMsg = (uint32*)NULL;
 static sint32 CTRLEPROM_s_s32StateReadMsg = 0U;
 static sint32 CTRLEPROM_s_s32StateWriteMsg = 0U;
 static uint32 CTRLEPROM_s_u32Addr = 0U;
 static uint16 CTRLEPROM_s_u16BufferSize = 0U;
 static void *CTRLEPROM_s_pvAddr = (void*)NULL;


static void CTRLEPROM_s_vReading(void)
{
	sint32 s32State = E_NOT_OK;
	CTRLEPROM_s_tstMsg* pstMsg = {0U};

	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_pvAddr);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u32Addr);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u16BufferSize);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);

	MSGQUEUE_vMsgAcquire((void**)&CTRLEPROM_s_pvAddr);
//	void* pvMsg = (void*)CTRLEPROM_s_pvAddr;

	pstMsg = (CTRLEPROM_s_tstMsg*)CTRLEPROM_s_pvAddr;

	pstMsg->u32Flag = (uint32)CTRLEPROM_s_nREAD;
	pstMsg->u32Addr = (uint32)CTRLEPROM_s_u32Addr;
	pstMsg->u32BufferSize = (uint32)CTRLEPROM_s_u16BufferSize;

	MSGQUEUE_vTxMsg((void**)&CTRLEPROM_s_pvAddr, (uint32)CTRLEPROM_CONTROL_PID_cfg);


	CTRLEPROM_s_u8FMS = (uint8)CTRLEPROM_s_nWAIT_STATE_ANS_READ;


}

static void CTRLEPROM_s_vWriting(void)
{
	uint8 u8CRC = (uint8)CTRLEPROM_s_nZERO;
	sint32 s32State = E_NOT_OK;

	CTRLEPROM_s_tstMsg* pstMsg = {0U};
	uint8 au8Data[CTRLEPROM_s_nMAX_LEN] = {0U};
	uint8 u8Dato1 = (uint8)CTRLEPROM_s_nFIRST_VALUE;
	uint8 u8Dato2 = (uint8)CTRLEPROM_s_nSECOND_VALUE;
//	void* pvMsg = (void*)NULL;

	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u32Addr);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u8Tries);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_pvAddr);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u16BufferSize);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_pu32WriteMsg);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);

	MSGQUEUE_vMsgAcquire((void**)&CTRLEPROM_s_pvAddr);
//	pvMsg = CTRLEPROM_s_pvAddr;

	pstMsg = (CTRLEPROM_s_tstMsg*)CTRLEPROM_s_pvAddr;

	pstMsg->u32Flag = (uint32)CTRLEPROM_s_nWRITE;
	pstMsg->u32Addr = (uint32)CTRLEPROM_s_u32Addr;
	pstMsg->u32BufferSize = (uint32)CTRLEPROM_s_u16BufferSize;
	pstMsg->pu32Data1 = (uint32*)&u8Dato1;
	pstMsg->pu32Data = CTRLEPROM_s_pu32WriteMsg;
	pstMsg->pu32Data2 = (uint32*)&u8Dato2;

	au8Data[CTRLEPROM_s_nDATA1] = u8Dato1;
	uint16 u16i;

	for(u16i = 1; u16i < CTRLEPROM_s_u16BufferSize + (uint16)CTRLEPROM_s_nONE_DATA; u16i++)
	{
		au8Data[u16i] = (uint8)pstMsg->pu32Data[u16i];

	}

    au8Data[CTRLEPROM_s_u16BufferSize + CTRLEPROM_s_nONE_DATA] = u8Dato2;

	CRC_LUT_vGetCRC8(au8Data, (uint16)sizeof(au8Data), &u8CRC);
	pstMsg->u32CRC = (uint32) u8CRC;

	MSGQUEUE_vTxMsg((void**)&CTRLEPROM_s_pvAddr, (uint32)CTRLEPROM_CONTROL_PID_cfg);

	CTRLEPROM_s_u8FMS = (uint8)CTRLEPROM_s_nWAIT_STATE_ANS_WRITE;



}

static void CTRLEPROM_s_vWaitStateRead(void)
{
	uint32 u32Time = 0U;
	sint32 s32State = E_NOT_OK;
	MSGQUEUE_tstMsgSelectBoth stReadMsg = {(uint32)CTRLEPROM_CONTROL_PID_cfg,0U,0U};
	void* pvMsg = (void*)NULL;
	CTRLEPROM_s_tstMsg* pstMsg;
	uint8 au8Data[CTRLEPROM_s_nMAX_LEN] = {0U};
	uint8 u8CRC = 0U;

	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u32KernelTime);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u16BufferSize);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);

	u32Time = RTC_u32GetKernelClock() - CTRLEPROM_s_u32KernelTime;

	pvMsg = MSGQUEUE_pvRxMsg(MSGQUEUE_nNO_TMO,(void*)&stReadMsg ,(uint32)MSGQUEUE_nMSGRX_BOTH);

	if(u32Time >= (uint32)CTRLEPROM_TIMEOUT_REQ)
	{
		CTRLEPROM_s_s32StateReadMsg = E_NOT_OK;

	}
	else if((pvMsg == (void*)NULL))
	{
		CTRLEPROM_s_s32StateReadMsg = (uint32)STD_NACK;

	}
	else
	{
		uint16 u16i;
		pstMsg = (CTRLEPROM_s_tstMsg*)pvMsg;
		au8Data[CTRLEPROM_s_nDATA1] = *pstMsg->pu32Data1;

		for(u16i = 1; u16i < CTRLEPROM_s_u16BufferSize + (uint16)CTRLEPROM_s_nONE_DATA; u16i++)
		{
			au8Data[u16i] = pstMsg->pu32Data[u16i];

		}

	    au8Data[CTRLEPROM_s_u16BufferSize + CTRLEPROM_s_nONE_DATA] = *pstMsg->pu32Data2;

		CRC_LUT_vGetCRC8(au8Data, (uint16)sizeof(au8Data), &u8CRC);
		s32State = (u8CRC == (uint8)pstMsg->u32CRC) ? E_OK: E_NOT_OK;

		if(s32State == E_OK )
		{
			CTRLEPROM_s_s32StateReadMsg = (sint32)STD_ACK;
			CTRLEPROM_s_pu32ReadMsg =pstMsg->pu32Data;
		}
		else
		{

		}

	}

	CTRLEPROM_s_u8FMS = (uint8)CTRLEPROM_s_nWITH_OUT_REQS;

}

static void CTRLEPROM_s_vWaitStateWrite(void)
{
	uint32 u32Time = 0U;
	sint32 s32State = E_NOT_OK;
	MSGQUEUE_tstMsgSelectBoth stReadMsg = {(uint32)CTRLEPROM_CONTROL_PID_cfg,0U,0U};
	void* pvMsg = (void*)NULL;

	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u32KernelTime);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);

	u32Time = RTC_u32GetKernelClock() - CTRLEPROM_s_u32KernelTime;

	pvMsg = MSGQUEUE_pvRxMsg(MSGQUEUE_nNO_TMO,(void*)&stReadMsg ,(uint32)MSGQUEUE_nMSGRX_BOTH);

	if(u32Time >= CTRLEPROM_TIMEOUT_REQ)
	{
		CTRLEPROM_s_s32StateWriteMsg = E_NOT_OK;
	}
	else if((pvMsg == (void*)NULL))
	{
		CTRLEPROM_s_s32StateWriteMsg = (sint32)STD_NACK;
	}
	else
	{
		CTRLEPROM_s_s32StateWriteMsg = (sint32)STD_ACK;
	}

	CTRLEPROM_s_u8FMS = (uint8)CTRLEPROM_s_nWITH_OUT_REQS;

}

/* public_functions */
void CTRLEPROM_vInit(void)
{
	CTRLEPROM_s_tstMsg* pstMsg;

	uint32 u32DataR = (uint32)CTRLEPROM_s_nZERO;
	uint32 u32DataW = (uint32)CTRLEPROM_s_nZERO;
//	CTRLEPROM_s_pvAddr = (void*)pstMsg;

	CTRLEPROM_s_pvAddr = MSGQUEUE_pvMsgAllocClr((uint16)(sizeof(pstMsg) + (uint16)sizeof(uint32)),
			    	(uint32)CTRLEPROM_MSG_POOL_cfg,
					(uint32)CTRLEPROM_REQ_MEM_R_ID_cfg,
					(uint32)MSGQUEUE_nNO_TMO);


	MSGQUEUE_vTxMsg((void**)&CTRLEPROM_s_pvAddr, (uint32)CTRLEPROM_CONTROL_PID_cfg);

	CTRLEPROM_s_u8Tries = (uint8)CTRLEPROM_s_nZERO;
	CTRLEPROM_s_u8FMS = (uint8)CTRLEPROM_s_nWITH_OUT_REQS;
	CTRLEPROM_s_pu32ReadMsg = &u32DataR;
	CTRLEPROM_s_pu32WriteMsg = &u32DataW;
	CTRLEPROM_s_s32StateReadMsg = (sint32)CTRLEPROM_s_nZERO;
	CTRLEPROM_s_s32StateWriteMsg = (sint32)CTRLEPROM_s_nZERO;
	CTRLEPROM_s_u32KernelTime = RTC_u32GetKernelClock();
	CTRLEPROM_s_u16BufferSize = (uint16)CTRLEPROM_s_nZERO;

}

void CTRLEPROM_vMonitor(void)
{
	sint32 s32State = E_NOT_OK;

	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u8FMS);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);

	switch(CTRLEPROM_s_u8FMS)
	{
	case (uint8)CTRLEPROM_s_nREAD:
		/*Proceso de lectura*/
		CTRLEPROM_s_vReading();
		break;

	case (uint8)CTRLEPROM_s_nWRITE:
		/*Proceso de escritura*/
		CTRLEPROM_s_vWriting();
		break;

	case (uint8)CTRLEPROM_s_nWAIT_STATE_ANS_READ:
		/*Espara el estatus de la respuesta de lectura en la memoria externa*/
		CTRLEPROM_s_vWaitStateRead();
		break;

	case (uint8)CTRLEPROM_s_nWAIT_STATE_ANS_WRITE:
		/*Espara el estatus de la respuesta de escritura en la memoria externa*/
		CTRLEPROM_s_vWaitStateWrite();
		break;

	default:
		/*En espera de recibir una solicitud de lectura o escritura*/
		/*Do nothing*/
		break;

	}
}

sint32 CTRLEPROM_s32ReqReadMem(uint32 u32MemAddr, uint16 u16BufferSize)
{
	sint32 s32State = E_NOT_OK;
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u8FMS);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);

	if(CTRLEPROM_s_u8FMS != (uint8)CTRLEPROM_s_nWITH_OUT_REQS)
	{
		s32State = E_NOT_OK;

	}
	else
	{
		CTRLEPROM_s_u32Addr = u32MemAddr;
		CTRLEPROM_s_u16BufferSize = u16BufferSize;
		CTRLEPROM_s_u8FMS = (uint8)CTRLEPROM_s_nREAD;

		s32State = E_OK;



	}

	return s32State;
}

sint32 CTRLEPROM_s32ReqWriteMem(uint32 u32MemAddr, uint8* pu8Buffer, uint16 u16BufferSize)
{
	sint32 s32State = E_NOT_OK;
	uint16 u16i = 0U;
	uint32 au32AuxData [CTRLEPROM_s_nMAX_LEN] = {0U};

	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_u8FMS);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA);
	ASSERT_HARD_ERROR(pu8Buffer != (uint8*)NULL,E_NULL_POINTER);

	if(CTRLEPROM_s_u8FMS != (uint8)CTRLEPROM_s_nWITH_OUT_REQS)
	{
		s32State = E_NOT_OK;

	}
	else
	{
		CTRLEPROM_s_u32Addr = u32MemAddr;
		CTRLEPROM_s_u16BufferSize = u16BufferSize;

		for(u16i = 0; u16i < u16BufferSize; u16i++)
		{
			au32AuxData[u16i] = (uint32)pu8Buffer[u16i];

		}

		CTRLEPROM_s_pu32WriteMsg = au32AuxData;
		s32State = E_OK;
		CTRLEPROM_s_u8FMS = (uint8)CTRLEPROM_s_nWRITE;

		DATACOMP_UPDATE_PER_BYTE(CTRLEPROM_s_pu32WriteMsg);
		DATACOMP_UPDATE_PER_BYTE(CTRLEPROM_s_u32Addr);
		DATACOMP_UPDATE_PER_BYTE(CTRLEPROM_s_u16BufferSize);
		DATACOMP_UPDATE_PER_BYTE(CTRLEPROM_s_u8FMS);

	}

	return s32State;
}

sint32 CTRLEPROM_s32GetReadMemResp(uint8* pu8Buffer, uint16 u16BufferSize)
{
	sint32 s32State = E_NOT_OK;
	uint16 u16i = 0U;

	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_s32StateReadMsg);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA)
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_pu32ReadMsg);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA)
	ASSERT_HARD_ERROR(pu8Buffer != (uint8*)NULL,E_NULL_POINTER);


	CTRLEPROM_s_u16BufferSize = u16BufferSize;

	for(u16i = 0; u16i < u16BufferSize; u16i++)
	{
		pu8Buffer= (uint8*)CTRLEPROM_s_pu32ReadMsg;

	}

	DATACOMP_UPDATE_PER_BYTE(CTRLEPROM_s_u16BufferSize);

	return CTRLEPROM_s_s32StateReadMsg;
}


sint32 CTRLEPROM_s32GetWriteMemResp(void)
{
	sint32 s32State = E_NOT_OK;
	s32State = DATACOMP_CHECK_PER_BYTE(CTRLEPROM_s_s32StateWriteMsg);
	ASSERT_HARD_ERROR(s32State == E_OK, E_CORRUPT_DATA)

	return CTRLEPROM_s_s32StateWriteMsg;
}

/* EOF */
