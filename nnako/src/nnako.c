#include <windows.h>

/*
cl nnako.c /link /subsystem:windows user32.lib shell32.lib
*/

typedef DWORD  (__stdcall *nako_load_t)(char *);
typedef void   (__stdcall *nako_free_t)(void);
typedef void   (__stdcall *nako_setDNAKO_DLL_handle_t)(DWORD);
typedef DWORD  (__stdcall *nako_run_t)(void);
typedef DWORD  (__stdcall *nako_getError_t)(char *,int);
typedef void   (__stdcall *nako_clearError_t)(void);
typedef void   (__stdcall *nako_addFileCommand_t)(void);
typedef void   (__stdcall *nako_LoadPlugins_t)(void);
typedef void   (__stdcall *nako_setPluginsDir_t)(char *);
typedef DWORD  (__stdcall *nako_loadSource_t)(char *);
typedef void   (__stdcall *nako_makeReport_t)(char *);

HMODULE hDNako=NULL;
nako_load_t nako_load=NULL;
nako_free_t nako_free=NULL;
nako_setDNAKO_DLL_handle_t nako_setDNAKO_DLL_handle=NULL;
nako_run_t nako_run=NULL;
nako_getError_t nako_getError=NULL;
nako_clearError_t nako_clearError=NULL;
nako_addFileCommand_t nako_addFileCommand=NULL;
nako_LoadPlugins_t nako_LoadPlugins=NULL;
nako_setPluginsDir_t nako_setPluginsDir=NULL;
nako_loadSource_t nako_loadSource=NULL;
nako_makeReport_t nako_makeReport=NULL;
/* 実行ファイルのあるフォルダのパス。最後に「\」は付かない */
char szBaseDirectory[1024];
char szPluginDirectory[1024];
char szDNakoFilepath[1024];
char szBokanDirectory[1024];
char *pSource;
int runMode;
int debugMode;

void init(){
	pSource=NULL;
	runMode=2;
	debugMode=0;
	szBaseDirectory[0]=0;
	szPluginDirectory[0]=0;
	szDNakoFilepath[0]=0;
	szBokanDirectory[0]=0;
}

void finish(){
	if (pSource!=NULL){
		free(pSource);
		pSource=NULL;
	}
}


int init_bokanDirectory(){
	char *p,*pb;
	DWORD dwLen;
	char szFilename[1024];
	return 0;
}

int init_DirectoryPath(){
	char *p,*pb;
	DWORD dwLen;
	char szFilename[1024];

	/* 実行ファイルのフルパスを取得する */
	dwLen=GetModuleFileName(NULL,szFilename,sizeof(szFilename));
	if (dwLen==sizeof(szFilename)-1){
		return -1;
	}
	/* 最後のディレクトリ後の'\'を見つける */
	for (p=szFilename,pb=NULL;*p;p++){
		if (*p=='\\'){
			pb=p;
		}else
		if (IsDBCSLeadByteEx(932,(BYTE)*p)&&*(p+1)!=0){
			p++;
		}
	}
	if (pb==NULL){
		pb=p;
	}
	*pb=0;
	lstrcpy(szBaseDirectory,szFilename);
	lstrcpy(szPluginDirectory,szBaseDirectory);
	lstrcat(szPluginDirectory,"\\plug-ins");
	lstrcpy(szDNakoFilepath,szBaseDirectory);
	lstrcat(szDNakoFilepath,"\\plug-ins\\dnako.dll");
	return 0;
}

int proc_args(){
	char *lpWork;
	int iWorkLen;
	int noopt;
	wchar_t *lpwCmdLine;
	wchar_t **lpwCmdArgv;
	int iCmdArgs;
	int optionError;
	int optend;
	int i;
	int iLen;

	lpWork=NULL;
	lpwCmdArgv=NULL;
	iWorkLen=0;
	optend=0;
	optionError=0;

	lpwCmdLine=GetCommandLineW();
	lpwCmdArgv=CommandLineToArgvW(lpwCmdLine,&iCmdArgs);
	if (lpwCmdArgv==NULL){
		return 1;
	}
	for (i=1;i<iCmdArgs;i++){
		iLen=WideCharToMultiByte(932,0,lpwCmdArgv[i],-1,NULL,0,NULL,NULL);
		if (iLen >= iWorkLen){
			if (lpWork!=NULL){
				free(lpWork);
				lpWork=NULL;
			}
			iWorkLen=((iLen+1)/16+1)*16;
			lpWork=(char *)malloc(iWorkLen);
		}
		iLen=WideCharToMultiByte(932,0,lpwCmdArgv[i],-1,lpWork,iWorkLen,NULL,NULL);
		lpWork[iLen]=0;
		if (lstrcmp(lpWork,"-e")==0||lstrcmp(lpWork,"-eval")==0){
			runMode=1;
		}else
		if (lstrcmp(lpWork,"-debug")==0){
			debugMode=1;
		}else
		if (lstrcmp(lpWork,"--")==0){
			optend=1;
		}else
		if (lpWork[0]=='-'&&optend==0){
			optionError=1;
			break;
		}else{
			if (pSource!=NULL){
				free(pSource);
				pSource=NULL;
			}
			iLen=lstrlen(lpWork);
			pSource=(char *)malloc(iLen+1);
			lstrcpy(pSource,lpWork);
		}
	}

	if (lpWork!=NULL){
		free(lpWork);
		lpWork=NULL;
	}
	if (optionError!=0){
		return 3;
	}
	if (pSource==NULL||*pSource==0){
		return 2;
	}
	return 0;
}

