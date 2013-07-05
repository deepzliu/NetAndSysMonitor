//#include "StdAfx.h"
#include "Ring0.h"
#include "./lpc/ISuperIO.h"
#include "./lpc/W836XX.h"

extern Ring0 m_ring0;
byte W836XX::ReadByte(byte bank, byte m_register)
{
	m_ring0.WriteIoPort(
		(ushort)(address + WIN_ADDRESS_REGISTER_OFFSET), WIN_BANK_SELECT_REGISTER);
	m_ring0.WriteIoPort(
		(ushort)(address + WIN_DATA_REGISTER_OFFSET), bank);
	m_ring0.WriteIoPort(
		(ushort)(address + WIN_ADDRESS_REGISTER_OFFSET), m_register);
	return m_ring0.ReadIoPort(
        (ushort)(address + WIN_DATA_REGISTER_OFFSET));
}

void W836XX::WriteByte(byte bank, byte m_register, byte value)
{
	m_ring0.WriteIoPort(
		(ushort)(address + WIN_ADDRESS_REGISTER_OFFSET), WIN_BANK_SELECT_REGISTER);
	m_ring0.WriteIoPort(
		(ushort)(address + WIN_DATA_REGISTER_OFFSET), bank);
	m_ring0.WriteIoPort(
		(ushort)(address + WIN_ADDRESS_REGISTER_OFFSET), m_register);
	m_ring0.WriteIoPort(
		(ushort)(address + WIN_DATA_REGISTER_OFFSET), value); 
}

void W836XX::Update(){
	if (!m_ring0.WaitIsaBusMutex(10))
        return;
	m_sensorcount=0;
	int i=0;
    for (i = 0; i < voltageRegistercount; i++) {
        if (voltageRegister[i] != WIN_VOLTAGE_VBAT_REG) {
			// two special VCore measurement modes for W83627THF
			float fvalue;
			if ((chip ==  W83627HF || chip ==  W83627THF || 
				chip ==  W83687THF) && i == 0) 
			{
				byte vrmConfiguration = ReadByte(0, 0x18);
				int value = ReadByte(voltageBank[i], voltageRegister[i]);
				if ((vrmConfiguration & 0x01) == 0)
					fvalue = 0.016f * value; // VRM8 formula
				else
					fvalue = 0.00488f * value + 0.69f; // VRM9 formula
					
			} else 
			{
				int value = ReadByte(voltageBank[i], voltageRegister[i]);
				fvalue = voltageGain * value;
			}
			if (fvalue > 0)
				voltages[i] = fvalue;
			else
				voltages[i] = 0;
			m_sensorvalue[m_sensorcount].m_type = TVOLTAGE;
            m_sensorvalue[m_sensorcount].m_description = "电压传感";
			m_sensorvalue[m_sensorcount].values = voltages[i];
			m_sensorcount++;
			
        }
		else 
		{
			// Battery voltage
			bool valid = (ReadByte(0, 0x5D) & 0x01) > 0;
			if (valid) {
				voltages[i] = voltageGain * ReadByte(5, WIN_VOLTAGE_VBAT_REG);
			} else 
			{
				voltages[i] = 0;
			}
			m_sensorvalue[m_sensorcount].m_type = TVOLTAGE;
            m_sensorvalue[m_sensorcount].m_description = "电池电压";
			m_sensorvalue[m_sensorcount].values = voltages[i];
			m_sensorcount++;
		}
	}
	
	for (i = 0; i <WIN_tempratureregcount; i++) {
        int value = ((sbyte)ReadByte(WIN_TEMPERATURE_BANK[i], 
			WIN_TEMPERATURE_REG[i])) << 1;
        if (WIN_TEMPERATURE_BANK[i] > 0) 
			value |= ReadByte(WIN_TEMPERATURE_BANK[i],
            (byte)(WIN_TEMPERATURE_REG[i] + 1)) >> 7;
		
        float temperature = value / 2.0f;
        if (temperature <= 125 && temperature >= -55 && !peciTemperature[i]) {
			temperatures[i] = temperature;
			m_sensorvalue[m_sensorcount].m_type = TTEMPERATURE;
            m_sensorvalue[m_sensorcount].m_description=TEMPERATUREDescription[i];
	        m_sensorvalue[m_sensorcount].values = temperatures[i];
	        m_sensorcount++;
        } else {
			temperatures[i] = 0;
        }
	

	}
	
	UINT64 bits = 0;
	for (i = 0; i < WIN_fanbitregcount; i++)
        bits = (bits << 8) | ReadByte(0, WIN_FAN_BIT_REG[i]);
	ulong newBits = bits;
	for (i = 0; i < WIN_fanbitregcount; i++) {
        int count = ReadByte(WIN_FAN_TACHO_BANK[i], WIN_FAN_TACHO_REG[i]);
        
        // assemble fan divisor
        int divisorBits = (int)(
			(((bits >> WIN_FAN_DIV_BIT2[i]) & 1) << 2) |
			(((bits >> WIN_FAN_DIV_BIT1[i]) & 1) << 1) |
												((bits >> WIN_FAN_DIV_BIT0[i]) & 1));
        int divisor = 1 << divisorBits;
		if(count!=0 && count!=0xff)
		{
         float value = (count < 0xff) ? 1.35e6f / (count * divisor) : 0;
		 if(value>0.1f)
		 {
           fans[i] = value;
           m_sensorvalue[m_sensorcount].m_type = FANSPEED;
           m_sensorvalue[m_sensorcount].m_description =FANDesription[i];
	       m_sensorvalue[m_sensorcount].values = fans[i];
		   m_sensorcount++;
		 }
		}
        // update fan divisor
        if (count > 192 && divisorBits < 7) 
			divisorBits++;
        if (count < 96 && divisorBits > 0)
			divisorBits--;
		
        newBits = SetBit(newBits, WIN_FAN_DIV_BIT2[i], (divisorBits >> 2) & 1);
        newBits = SetBit(newBits, WIN_FAN_DIV_BIT1[i], (divisorBits >> 1) & 1);
        newBits = SetBit(newBits, WIN_FAN_DIV_BIT0[i], divisorBits & 1);
	}
	
	// write new fan divisors 
	for (i = WIN_fanbitregcount - 1; i >= 0; i--) {
        byte oldByte = (byte)(bits & 0xFF);
        byte newByte = (byte)(newBits & 0xFF);
        bits = bits >> 8;
        newBits = newBits >> 8;
        if (oldByte != newByte) 
			WriteByte(0, WIN_FAN_BIT_REG[i], newByte);        
	}
	

	m_ring0.ReleaseIsaBusMutex();
}

