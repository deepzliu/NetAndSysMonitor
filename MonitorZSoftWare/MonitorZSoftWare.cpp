// MonitorZSoftWare.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"

#include "monitor.h"
#include "sendmail.h"
#include <Windows.h>
#include <fstream>
#include <string>
#include <conio.h>
//using namespace std;

char receiver[64];
char sender[64];
char senderpwd[64];
char sendtitle[64];
char serverip[64];
char servername[64];
FILE *flog;

int SendMail(char *content);

//将wchar_t* 转成char*的实现函数如下：
char *w2c(char *pcstr,const wchar_t *pwstr, size_t len){
	int nlength=wcslen(pwstr);

	//获取转换后的长度
	int nbytes = WideCharToMultiByte( 0, // specify the code page used to perform the conversion
					0,         // no special flags to handle unmapped characters
					pwstr,     // wide character string to convert
					nlength,   // the number of wide characters in that string
					NULL,      // no output buffer given, we just want to know how long it needs to be
					0,
					NULL,      // no replacement character given
					NULL );    // we don't want to know if a character didn't make it through the translation

	// make sure the buffer is big enough for this, making it larger if necessary
	//if(nbytes>len)   nbytes=len;
	if(nbytes < len)   nbytes=len; //liudonghua

	// 通过以上得到的结果，转换unicode 字符为ascii 字符
	WideCharToMultiByte( 0, // specify the code page used to perform the conversion
					0,         // no special flags to handle unmapped characters
					pwstr,   // wide character string to convert
					nlength,   // the number of wide characters in that string
					pcstr, // put the output ascii characters at the end of the buffer
					nbytes, // there is at least this much space there
					NULL,      // no replacement character given
					NULL );
	pcstr[nbytes] = '\0';
	return pcstr ;

}

//将char* 转成wchar_t*的实现函数如下：
//这是把asii字符转换为unicode字符，和上面相同的原理

//void c2w(wchar_t *pwstr,size_t len,const char *str)
//{
//	if(str){
//		size_t nu = strlen(str);
//		size_t n =(size_t)multibytetowidechar(cp_acp,0,(const char *)str,(int)nu, NULL,0);
//		if(n>=len) n=len-1;
//		multibytetowidechar(cp_acp,0,(const char *)str,(int)nu,pwstr,(int)n);
//		pwstr[n]=0;
//	}
//}

char *getpwd(char *pwd){
	if(pwd == NULL) return NULL;

	int n = 0;
	char c;
	do{
		c = getch();
		pwd[n] = c;
		n++;
		if(c != 13) printf("*");
	}while(c != 13);
	pwd[n-1] = '\0';

	return pwd;
}

int ReadConfig(){
	char *file = "email.ini";
	ifstream fin(file);
	string line;
	int nSize = 0, ret = 0;
	char str1[64], str2[64], c;

	if(!fin.is_open()){
		MessageBox(NULL, L"配置文件打开失败", L"警告", MB_OK | MB_ICONWARNING);
		return -1;
	}
	
	while(getline(fin, line)){
		nSize = line.size();
		if(nSize < 1) continue;
		if(line[0] == '#'){
			continue;
		}else{
			ret = sscanf(line.c_str(), "%[^=]=%[^\0]", str1, str2);
			if(ret != 2) continue;

			if(strcmp(str1, "receiver") == 0){
				memcpy(receiver, str2, strlen(str2)+1);
			}else if(strcmp(str1, "sender") == 0){
				memcpy(sender, str2, strlen(str2)+1);
			}else if(strcmp(str1, "title") == 0){
				memcpy(sendtitle, str2, strlen(str2)+1);
			}else if(strcmp(str1, "serverip") == 0){
				memcpy(serverip, str2, strlen(str2)+1);
			}else if(strcmp(str1, "servername") == 0){
				memcpy(servername, str2, strlen(str2)+1);
			}else{
				continue;
			}
		}
	}

	printf("Please input password of e-mail [%s]: ", sender);
	//scanf("%s", senderpwd);
	getpwd(senderpwd);
	
	printf("\nWait...\r");
	if(SendMail("E-mail Test") == 0){
		printf("E-mail test success\n");
		flog = fopen("log.txt", "a");
		if(flog == NULL){
			printf("Log open failed\n");
		}else{
			printf("Log open success\n");
		}
		printf("Press 'y' to continue, 'n' to end: ");
		fflush(stdin);
		scanf("%c", &c);
		if(c == 'y'){
			return 0;
		}else{
			return -1;
		}
	}else{
		printf("E-mail test failed\r");
		printf("Some E-mail configuration information incorrect\n");
		return -1;
	}

	

	return 0;
}

int SendMail(char *content){
	sMailInfo smailInfo;
	memset(&smailInfo, 0, sizeof(sMailInfo));

	smailInfo.m_pcUserName = sender;
	smailInfo.m_pcUserPassWord = senderpwd;
	smailInfo.m_pcSenderName = sender;
	smailInfo.m_pcSender = sender;
	smailInfo.m_pcReceiver = receiver;
	smailInfo.m_pcTitle = sendtitle;
	smailInfo.m_pcBody = content;
	smailInfo.m_pcIPAddr = serverip;
	smailInfo.m_pcIPName = servername;

	CSendMail mail;

	if(mail.SendMail(smailInfo)){
		//printf("Send mail success\n");
		return 0;
	}else{
		//最好写个日志
		if(flog == NULL) flog = fopen("log.txt", "a");
		if(flog){
			fprintf(flog, "[Send mail failed]\n%s", content);
			fflush(flog);
		}
		return 1;
	}

	return 0;
}

//
// PID:
//		1.PID == 0, 进程不存在，需要报出来，或服务停止
//		2.PID > 0, 进程存在，状态提起或有异常窗体
//		3.PID < 0, 其它
// ProcessName: 为进程名或服务名
// msg:
//		1.process not found
//		2.process hung
//		3.abnormal dialog appeared
//		4.service stopped
// msgLen: msg的长度
// lparam：异常窗口时的窗口标题
//

wchar_t wBuf[4096];
char cBuf[4096];
char *title = "Monitor ZSoftWare";
void CallbackFunc(unsigned int PID, wchar_t *ProcessName, wchar_t *msg, int msgLen, void *lparam)
{
	static int count = 0;
	wchar_t *str;
	if(PID == -1){
		//count = 0;
		//printf("\n\n");
	}else{
		//count ++;
		if(lparam){
			str = (wchar_t *)lparam;
			swprintf(wBuf, L"进程ID[%x] 进程名[%ws] %ws - %ws", PID, ProcessName, msg, str);			
		}else{
			swprintf(wBuf, L"进程ID[%x] 进程名[%ws] %ws", PID, ProcessName, msg);
		}
		//wprintf(L"%ws\n", wBuf);
		w2c(cBuf, wBuf, wcslen(wBuf));
		SendMail(cBuf);
	}
}


int main(int argc, wchar_t* argv[])
{
	wchar_t configfile[64] = L"config.ini";
	if(ReadConfig() == -1){
		return 1;
	}

	//隐藏程序窗口
	HWND   hWnd;
	hWnd=GetConsoleWindow();
	ShowWindow(hWnd,SW_HIDE);

	StartMonitor(CallbackFunc, 5, NULL);
	return 0;
}

