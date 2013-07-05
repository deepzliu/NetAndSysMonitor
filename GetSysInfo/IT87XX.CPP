//#include "StdAfx.h"
#include "Ring0.h"
#include "lpc/ISuperIO.h"
#include "./lpc/IT87XX.h"

extern Ring0 m_ring0;

#if 1
byte IT87XX::ReadByte(byte m_register, bool &valid)
{
      m_ring0.WriteIoPort(addressReg, m_register);
      byte value = m_ring0.ReadIoPort(dataReg);
      valid = m_register == m_ring0.ReadIoPort(addressReg);
      return value;
}
#else
byte IT87XX::ReadByte(byte m_register, bool &valid)
{
	USHORT addw = 0xE85, addr = 0xE86;
      m_ring0.WriteIoPort(addw, m_register);
      byte value = m_ring0.ReadIoPort(addr);
      valid = m_register == m_ring0.ReadIoPort(addw);
      return value;
}
#endif
byte IT87XX::ReadGPIO(int index)
{
	  if (index >= gpioCount)
        return NULL;
      return m_ring0.ReadIoPort((USHORT)(gpioAddress + index));
}

void IT87XX::WriteGPIO(int index, byte value)
{
     if (index >= gpioCount)
        return;
      m_ring0.WriteIoPort((USHORT)(gpioAddress + index), value);
}

IT87XX::IT87XX(Chip chip, USHORT address, USHORT gpioAddress, byte version)
{
      
      this->address = address;
      this->chip = chip;
      this->version = version;
      this->addressReg = (USHORT)(address +ITE_ADDRESS_REGISTER_OFFSET);
      this->dataReg = (USHORT)(address + ITE_DATA_REGISTER_OFFSET);
      this->gpioAddress = gpioAddress;

      // Check vendor id
      bool valid;
      byte vendorId = ReadByte(ITE_VENDOR_ID_REGISTER, valid);       
      if (!valid || vendorId != ITE_VENDOR_ID)
        return;

      // Bit 0x10 of the configuration register should always be 1
      if ((ReadByte(ITE_CONFIGURATION_REGISTER, valid) & 0x10) == 0)
        return;
      if (!valid)
        return;
      // The IT8721F uses a 12mV resultion ADC, all others 16mV
      if (chip == IT8721F) {
        voltageGain = 0.012f;
      } else {
        voltageGain = 0.016f;
      }

      // Set the number of GPIO sets
      switch (chip) {
        case IT8712F:
        case IT8716F:
        case IT8718F:
        case IT8726F:
          gpioCount = 5;
          break;
        case IT8720F:
        case IT8721F:
          gpioCount = 8;
          break;
 }
}

void IT87XX::Update(void)
{
      if (!m_ring0.WaitIsaBusMutex(10))
            return;
	  m_sensorcount=0;
	  int i=0;
      for (i = 0; i < sizeof(voltages)/sizeof(float); i++) {
        bool valid;
      float value = 
          voltageGain * ReadByte((byte)(ITE_VOLTAGE_BASE_REG + i),  valid);   

        if (!valid)
          continue;
        if (value > 0)
          voltages[i] = value;  
        else
          voltages[i] = 0;
		m_sensorvalue[m_sensorcount].m_type = TVOLTAGE;
        m_sensorvalue[m_sensorcount].m_description = "µçÑ¹´«¸Ð";
		m_sensorvalue[m_sensorcount].values = voltages[i];
		m_sensorcount++;
      }

      for (i = 0; i < sizeof(temperatures)/sizeof(float); i++) {
        bool valid;
        sbyte value = (sbyte)ReadByte(
         (byte)(ITE_TEMPERATURE_BASE_REG + i),valid);

        if (!valid)
          continue;

        if (value < 128 && value > 0.1f)
		{
			temperatures[i] = value;
		    m_sensorvalue[m_sensorcount].m_type = TTEMPERATURE;
            m_sensorvalue[m_sensorcount].m_description = ITE_TemperDescription[i];
	        m_sensorvalue[m_sensorcount].values = temperatures[i];
			m_sensorvalue[m_sensorcount].reg = ITE_TEMPERATURE_BASE_REG + i;//Liudonghua
	        m_sensorcount++;
		}
        else
          temperatures[i] = NULL;
	
      }

      for (i = 0; i <sizeof(fans)/sizeof(float); i++) {
        bool valid;
        int value = ReadByte(ITE_FAN_TACHOMETER_REG[i],  valid);
        if (!valid) 
          continue;
        value |= ReadByte(ITE_FAN_TACHOMETER_EXT_REG[i], valid) << 8;
        if (!valid)
          continue;

        if (value > 0x3f) {
          fans[i] = (value < 0xffff) ? 1.35e6f / ((value) * 2) : 0;
		  if(fans[i]>=0.1f)
		  {
		    m_sensorvalue[m_sensorcount].m_type = FANSPEED;
            m_sensorvalue[m_sensorcount].m_description=ITE_FanDescription[i];
	        m_sensorvalue[m_sensorcount].values = fans[i];
		    m_sensorcount++;
		  }
        } else {
          fans[i] = NULL;
        }
	    
      }
	  
      m_ring0.ReleaseIsaBusMutex();
}