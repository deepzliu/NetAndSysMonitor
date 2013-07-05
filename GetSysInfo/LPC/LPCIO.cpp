//#include "StdAfx.h"
#include "ISuperIO.h"
#include "../Sensors.h"
#include <winioctl.h>
#include "../Ring0.h"
#include "LPCIO.H"
#include "../lpc/F718XX.h"
#include "../lpc/IT87XX.h"
#include "../lpc/W836XX.h"
using namespace  Chips;
extern Ring0 m_ring0;

USHORT LPCIO::ReadWord(byte m_register)
{
	 return (USHORT)((ReadByte(m_register) << 8) |
        ReadByte((byte)(m_register + 1)));
}

byte LPCIO::ReadByte(byte m_register)
{
  m_ring0.WriteIoPort(registerPort, m_register);
      return m_ring0.ReadIoPort(valueport);
}

bool LPCIO::DetectWinbondFintek()
{
  WinbondNuvotonFintekEnter();

      byte logicalDeviceNumber = 0;
      byte id = ReadByte(CHIP_ID_REGISTER);
      byte revision = ReadByte(CHIP_REVISION_REGISTER);
	  Chip chip = Chip::Unknown;

	  switch (id) {
		case 0x05:
			switch (revision) {
		case 0x07:
			chip = Chip::F71858;
			logicalDeviceNumber = F71858_HARDWARE_MONITOR_LDN;
			break;
		case 0x41:
			chip = Chip::F71882;
			logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x06:
			switch (revision) {
		case 0x01:
			chip = Chip::F71862;
			logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x07:
			switch (revision) {
		case 0x23:
			chip = Chip::F71889F;
			logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x08:
			switch (revision) {
		case 0x14:
			chip = Chip::F71869;
			logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x09:
			switch (revision) {
		case 0x09:
			chip = Chip::F71889ED;
			logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x10:
			switch (revision) {
		case 0x05:
			chip = Chip::F71889AD;
			logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x52:
			switch (revision) {
		case 0x17:
		case 0x3A:
		case 0x41:
			chip = Chip::W83627HF;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x82:
			switch (revision & 0xF0) {
		case 0x80:
			chip = Chip::W83627THF;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x85:
			switch (revision) {
		case 0x41:
			chip = Chip::W83687THF;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0x88:
			switch (revision & 0xF0) {
		case 0x50:
		case 0x60:
			chip = Chip::W83627EHF;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0xA0:
			switch (revision & 0xF0) {
		case 0x20:
			chip = Chip::W83627DHG;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0xA5:
			switch (revision & 0xF0) {
		case 0x10:
			chip = Chip::W83667HG;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0xB0:
			switch (revision & 0xF0) {
		case 0x70:
			chip = Chip::W83627DHGP;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0xB3:
			switch (revision & 0xF0) {
		case 0x50:
			chip = Chip::W83667HGB;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0xB4:
			switch (revision & 0xF0) {
		case 0x70:
			chip = Chip::NCT6771F;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
		case 0xC3:
			switch (revision & 0xF0) {
		case 0x30:
			chip = Chip::NCT6776F;
			logicalDeviceNumber = WINBOND_NUVOTON_HARDWARE_MONITOR_LDN;
			break;
			} break;
	  
      } 
	  if (chip == Chip::Unknown) {
		  if (id != 0 && id != 0xff) {
			  WinbondNuvotonFintekExit();

		  }
	  }
      else {

        Select(logicalDeviceNumber);
        USHORT address = ReadWord(BASE_ADDRESS_REGISTER);
		Sleep(1);
        USHORT verify = ReadWord(BASE_ADDRESS_REGISTER);

        USHORT vendorID = ReadWord(FINTEK_VENDOR_ID_REGISTER);

        WinbondNuvotonFintekExit();

        if (address != verify) {
         // report.Append(CString("Chip ID: 0x"));
         // report.AppendLine(CString(ToString("X")));
         // report.Append("Chip revision: 0x");
         // report.AppendLine(revision.ToString("X",
         //   CultureInfo.InvariantCulture));
        //  report.AppendLine("Error: Address verification failed");
         // report.AppendLine();
          return false;
        }

        // some Fintek chips have address register offset 0x05 added already
        if ((address & 0x07) == 0x05)
          address &= 0xFFF8;

        if (address < 0x100 || (address & 0xF007) != 0) {
       /*   report.Append("Chip ID: 0x");
          report.AppendLine(ToString("X"));
          report.Append("Chip revision: 0x");
          report.AppendLine(revision.ToString("X",
            CultureInfo.InvariantCulture));
          report.Append("Error: Invalid address 0x");
          report.AppendLine(address.ToString("X",
            CultureInfo.InvariantCulture));
          report.AppendLine();*/
          return false;
        }

        switch (chip) {
		  case Chip::W83627DHG:
		  case Chip::W83627DHGP:
		  case Chip::W83627EHF:
		  case Chip::W83627HF:
		  case Chip::W83627THF:
		  case Chip::W83667HG:
		  case Chip::W83667HGB:
          case W83687THF:
            //superIOs.Add(new W836XX(chip, revision, address));
			  pSuperIO = (ISuperIO*) new W836XX(chip,revision,address);
            break;
		  case Chip::NCT6771F:
		  case Chip::NCT6776F:
			//  pSuperIO =(ISuperIO*)  new NCT677X(chip, revision, address);
			  break;
		  case Chip::F71858:
		  case Chip::F71862:
		  case Chip::F71869:
		  case Chip::F71882:
		  case Chip::F71889AD:
		  case Chip::F71889ED:
		  case Chip::F71889F:
            if (vendorID != FINTEK_VENDOR_ID) {
              //report.Append("Chip ID: 0x");
              //report.AppendLine(ToString("X"));
              //report.Append("Chip revision: 0x");
              //report.AppendLine(revision.ToString("X",
              //  CultureInfo.InvariantCulture));
              //report.Append("Error: Invalid vendor ID 0x");
              //report.AppendLine(vendorID.ToString("X",
               // CultureInfo.InvariantCulture));
              //report.AppendLine();
              return false;
            }
            //superIOs.Add(new F718XX(chip, address));
			pSuperIO =(ISuperIO*) new F718XX(chip,address);
            break;
          default: break;
        }

        return true;
      }
    return false;
}

void LPCIO::WinbondNuvotonFintekEnter()
{
   m_ring0.WriteIoPort(registerPort, 0x87);
   m_ring0.WriteIoPort(registerPort, 0x87);
}

void LPCIO::WinbondNuvotonFintekExit()
{
  m_ring0.WriteIoPort(registerPort, 0xAA);
}

void LPCIO::Select(byte logicalDeviceNumber)
{
	 m_ring0.WriteIoPort(registerPort, DEVCIE_SELECT_REGISTER);
     m_ring0.WriteIoPort(valueport, logicalDeviceNumber);
}

void LPCIO::ReportUNKnowChip(string type, int chip)
{
	
}


bool LPCIO::DetectIT87()
{
	    // IT87XX can enter only on port 0x2E
      if (registerPort != 0x2E)
        return false;

      IT87Enter();

      ushort chipID = ReadWord(CHIP_ID_REGISTER);
      Chip chip;
      switch (chipID) {
		 case 0x8712: chip = Chip::IT8712F; break;
		 case 0x8716: chip = Chip::IT8716F; break;
		 case 0x8718: chip = Chip::IT8718F; break;
		 case 0x8720: chip = Chip::IT8720F; break;
		 case 0x8721: chip = Chip::IT8721F; break;
		 case 0x8726: chip = Chip::IT8726F; break;
		 case 0x8728: chip = Chip::IT8728F; break;
		 case 0x8771: chip = Chip::IT8771E; break;
		 case 0x8772: chip = Chip::IT8772E; break;
		 default: chip = Chip::Unknown; break;
      }
	  if (chip ==  Chip::Unknown) {
        if (chipID != 0 && chipID != 0xffff) {
          IT87Exit();

       //   ReportUnknownChip("ITE", chipID);
        }
      } else {
        Select(IT87_ENVIRONMENT_CONTROLLER_LDN);
        ushort address = ReadWord(BASE_ADDRESS_REGISTER);
        Sleep(1);
        ushort verify = ReadWord(BASE_ADDRESS_REGISTER);

        byte version = (byte)(ReadByte(IT87_CHIP_VERSION_REGISTER) & 0x0F);

        Select(IT87_GPIO_LDN);
        ushort gpioAddress = ReadWord(BASE_ADDRESS_REGISTER + 2);
        Sleep(1);
        ushort gpioVerify = ReadWord(BASE_ADDRESS_REGISTER + 2);

		if(gpioAddress == 0){
			gpioAddress = 2624;
			gpioVerify = 2624;
		}

        IT87Exit();

        if (address != verify || address < 0x100 || (address & 0xF007) != 0) {/*
          report.Append("Chip ID: 0x");
          report.AppendLine( ToString("X"));
          report.Append("Error: Invalid address 0x");
          report.AppendLine(address.ToString("X",
            CultureInfo.InvariantCulture));
          report.AppendLine();*/
          return false;
        }

        if (gpioAddress != gpioVerify || gpioAddress < 0x100 ||
          (gpioAddress & 0xF007) != 0) {/*
          report.Append("Chip ID: 0x");
          report.AppendLine( ToString("X"));
          report.Append("Error: Invalid GPIO address 0x");
          report.AppendLine(gpioAddress.ToString("X",
            CultureInfo.InvariantCulture));
          report.AppendLine();*/
          return false;
        }

      //  superIOs.Add(new IT87XX(chip, address, gpioAddress, version));
		pSuperIO =(ISuperIO*) new IT87XX(chip,address,gpioAddress,version);
		return true;
      }

      return false;
}


void LPCIO::IT87Enter()
 {
      m_ring0.WriteIoPort(registerPort, 0x87);
      m_ring0.WriteIoPort(registerPort, 0x01);
      m_ring0.WriteIoPort(registerPort, 0x55);
      m_ring0.WriteIoPort(registerPort, 0x55);
}

void LPCIO::IT87Exit()
{
      m_ring0.WriteIoPort(registerPort, CONFIGURATION_CONTROL_REGISTER);
      m_ring0.WriteIoPort(valueport, 0x02);
}

void LPCIO::SMSCEnter()
{
  m_ring0.WriteIoPort(registerPort, 0x55);
}
void LPCIO::SMSCExit()
{
  m_ring0.WriteIoPort(registerPort, 0xAA);
}

bool LPCIO::DetectSMSC()
{
	  SMSCEnter();

      ushort chipID = ReadWord(CHIP_ID_REGISTER);
      Chip chip;
      switch (chipID) {
		default: chip =  Chip::Unknown; break;
      }
	  if (chip ==  Chip::Unknown) {
        if (chipID != 0 && chipID != 0xffff) {
          SMSCExit();

         // ReportUnknownChip("SMSC", chipID);
        }
      } else {
        SMSCExit();
        return true;
      }

      return false;
}

void LPCIO::Detect()
 {

      for (int i = 0; i <2; i++) {
        registerPort = REGISTER_PORTS[i];
        valueport = VALUE_PORTS[i];

        if (DetectWinbondFintek()) continue;

        if (DetectIT87()) continue;

        if (DetectSMSC()) continue;
      }
}

LPCIO::LPCIO() {
      if (!m_ring0.IsOpen())
        return;
      pSuperIO = NULL;
      if (!m_ring0.WaitIsaBusMutex(100))
        return;

      Detect();

      m_ring0.ReleaseIsaBusMutex();
}