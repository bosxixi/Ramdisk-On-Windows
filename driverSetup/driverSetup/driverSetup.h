
// driverSetup.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CdriverSetupApp:
// �йش����ʵ�֣������ driverSetup.cpp
//

class CdriverSetupApp : public CWinApp
{
public:
	CdriverSetupApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CdriverSetupApp theApp;