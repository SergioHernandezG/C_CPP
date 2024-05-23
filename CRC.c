/* internal__pre-processor macros */
#define CRC_LUT_s_nMAX_LEN		        (1024U)

#define CRC_LUT_s_nONE_BIT		    (1U)
#define CRC_LUT_s_n8_BITS		    (8U)
#define CRC_LUT_s_n16_BITS			(16U)
#define CRC_LUT_s_n24_BITS			(24U)
#define CRC_LUT_s_n32_BITS			(32U)


static uint32 CRC_LUT_s_u32Reverse(uint32 u32Data, uint8 u8Len)
{
    uint8 u8i;
    uint32 u32DataReverse = 0U;

    for (u8i = 0; u8i < u8Len; u8i ++)
    {
    	u32DataReverse |= ((u32Data >> u8i) & CRC_LUT_s_nONE_BIT) << ((u8Len - CRC_LUT_s_nONE_BIT) - u8i);
    }

    return u32DataReverse;
}


static uint32 CRC_LUT_s_u32CalculateCRC(uint8  u8Data, CRC_LUT_tstPolValues stPolyData)
{
    uint32 u32Poly;
    uint32 u32Crc;
    uint8 u8i;
    uint8 u8AuxData = u8Data;
    uint32 u32Flag;

    if(stPolyData.u32ReInput == (uint32)TRUE)
    {
    	u8AuxData = (uint8)CRC_LUT_s_u32Reverse((uint32)u8AuxData, (uint8)CRC_LUT_s_n8_BITS );
    }
    else
    {
    	/*Nothing to do*/
    }

    u32Crc = ((uint32)u8AuxData << (stPolyData.u32LBits-(uint32)CRC_LUT_s_n8_BITS)) ^ stPolyData.u32InitVal;
    u32Poly = (stPolyData.u32Poly >> (uint32)CRC_LUT_s_nONE_BIT) | ((uint32)CRC_LUT_s_nONE_BIT << (stPolyData.u32LBits-(uint32)CRC_LUT_s_nONE_BIT));

    for(u8i = (uint8)0U; u8i < CRC_LUT_s_n8_BITS; u8i++)
    {
    	u32Flag = u32Crc & ((uint32)CRC_LUT_s_nONE_BIT << (stPolyData.u32LBits-(uint32)CRC_LUT_s_nONE_BIT));

        if (u32Flag != (uint32)FALSE)
        {
        	u32Crc = (u32Crc ^ u32Poly);
        	u32Crc <<= CRC_LUT_s_nONE_BIT;
            u32Crc = u32Crc | CRC_LUT_s_nONE_BIT;
        }
        else
        {
        	u32Crc <<= CRC_LUT_s_nONE_BIT;
        }
    }

    return u32Crc;
}

/* public_functions */
void CRC_LUT_vGetGenCRC(const uint8*  pu8Data, uint16 u16Len, uint32* pu32CRC, CRC_LUT_tstPolValues* pstPolyData)
{
    CRC_LUT_tstPolValues stType = *pstPolyData;
    uint32 u32Crc;
    uint8 u8i;

    u32Crc = stType.u32InitVal;

    for(u8i = 0U; u8i < (uint8)u16Len; u8i++)
    {
    	stType.u32InitVal = u32Crc;
    	u32Crc = CRC_LUT_s_u32CalculateCRC(pu8Data[u8i], stType);
    }

    u32Crc = (u32Crc ^ stType.u32FinalVal);

    if(stType.u32ReOutput == (uint32)TRUE)
    {
    	*pu32CRC = CRC_LUT_s_u32Reverse(u32Crc, (uint8)stType.u32LBits);
    }
    else
    {
    	*pu32CRC = u32Crc;
    }
}


