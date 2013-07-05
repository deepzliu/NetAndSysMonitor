#pragma once

//#include "stdafx.h"
#include <stdio.h>

#define PROC_NOT_FOUND L"process not found"
#define PROC_HUNG L"process hung"
#define AB_DLG L"abnormal dialog appeared"
#define SERV_STOPPED L"service stopped"

typedef void (*msgcallback)(unsigned int PID, wchar_t *ProcessName, wchar_t *msg, int msgLen, void *lparam);

int StartMonitor(msgcallback msgfunc, int pTimer, wchar_t *confFile = NULL);