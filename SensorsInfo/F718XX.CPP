#include "StdAfx.h"
#include "Ring0.h"
#include "./lpc/ISuperIO.h"
#include "./lpc/F718XX.h"


extern Ring0 m_ring0;
F718XX::F718XX(Chip chip, ushort address)
{
      this->address = address;
      this->chip = chip;

      voltages = new float[chip == F71858 ? 3 : 9];
	  voltageslength = chip == F71858 ? 3 : 9;
      temperatures = new float[3];
	  temperatureslength = 3;
      fans = new float[chip == F71882 || chip == F71858? 4 : 3];
      fanscount = chip == F71882 || chip == F71858? 4 : 3;

}

byte F718XX::ReadByte(byte m_register)
{

      m_ring0.WriteIoPort(
        (ushort)(address + F718_ADDRESS_REGISTER_OFFSET), m_register);
      return m_ring0.ReadIoPort((ushort)(address + F718_DATA_REGISTER_OFFSET));
}

void F718XX::Update()
{
      if (!m_ring0.WaitIsaBusMutex(10))
        return;
       m_sensorcount = 0;
	   int i=0;
      for (i = 0; i <voltageslength; i++) {
        int value = ReadByte((byte)(F718_VOLTAGE_BASE_REG + i));
        voltages[i] = 0.008f * value;
		m_sensorvalue[m_sensorcount].m_type = TVOLTAGE;
        m_sensorvalue[m_sensorcount].m_description = "µçÑ¹´«¸Ð";
		m_sensorvalue[m_sensorcount].values = voltages[i];
		m_sensorcount++;
      }
     
      for (i = 0; i < temperatureslength; i++) {
        switch (chip) {
          case F71858: {
              int tableMode = 0x3 & ReadByte(F718_TEMPERATURE_CONFIG_REG);
              int high = 
                ReadByte((byte)(F718_TEMPERATURE_BASE_REG + 2 * i));
              int low =
                ReadByte((byte)(F718_TEMPERATURE_BASE_REG + 2 * i + 1));              
              if (high != 0xbb && high != 0xcc) {
                int bits = 0;
                switch (tableMode) {
                  case 0: bits = 0; break;
                  case 1: bits = 0; break;
                  case 2: bits = (high & 0x80) << 8; break;
                  case 3: bits = (low & 0x01) << 15; break;
                }
                bits |= high << 7;
                bits |= (low & 0xe0) >> 1;
                short value = (short)(bits & 0xfff0);
                temperatures[i] = value / 128.0f;
              } else {
                temperatures[i] = NULL;
              }
          } break;
          default: {
            sbyte value = (sbyte)ReadByte((byte)(
              F718_TEMPERATURE_BASE_REG + 2 * (i + 1)));            
            if (value < 127 && value > 0)
              temperatures[i] = value;
            else
              temperatures[i] = NULL;
          } break;
        }
		if(temperatures[i]>=0.1f)
		{
	      m_sensorvalue[m_sensorcount].m_type = TTEMPERATURE;
          m_sensorvalue[m_sensorcount].m_description = F718_TemperDescription[i];
	      m_sensorvalue[m_sensorcount].values = temperatures[i];
	      m_sensorcount++;
		}
      }

      for ( i = 0; i < fanscount; i++) {
        int value = ReadByte(F718_FAN_TACHOMETER_REG[i]) << 8;
        value |= ReadByte((byte)(F718_FAN_TACHOMETER_REG[i] + 1));

        if (value >= 0.1f) 
        {  fans[i] = (value < 0x0fff) ? 1.5e6f / value : 0;
		   m_sensorvalue[m_sensorcount].m_type = FANSPEED;
           m_sensorvalue[m_sensorcount].m_description = F718_FanDescription[i];
	       m_sensorvalue[m_sensorcount].values = fans[i];
		   m_sensorcount++;
        }
        else 
          fans[i] = NULL;   
	
      }

      m_ring0.ReleaseIsaBusMutex();
}