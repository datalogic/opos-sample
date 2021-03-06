#include "stdafx.h"
#include "scannersink.h"

DWORD tid;
BOOL handler(DWORD event);

int main() {

	// setup the console program to exit gracefully 
	tid = GetCurrentThreadId();
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)(handler), TRUE);

	// initialize COM objects on this thread
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// create a COM object and point to it
	OposScanner_CCO::IOPOSScannerPtr scanner;
	scanner.CreateInstance("OPOS.Scanner");

	// the name of a scanner profile defined under
	// HKLM\Software\Wow6432Node\OLEforRetail\ServiceOPOS\SCANNER
	std::string name("HandScanner");

	// attempt to open, claim, and enable a scanner
	// these functions are defined by OPOS, not COM
	// notice that scanner is being dereferenced here
	scanner->Open(name.c_str());
	scanner->ClaimDevice(1000);
	if (scanner->Claimed) {
		std::cout << "Connected to " << name << std::endl;
		scanner->DeviceEnabled = true;
		scanner->DataEventEnabled = true;
		scanner->DecodeData = true;
	}
	else {
		scanner->Close();
		scanner.Release();
		CoUninitialize();

		return 1;
	}

	// query whether scanner is connectable
	IConnectionPointContainer *cpc;
	scanner->QueryInterface(IID_IConnectionPointContainer, (void **) &cpc);

	// query whether _IOPOSScannerEvents connection point is supported
	IConnectionPoint *cp; 
	cpc->FindConnectionPoint(
		__uuidof(OposScanner_CCO::_IOPOSScannerEvents), &cp);
	cpc->Release();

	// initialize and subscribe the sink
	DWORD cookie;
	ScannerSink *sink = new ScannerSink(*scanner);
	cp->Advise(sink, &cookie);

	// message loop for scanner
	std::cout << "Press \'Ctrl + C\' to quit." << std::endl;
	static MSG msg = { 0 };
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	// disable operation, release control, and close the scanner
	scanner->DeviceEnabled = false;
	scanner->ReleaseDevice();
	scanner->Close();
	
	// unsubscribe the sink
	cp->Unadvise(cookie);
	cp->Release();
	
	// release the COM object and shutdown COM
	scanner.Release();
	CoUninitialize();

	return 0;
}

BOOL handler(DWORD event) {
	
	PostThreadMessage(tid, WM_QUIT, 0, 0);
	return TRUE;
}
