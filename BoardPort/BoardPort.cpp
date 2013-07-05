// BoardPort.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <conio.h>

int TempSensor(){
	char value;

	while(1){
		value = _inp(0x60);
	//__asm{
	//	//mov ebx,cr0
	//	//mov [EAX],ebx
	//	//retf
	//	//in al, 60h
	//	_emit(0x66, 0x60);
	//	mov value,ax
	//}
		printf("%d [%c]\n", value, value);
		Sleep(1000);
	}
	return 0;
}
int FanSensor(){
	return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{
	TempSensor();
	return 0;
}

