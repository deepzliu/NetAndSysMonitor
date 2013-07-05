/*
  
  Version: MPL 1.1/GPL 2.0/LGPL 2.1

  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
 
  http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the License.

  The Original Code is the Open Hardware Monitor code.

  The Initial Developer of the Original Code is 
  Michael M?ller <m.moeller@gmx.ch>.
  Portions created by the Initial Developer are Copyright (C) 2009-2010
  the Initial Developer. All Rights Reserved.

  Contributor(s):

  Alternatively, the contents of this file may be used under the terms of
  either the GNU General Public License Version 2 or later (the "GPL"), or
  the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
  in which case the provisions of the GPL or the LGPL are applicable instead
  of those above. If you wish to allow use of your version of this file only
  under the terms of either the GPL or the LGPL, and not to allow others to
  use your version of this file under the terms of the MPL, indicate your
  decision by deleting the provisions above and replace them with the notice
  and other provisions required by the GPL or the LGPL. If you do not delete
  the provisions above, a recipient may use your version of this file under
  the terms of any one of the MPL, the GPL or the LGPL.
 
*/


// converter c# to c++ by zhao
// 2010.11.15
#ifndef _LPCIO_H_
#define _LPCIO_H_

#include "chip.h"
#include "ISuperIO.h"

      const byte FINTEK_VENDOR_ID_REGISTER = 0x23;
      const USHORT FINTEK_VENDOR_ID = 0x1934;
      const byte WINBOND_HARDWARE_MONITOR_LDN = 0x0B;
      const byte F71858_HARDWARE_MONITOR_LDN = 0x02;
      const byte FINTEK_HARDWARE_MONITOR_LDN = 0x04;

	  const byte WINBOND_NUVOTON_HARDWARE_MONITOR_LDN = 0x0b;



	

	  const byte CONFIGURATION_CONTROL_REGISTER = 0x02;
      const byte DEVCIE_SELECT_REGISTER = 0x07;
      const byte CHIP_ID_REGISTER = 0x20;
      const byte CHIP_REVISION_REGISTER = 0x21;
      const byte BASE_ADDRESS_REGISTER = 0x60;

	  const byte IT87_ENVIRONMENT_CONTROLLER_LDN = 0x04;
      const byte IT87_GPIO_LDN = 0x07;
      const byte IT87_CHIP_VERSION_REGISTER = 0x22;
      
	  const USHORT REGISTER_PORTS[2] = { 0x2E, 0x4E };
      const USHORT VALUE_PORTS[2] = { 0x2F, 0x4F };


//	   const byte IT87_ENVIRONMENT_CONTROLLER_LDN = 0x04;
//     const byte IT87_GPIO_LDN = 0x07;
//     const byte IT87_CHIP_VERSION_REGISTER = 0x22;

class LPCIO{

private:
	// list <int> superios;
	 CStringArray   report;
	  // I/O Ports
    
	 USHORT registerPort;
	 USHORT valueport;
	 
	 byte ReadByte(byte m_register);
	 USHORT ReadWord(byte m_register);
	 void ReportUNKnowChip(string type, int chip);
	 void  Select(byte logicalDeviceNumber);

//region Winbond, Fintek

    void WinbondNuvotonFintekEnter();
    void WinbondNuvotonFintekExit(); 
    bool DetectWinbondFintek();

//#endregion
   
    private: void IT87Enter();
			 void IT87Exit();
             bool DetectIT87();
	
	
	private: void SMSCEnter();
			 void SMSCExit() ;
			 bool DetectSMSC();

    void Detect();
public:
	LPCIO();
	ISuperIO *pSuperIO;
	//public ISuperIO[] SuperIO();
	//public: string GetReport();
    
		   

};
#endif