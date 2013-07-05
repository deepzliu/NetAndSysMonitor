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
  Michael M?ller <m.moeller@gmx.ch>.
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
#ifndef _W836XX_H_
#define _W836XX_H_

#include "chip.h "
using namespace Chips;


// Consts 
    static const ushort WINBOND_VENDOR_ID = 0x5CA3;
    static const byte WIN_HIGH_BYTE = 0x80;

    // Hardware Monitor
    static const byte WIN_ADDRESS_REGISTER_OFFSET = 0x05;
    static const byte WIN_DATA_REGISTER_OFFSET = 0x06;

    // Hardware Monitor Registers
    static const byte WIN_VOLTAGE_VBAT_REG = 0x51;
    static const byte WIN_BANK_SELECT_REGISTER = 0x4E;
    static const byte WIN_VENDOR_ID_REGISTER = 0x4F;
    static const byte WIN_TEMPERATURE_SOURCE_SELECT_REG = 0x49;
    static const byte  WIN_TEMPERATURE_REG[3] = { 0x50, 0x50, 0x27};
    static const byte  WIN_TEMPERATURE_BANK[3] ={ 1, 2, 0 };
    static const byte  WIN_FAN_TACHO_REG[5] = { 0x28, 0x29, 0x2A, 0x3F, 0x53 };
	//static const byte  WIN_FAN_TACHO_REG[5] = { 0x28, 0x29, 0x2A, 0x2B, 0x2C }; //liudonghua
    static const byte  WIN_FAN_TACHO_BANK[5] ={ 0, 0, 0, 0, 5 };       
    static const byte  WIN_FAN_BIT_REG[5] ={ 0x47, 0x4B, 0x4C, 0x59, 0x5D };
    static const byte  WIN_FAN_DIV_BIT0[5]={ 36, 38, 30, 8, 10 };
    static const byte  WIN_FAN_DIV_BIT1[5] = { 37, 39, 31, 9, 11 };
    static const byte  WIN_FAN_DIV_BIT2[5] = { 5, 6, 7, 23, 15 };
    static const uint WIN_tempratureregcount = sizeof(WIN_TEMPERATURE_REG)/sizeof(byte);
	static const uint WIN_fanbitregcount = sizeof(WIN_FAN_BIT_REG)/sizeof(byte);
 // W83627EHF voltageRegister
	static const byte  wEHFvoltageRegister[10] ={0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x50, 0x51, 0x52 };
    static const byte  wEHFvoltageBank[10] ={ 0, 0, 0, 0, 0, 0, 0, 5, 5, 5 };
//W83627DHG W83627DHGP W83667HG W83667HGB:   
	static const byte  wDHGvoltageRegister[9] ={0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x50, 0x51 };
    static const byte  wDHGvoltageBank[9] ={ 0, 0, 0, 0, 0, 0, 0, 5, 5};

//W83627HF W83627THF W83687THF:
    static const byte   wTHFvoltageRegister[7] ={0x20, 0x21, 0x22, 0x23, 0x24, 0x50, 0x51 };
    static const byte   wTHFvoltageBank[7] ={0, 0, 0, 0, 0, 5, 5};
#if LG_CN
	const string TEMPERATUREDescription[3] ={"CPU","Auxiliary","system"};
	const string FANDesription[5] = {"SYSTEMFAN:","CPUFAN:","AuxiliaryFAN","CPU FAN#2","AuxiliaryFAN #2"};
#else
    const string TEMPERATUREDescription[3] ={"CPU温度","主板温度","系统温度"};
	const string FANDesription[5] = {"系统风扇:","CPU风扇:","辅助风扇","CPU风扇2","辅助风扇#2"};
#endif
    
	

class W836XX:ISuperIO
{

private:
	readonly ushort address;
	readonly byte revision;
    //readonly Chip chip;
    readonly float *voltages;
	readonly float *temperatures;    
	readonly float *fans;
 


    private:readonly bool *peciTemperature;
    private:readonly byte *voltageRegister;
    private:readonly byte *voltageBank;
    private:readonly float voltageGain;
    uint voltageRegistercount;

    
    private:byte ReadByte(byte bank, byte m_register);

    private:void WriteByte(byte bank, byte register, byte value); 
    byte  ReadGPIO(int index) {
      return 0;
    };

    public: void WriteGPIO(int index, byte value) { ;};
   
    public: W836XX(Chip chip, byte revision, ushort address) {
      this->address = address;
      this->revision = revision;
      this->chip = chip;

      if (!IsWinbondVendor())
        return;
      
      temperatures = new float[3];
      peciTemperature = new bool[3];
      switch ((ushort)chip) {
	    case W83667HG:
        case W83667HGB:
          // note temperature sensor registers that read PECI
			{
			 byte flag = ReadByte(0, WIN_TEMPERATURE_SOURCE_SELECT_REG);
             peciTemperature[0] = (flag & 0x04) != 0;
             peciTemperature[1] = (flag & 0x40) != 0;
             peciTemperature[2] = false;
			}
          break;
        case W83627DHG:        
        case  W83627DHGP:
          // note temperature sensor registers that read PECI
			{
				byte sel = ReadByte(0, WIN_TEMPERATURE_SOURCE_SELECT_REG);
                peciTemperature[0] = (sel & 0x07) != 0;
                peciTemperature[1] = (sel & 0x70) != 0;
                peciTemperature[2] = false;
			}
          break;
        default:
          // no PECI support
          peciTemperature[0] = false;
          peciTemperature[1] = false;
          peciTemperature[2] = false;
          break;
      }

      switch (chip) {
        case  W83627EHF:
          voltages = new float[10];
          voltageRegister = (byte *)wEHFvoltageRegister;
          voltageBank = (byte*)wEHFvoltageBank;
          voltageGain = 0.008f;
		  voltageRegistercount = 10;
          fans = new float[5];
          break;
        case  W83627DHG:
        case  W83627DHGP:        
        case  W83667HG:
        case  W83667HGB:
          voltages = new float[9];
          voltageRegister = (byte*)wDHGvoltageRegister;
          voltageBank = (byte*)wDHGvoltageBank;
		  voltageRegistercount = 9;
          voltageGain = 0.008f;
          fans = new float[5];
          break;
        case  W83627HF:
        case  W83627THF:
        case  W83687THF:
          voltages = new float[7];
          voltageRegister =(byte*)wTHFvoltageRegister;
          voltageBank = (byte*)wTHFvoltageBank;
		  voltageRegistercount = 7;
          voltageGain = 0.016f;
          fans = new float[3];         
          break;
      }
    }   ;

    private:bool IsWinbondVendor() {
      ushort vendorId =
        (ushort)((ReadByte(WIN_HIGH_BYTE, WIN_VENDOR_ID_REGISTER) << 8) |
           ReadByte(0, WIN_VENDOR_ID_REGISTER));
      return vendorId == WINBOND_VENDOR_ID;
    };

    private: ulong SetBit(ulong target, int bit, int value) {
      if ((value & 1) != value)
        throw  string("Value must be one bit only.");

      if (bit < 0 || bit > 63)
        throw string("Bit out of range.");

      ulong mask = (((ulong)1) << bit);
      return value > 0 ? target | mask : target & ~mask;
    };

    //public: Chip Chip { get { return chip; } }
    //public: float?[] Voltages { get { return voltages; } }
    //public: float?[] Temperatures { get { return temperatures; } }
    //public: float?[] Fans { get { return fans; } }

    public: void Update();
   // bool Reset();
   // bool ReadValues(sensorsValue *pSensorValue);
    public: string GetReport() {
      return "";
    };
 }; 

#endif
