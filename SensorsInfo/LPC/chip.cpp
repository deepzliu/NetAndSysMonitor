#include "StdAfx.h"
#include "..\StdAfx.h"
#include "chip.h"
using namespace Chips;

string ChipName::GetName(Chip chip)
{
      switch (chip) 
	  {
	    case Chips::F71858: return string("Fintek F71858");
        case Chips::F71862: return string("Fintek F71862");
        case Chips::F71869: return string("Fintek F71869");
        case Chips::F71882: return string("Fintek F71882");
        case Chips::F71889ED: return string("Fintek F71889ED");
        case Chips::F71889F: return string("Fintek F71889F");
        case Chips::IT8712F: return string("ITE IT8712F");
        case Chips::IT8716F: return string("ITE IT8716F");
        case Chips::IT8718F: return string("ITE IT8718F");
        case Chips::IT8721F: return string("ITE IT8721F");
        case Chips::IT8720F: return string("ITE IT8720F");
        case Chips::IT8726F: return string("ITE IT8726F");
		case Chips::IT8728F: return string("ITE IT8728F");
        case Chips::W83627DHG: return string("Winbond W83627DHG");
        case Chips::W83627DHGP: return string("Winbond W83627DHG-P");
        case Chips::W83627EHF: return string("Winbond W83627EHF");
        case Chips::W83627HF: return string("Winbond W83627HF");
        case Chips::W83627THF: return string("Winbond W83627THF");
        case Chips::W83667HG: return string("Winbond W83667HG");
        case Chips::W83667HGB: return string("Winbond W83667HG-B");
        case Chips::W83687THF: return string("Winbond W83687THF");
        case Chips::Unknown: return string("Unkown");
        default: return string("Unknown");
      }
	  return string("Unknow");
}