int init_dnako(){
	hDNako=LoadLibrary(szDNakoFilepath);
	if (hDNako==NULL){
		return -2;
	}
	nako_load=(nako_load_t)GetProcAddress(hDNako,"nako_load");
	nako_run=(nako_run_t)GetProcAddress(hDNako,"nako_run");
	nako_free=(nako_free_t)GetProcAddress(hDNako,"nako_free");
	nako_setDNAKO_DLL_handle=(nako_setDNAKO_DLL_handle_t)GetProcAddress(hDNako,"nako_setDNAKO_DLL_handle");
	nako_getError=(nako_getError_t)GetProcAddress(hDNako,"nako_getError");
	nako_clearError=(nako_clearError_t)GetProcAddress(hDNako,"nako_clearError");
	nako_addFileCommand=(nako_addFileCommand_t)GetProcAddress(hDNako,"nako_addFileCommand");
	nako_LoadPlugins=(nako_LoadPlugins_t)GetProcAddress(hDNako,"nako_LoadPlugins");
	nako_setPluginsDir=(nako_setPluginsDir_t)GetProcAddress(hDNako,"nako_setPluginsDir");
	nako_loadSource=(nako_loadSource_t)GetProcAddress(hDNako,"nako_loadSource");
	nako_makeReport=(nako_makeReport_t)GetProcAddress(hDNako,"nako_makeReport");
	if (nako_load==NULL
	  ||nako_run==NULL
	  ||nako_free==NULL
	  ||nako_setDNAKO_DLL_handle==NULL
	  ||nako_getError==NULL
	  ||nako_clearError==NULL
	  ||nako_addFileCommand==NULL
	  ||nako_LoadPlugins==NULL
	  ||nako_setPluginsDir==NULL
	  ||nako_loadSource==NULL
	  ||nako_makeReport==NULL
	   ){
		FreeLibrary(hDNako);
		hDNako=NULL;
		return -3;
	}
	nako_setDNAKO_DLL_handle((DWORD)hDNako);
	return 0;
}

void finish_dnako(){
	if (hDNako!=NULL){
		if (nako_free!=NULL){
			(*nako_free)();
		}
		FreeLibrary(hDNako);
		hDNako=NULL;
	}
}

void showErrorDialog(char *lpMsg,char *lpTitle){
	MessageBox(NULL,
	           lpMsg,
	           lpTitle,
	           MB_OK|MB_ICONERROR);
}

int WINAPI WinMain(
  HINSTANCE hInstance,      // 現在のインスタンスのハンドル
  HINSTANCE hPrevInstance,  // 以前のインスタンスのハンドル
  LPSTR lpCmdLine,          // コマンドライン
  int nCmdShow              // 表示状態
)
{
	DWORD dwLen;
	char *lpMsg;
	int rtn;
	char szWork[1024];

	init();

	if (init_DirectoryPath()!=0){
		showErrorDialog("実行ファイルの置かれているファイルパスが長すぎます","初期化エラー");
		rtn=1;
		goto errorexit;
	}

	rtn=init_dnako();
	if (rtn!=0){
		showErrorDialog("初期化エラー(dnako初期化エラー)","初期化エラー");
		rtn=1;
		goto errorexit;
	}

	rtn=proc_args();
	if (rtn==1){
		showErrorDialog("引数の解析エラー","起動オプションエラー");
		rtn=1;
		goto errorexit;
	}else
	if (rtn==2){
		showErrorDialog("実行ファイルもしくは実行スクリプトが指定されていません","起動オプションエラー");
		rtn=1;
		goto errorexit;
	}else
	if (rtn!=0){
		showErrorDialog("無効なオプション引数があります","起動オプションエラー");
		rtn=1;
		goto errorexit;
	}

	nako_addFileCommand();
	nako_setPluginsDir(szPluginDirectory);
	nako_LoadPlugins();

	if (runMode==1){
		rtn = nako_loadSource(pSource);
	}else
	if (runMode==2){
		rtn = nako_load(pSource);
	}
	if (rtn==0){
		dwLen=nako_getError(NULL,0);
		if (dwLen > 0){
			lpMsg=(char *)malloc(dwLen+1);
			nako_getError(lpMsg,dwLen+1);
		}else{
			lpMsg="読込み時の不明なエラー";
		}
		showErrorDialog(lpMsg,"読込みエラー");
		if (dwLen > 0){
			free(lpMsg);
		}
		rtn=1;
		goto errorexit;
	}
	rtn = nako_run();
	if (rtn==0){
		dwLen=nako_getError(NULL,0);
		if (dwLen > 0){
			lpMsg=(char *)malloc(dwLen+1);
			nako_getError(lpMsg,dwLen+1);
		}else{
			lpMsg="実行時の不明なエラー";
		}
		showErrorDialog(lpMsg,"実行エラー");
		if (dwLen > 0){
			free(lpMsg);
		}
		rtn=1;
		goto errorexit;
	}

	rtn=0;
errorexit:
	finish_dnako();
	finish();
	return rtn;
}