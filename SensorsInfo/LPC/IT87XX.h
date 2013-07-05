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
  Michael M枚ller <m.moeller@gmx.ch>.
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
#ifndef _IT87XX_H_
#define _IT87XX_H_
#include "chip.h"
  // Consts
    const byte ITE_VENDOR_ID = 0x90;
       
    // Environment Controller
    const byte ITE_ADDRESS_REGISTER_OFFSET = 0x05;
    const byte ITE_DATA_REGISTER_OFFSET = 0x06;

    // Environment Controller Registers    
    const byte ITE_CONFIGURATION_REGISTER = 0x00;
    const byte ITE_TEMPERATURE_BASE_REG = 0x29;
    const byte ITE_VENDOR_ID_REGISTER = 0x58;
    const byte ITE_FAN_TACHOMETER_16_BIT_ENABLE_REGISTER = 0x0c;
    const byte ITE_FAN_TACHOMETER_REG[5] = { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
    const byte ITE_FAN_TACHOMETER_EXT_REG[5] ={ 0x18, 0x19, 0x1a, 0x81, 0x83 };
    const byte ITE_VOLTAGE_BASE_REG = 0x20;
//#ifdef LG_CN
	const string ITE_TemperDescription[3]={"CPU温度","系统温度","北桥温度"};
	const string ITE_FanDescription[4]={"CPU风扇","系统风扇2","电源风扇","系统风扇1"};
//else
//    const string ITE_TemperDescription[3]={"system","CPU","northbridge"};
//	const string ITE_FanDescription[4]={"CPU fan","systemfan#2","powerfan","systemfan#1"};
//#endif
typedef char sbyte;
using namespace Chips;
class IT87XX:ISuperIO
{
       
	USHORT address;
	//Chip chip;
    byte version;
    USHORT gpioAddress;
    int gpioCount;
    USHORT addressReg;
    USHORT dataReg;
    float voltages[9];
    float temperatures[5];
    float fans[3];
    float voltageGain;
    byte ReadByte(byte m_register,  bool &valid);

public:
	 byte ReadGPIO(int index);
     void WriteGPIO(int index, byte value);
     IT87XX(Chip chip, USHORT address, USHORT gpioAddress, byte version);
     string GetReport(){return string("");}
     void Update(void);
	 //bool Reset();
    // bool ReadValues(sensorsValue *pSensorValue);
};
#endif