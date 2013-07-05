/*
  
  Version: MPL 1.1/GPL 2.0/LGPL 2.1

  The contents of this file are subject to the Mozilla public: License Version
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
  either the GNU General public: License Version 2 or later (the "GPL"), or
  the GNU Lesser General public: License Version 2.1 or later (the "LGPL"),
  in which case the provisions of the GPL or the LGPL are applicable instead
  of those above. If you wish to allow use of your version of this file only
  under the terms of either the GPL or the LGPL, and not to allow others to
  use your version of this file under the terms of the MPL, indicate your
  decision by deleting the provisions above and replace them with the notice
  and other provisions required by the GPL or the LGPL. If you do not delete
  the provisions above, a recipient may use your version of this file under
  the terms of any one of the MPL, the GPL or the LGPL.
 
*/
#ifndef  F718XX_H
#define  F718XX_H
#include "chip.h"
   // Hardware Monitor
     const byte F718_ADDRESS_REGISTER_OFFSET = 0x05;
     const byte F718_DATA_REGISTER_OFFSET = 0x06;
    // Hardware Monitor Registers
    const byte F718_VOLTAGE_BASE_REG = 0x20;
    const byte F718_TEMPERATURE_CONFIG_REG = 0x69;
    const byte F718_TEMPERATURE_BASE_REG = 0x70;
    const byte F718_FAN_TACHOMETER_REG[4] ={ 0xA0, 0xB0, 0xC0, 0xD0 };

	const string F718_TemperDescription[3]={"系统温度","CPU温度","北桥温度"};
	const string F718_FanDescription[4]={"CPU风扇","系统风扇2","电源风扇","系统风扇1"};
using namespace  Chips;

class F718XX:ISuperIO{

    private: readonly ushort address;
   // private: readonly Chip chip;

    private: readonly float *voltages;
    private: readonly float *temperatures;
    private: readonly float *fans;
    private: readonly uint  voltageslength;
    private: readonly uint  temperatureslength;
    private: readonly uint  fanscount;
 
    private: byte ReadByte(byte m_register);

    public: byte ReadGPIO(int index) {
      return 0;
    };

    public: void WriteGPIO(int index, byte value) { };

    public: F718XX(Chip chip, ushort address);


    public: string GetReport() {return "null";
    }
	//bool Reset();
    //bool ReadValues(sensorsValue *pSensorValue);
    public: void Update();
 };

#endif