
// USB_Viewer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CUSB_ViewerApp: 
// �йش����ʵ�֣������ USB_Viewer.cpp
//

class CUSB_ViewerApp : public CWinApp
{
public:
	CUSB_ViewerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CUSB_ViewerApp theApp;