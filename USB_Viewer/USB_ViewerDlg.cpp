
// USB_ViewerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "afxdialogex.h"

#include "setupapi.h"
//#include "hidsdi.h"		//古月guyue
#include "USB_Viewer.h"
#include "USB_ViewerDlg.h"
#include "CreateStartDlg.h"	//guyue
#include "PartitionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CUSB_ViewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CUSB_ViewerDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CUSB_ViewerDlg::OnCbnSelchangeCombo1)
	ON_WM_DEVICECHANGE()		//用于接收设备(U盘)变动, 已更新界面的消息
	ON_COMMAND(ID_CeateStart, &CUSB_ViewerDlg::OnCeatestart)
	ON_COMMAND(ID_EXIT, &CUSB_ViewerDlg::OnExit)
	ON_NOTIFY(NM_CLICK, IDC_LIST2, &CUSB_ViewerDlg::OnClickList2)
	ON_COMMAND(ID_ABOUT, &CUSB_ViewerDlg::OnAbout)
	ON_BN_CLICKED(IDCANCEL, &CUSB_ViewerDlg::OnBnClickedCancel)
	ON_COMMAND(ID_Partition, &CUSB_ViewerDlg::OnPartition)
END_MESSAGE_MAP()
// CUSB_ViewerDlg 对话框



CUSB_ViewerDlg::CUSB_ViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUSB_ViewerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUSB_ViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}













// SetupDiGetInterfaceDeviceDetail所需要的输出长度，定义足够大
#define INTERFACE_DETAIL_SIZE    (1024)
//DEFINE_GUID(UsbClassGuid, 0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed);
// 根据GUID获得设备路径
// lpGuid: GUID指针
// pszDevicePath: 设备路径指针的指针
// 返回: 成功得到的设备路径个数，可能不止1个
int GetDevicePath(LPGUID lpGuid, LPTSTR* pszDevicePath)
{
	
	HDEVINFO hDevInfoSet;    //设备信息集句柄；
	SP_DEVICE_INTERFACE_DATA ifdata;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;
	int nCount;
	BOOL bResult;
	
	GUID UsbClassGuid = 
	{
		/*//disk  */0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b
		///*//usb */ 0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed
	};


	// 取得一个该GUID相关的设备信息集句柄
	hDevInfoSet = ::SetupDiGetClassDevs((const GUID*)&UsbClassGuid,     // class GUID
		NULL,                    // 无关键字
		NULL,                    // 不指定父窗口句柄
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);    // 目前存在的设备

												   // 失败...
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// 申请设备接口数据空间
	pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, INTERFACE_DETAIL_SIZE);

	pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	nCount = 0;
	bResult = TRUE;

	// 设备序号=0,1,2... 逐一测试设备接口，到失败为止
	while (bResult)
	{
		ifdata.cbSize = sizeof(ifdata);

		// 枚举符合该GUID的设备接口
		bResult = ::SetupDiEnumDeviceInterfaces(
			hDevInfoSet,     // 设备信息集句柄
			NULL,            // 不需额外的设备描述
			lpGuid,          // GUID
			(ULONG)nCount,   // 设备信息集里的设备序号
			&ifdata);        // 设备接口信息

		if (bResult)
		{
			// 取得该设备接口的细节(设备路径)
			bResult = SetupDiGetInterfaceDeviceDetail(
				hDevInfoSet,    // 设备信息集句柄
				&ifdata,        // 设备接口信息
				pDetail,        // 设备接口细节(设备路径)
				INTERFACE_DETAIL_SIZE,   // 输出缓冲区大小
				NULL,           // 不需计算输出缓冲区大小(直接用设定值)
				NULL);          // 不需额外的设备描述
			if (bResult)
			{
				// 复制设备路径到输出缓冲区
				CString strShow;
				strShow.Format("%08x-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n %s",
					lpGuid->Data1, lpGuid->Data2, lpGuid->Data3, lpGuid->Data4[0],
					lpGuid->Data4[1], lpGuid->Data4[2], lpGuid->Data4[3], lpGuid->Data4[4],
					lpGuid->Data4[5], lpGuid->Data4[6], lpGuid->Data4[7], pDetail->DevicePath);
				//printf("%s\n", strShow);
				//AfxMessageBox(strShow);

				//::strcpy(pszDevicePath[nCount], pDetail->DevicePath);
				// 调整计数值
				nCount++;



				//////////////////////////////////////////////////////////////////////////
				SP_DEVINFO_DATA DeviceInfoData;
				DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
				nCount--;
				SetupDiEnumDeviceInfo(hDevInfoSet, nCount,&DeviceInfoData);
				TCHAR DeviceInstanceId[100] = {0};
				DWORD nSize = 0;
				int isOK = SetupDiGetDeviceInstanceId(
					hDevInfoSet,    // 设备信息集句柄
					&DeviceInfoData,        // 设备接口信息);
					DeviceInstanceId,
					100,
					&nSize
					);
				nCount++;
				strShow.Append("\n开始 ", 4);
				strShow.Append(DeviceInstanceId,100);
				if (AfxMessageBox(strShow, 1, 0) == IDOK)
				{
					SP_PROPCHANGE_PARAMS spPropChangeParams;
					spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
					spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
					spPropChangeParams.Scope = DICS_FLAG_GLOBAL;
					spPropChangeParams.HwProfile = 0; // current hardware profile
					spPropChangeParams.StateChange = DICS_PROPCHANGE;
					/*	1 给SP_PROPCHANGE_PARAMS结构体赋上正确的值
						2 把上面赋完值的SP_PROPCHANGE_PARAMS作为参数传入到SetupDiSetClassInstallParams（）
						3 调用SetupDiCallClassInstaller()，传递参数DIF_PROPEFRTYCHANGE
						实际上，DIF也是按位做与运算后兼容的，你也可以去传递不同的DIF参数来调用SetupDiSetClassInstallParams()。 更多信息，请参考MSDN”Handling DIF Codes”*/
						if (!SetupDiSetClassInstallParams(hDevInfoSet, &DeviceInfoData,
							// note we pass spPropChangeParams as SP_CLASSINSTALL_HEADER
							// but set the size as sizeof(SP_PROPCHANGE_PARAMS)
							(SP_CLASSINSTALL_HEADER*)&spPropChangeParams, sizeof(SP_PROPCHANGE_PARAMS)))
						{
							AfxMessageBox("error1");// handle error
						}
						else if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfoSet, &DeviceInfoData))
						{
							AfxMessageBox("error2");// handle error
						}
						else
						{
							AfxMessageBox("成功ok");
							// ok, show disable success dialog
							// note, after that, the OS will post DBT_DEVICEREMOVECOMPLETE for the disabled device
						}
				}
				
			}
		}
	}
	// 释放设备接口数据空间
	::GlobalFree(pDetail);
	// 关闭设备信息集句柄
	::SetupDiDestroyDeviceInfoList(hDevInfoSet);
	return nCount;
}




// CUSB_ViewerDlg 消息处理程序
BOOL CUSB_ViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//////////////////////////////////////////////////////////////////////////

	CListCtrl *m_wndPath = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	m_wndPath->SetExtendedStyle(m_wndPath->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );	//LVS_EX_CHECKBOXES
	m_wndPath->InsertColumn(0, _T("U盘中的分区大小"));//添加列
	m_wndPath->InsertColumn(1, _T("文件系统"));
	m_wndPath->SetColumnWidth(0, 150);//设置列宽
	m_wndPath->SetColumnWidth(1, 163);

	m_wndPath->SetRedraw(TRUE);//显示

	//控件内容初始化
	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	this->GetUSB(this, IDC_COMBO1);		// 获取当前所有U盘设备,并显示到组合框中
	this->OnCbnSelchangeCombo1();		//更新列表框中 分区表 的显示

	//m_wndPath->SetRedraw(FALSE);//防止重绘

/*
	int nIndex;
	//char|TCHAR项目属性->字符集：使用多字节字符集
	TCHAR Path[MAX_PATH + 1];//TCHAR取代char  MAX_PATH最长路径
	nIndex = m_wndPath->InsertItem(0, _T("Windows目录"));
	if (nIndex < 0) return TRUE;
	GetWindowsDirectory(Path, MAX_PATH);//取得windows目录
	m_wndPath->SetItemText(nIndex, 1, Path);

	LPITEMIDLIST pidl;//桌面CSIDL_DESKTOPDIRECTORY
					  //用来得到系统的某些特定文件夹的位置信息
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidl)))
	{
		if (SHGetPathFromIDList(pidl, Path))//功能是把项目标志符列表转换为文档系统路径
		{
			nIndex = m_wndPath->InsertItem(0, _T("桌面"));//成功则返回0
			if (nIndex < 0)
			{
				return TRUE;
			}
			m_wndPath->SetItemText(nIndex, 1, Path);
		}
	}*/

	//m_wndPath->SetRedraw(TRUE);//显示
	
	
	//////////////////////////////////////////////////////////////////////////






	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_NORMAL);
	
	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUSB_ViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUSB_ViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CUSB_ViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <tchar.h>
#include <winioctl.h>
void CUSB_ViewerDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str;
	CListCtrl *m_ListCtrl = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	CComboBox *m_CComboBox = ((CComboBox*)GetDlgItem(IDC_COMBO1));

	m_CComboBox->GetWindowText(str);
	INT pos = str.Find(":\\");	//因为U盘的显示格式是"G:\ 16G"
	if (pos == -1)return;		//针对	无用的空白选项
	str = str.Mid(pos - 1, 2);		//现在str表示"G:"
	

	//获取要切换显示的分区序号, ps: m_ListCtrl->GetFirstSelectedItemPosition();  若选中则返回 >0, 否则 0
	INT num = (INT)m_ListCtrl->GetFirstSelectedItemPosition();
	if (0 == num)
	{
		AfxMessageBox(TEXT("亲, 请选择要让系统显示的U盘分区哈^_^"));
		return;
	}

	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	CCreateStartDlg m_CreateStartDlg;
	if (m_CreateStartDlg.RemountDrive(num, FALSE, TEXT(str.GetBuffer())))
	{
		m_ListCtrl->DeleteAllItems();	//清楚所有列表项
		AfxMessageBox(TEXT("奥, 恭喜啊, 操作成功了 (0_0)\n  快看看U盘啦~"));
	}
	else
	{
		AfxMessageBox(TEXT("出大事了, 操作失败啊~啊~啊~~ T_T"));
	}
	((CButton*)GetDlgItem(IDOK))->EnableWindow(TRUE);
	this->GetUSB(this, IDC_COMBO1);		// 获取当前所有U盘设备,并显示到组合框中
	this->OnCbnSelchangeCombo1();		//每当选择的U盘改变时候, 自动更新 U盘的 分区列表显示


/*
	CString str;
	CListCtrl *m_ListCtrl = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	CComboBox *m_CComboBox = ((CComboBox*)GetDlgItem(IDC_COMBO1));
	m_ListCtrl->DeleteAllItems();	//清楚所有列表项

	m_CComboBox->GetWindowText(str);
	INT pos = str.Find("\\");	//因为U盘的显示格式是"G:\ 16G"
	str = str.Mid(0, pos);		//现在str表示"G:"

	str = "\\\\.\\" + str;
	HANDLE hDrv = CreateFile(str, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hDrv)
		goto ERROR1;
	
	VOLUME_DISK_EXTENTS vde;
	DWORD dwBytes = 0;
	BOOL isOK = DeviceIoControl(hDrv, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &vde, sizeof(VOLUME_DISK_EXTENTS), &dwBytes, NULL);
	if (!isOK)
		goto ERROR2;

	str.Format("\\\\.\\PhysicalDrive%d", INT(vde.Extents->DiskNumber));
	CloseHandle(hDrv);
	hDrv = CreateFile(str, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hDrv)
		goto ERROR1;

	DISK_GEOMETRY diskGeometry;
	//
	// 获得磁盘结构信息
	//
	if (DeviceIoControl(hDrv,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,    // 调用了CTL_CODE macro宏
		NULL,
		0,
		&diskGeometry,
		sizeof(DISK_GEOMETRY),
		&dwBytes,
		NULL))
	{

		DWORD dwSize = diskGeometry.BytesPerSector;    // 每sector扇区字节数
		UCHAR *sector = new UCHAR[dwSize]{ 0 };
		if (sector)
		{
			ReadFile(hDrv, sector, dwSize, &dwBytes, NULL);
			if (dwBytes == dwSize)
			{
				UCHAR DPT1[16] = { 0 };
				UCHAR DPT2[16] = { 0 };
				UCHAR DPT3[16] = { 0 };
				UCHAR DPT4[16] = { 0 };
				memcpy_s(DPT1, 16, &sector[dwBytes - 66], 16);
				memcpy_s(DPT2, 16, &sector[dwBytes - 50], 16);
				memcpy_s(DPT3, 16, &sector[dwBytes - 34], 16);
				memcpy_s(DPT4, 16, &sector[dwBytes - 18], 16);

				
				
				memcpy_s(&sector[dwBytes - 66], 16, DPT2, 16);
				memcpy_s(&sector[dwBytes - 50], 16, DPT1, 16);
				memcpy_s(&sector[dwBytes - 34], 16, DPT3, 16);
				memcpy_s(&sector[dwBytes - 18], 16, DPT4, 16);

				dwBytes = 0;
				SetFilePointer(hDrv, 0, NULL, FILE_BEGIN);

				WriteFile(hDrv, sector, dwSize, &dwBytes, NULL);
				if (dwBytes == dwSize)
				{
					AfxMessageBox(_T(" U盘分区调整成功!"));
					delete[] sector;
					CloseHandle(hDrv);
				}
				else
					goto ERROR2;
				
			}
			else
				goto ERROR2;
		}
		else
		{
			delete[] sector;
			goto ERROR2;
		}
			



	}
	else
		goto ERROR2;

	//CDialogEx::OnOK();
	return;

ERROR2:
	CloseHandle(hDrv);
ERROR1:
	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	m_CComboBox->GetWindowText(str);
	str.Format("读取(%s)U盘错误", str);
	m_ListCtrl->InsertItem(0, _T(str));
	return;*/
}


// 获取当前所有U盘设备,并显示到组合框中
INT CUSB_ViewerDlg::GetUSB(CDialogEx* dialog, INT ID)
{
	CString strTemp;
	((CComboBox*)dialog->GetDlgItem(ID))->ResetContent();//消除现有所有内容
	
	TCHAR Drive[MAX_PATH] = { 0 };
	DWORD len = ::GetLogicalDriveStrings(sizeof(Drive) / sizeof(TCHAR) , Drive);

	INT i = 0, num = 0;
	BOOL IsFind = FALSE;
	while (strlen(&Drive[i]) > 0)
	{
		if (::GetDriveType(&Drive[i]) == DRIVE_REMOVABLE)
		{
			//如果是可移动设备, 如U盘 , 就显示到界面, 格式是"G:\ 16G"

			unsigned __int64 i64FreeBytesToCaller = 0;
			unsigned __int64 i64TotalBytes = 0;
			unsigned __int64 i64FreeBytes = 0;

			CCreateStartDlg m_CreateStartDlg;
			LARGE_INTEGER usb_allSize = m_CreateStartDlg.GetUSBAllSize((LPCSTR)(&Drive[i]));
/*
			GetDiskFreeSpaceEx(&Drive[i],
				(PULARGE_INTEGER)&i64FreeBytesToCaller,
				(PULARGE_INTEGER)&i64TotalBytes,
				(PULARGE_INTEGER)&i64FreeBytes);*/

			if (usb_allSize.QuadPart / (1024.0 * 1024.0) <= 1000)
			{
				//容量少于1000M 就用mb做显示单位
				strTemp.Format("%s  %.0fM", &Drive[i], usb_allSize.QuadPart / (1024.0 * 1024.0));
				((CComboBox*)dialog->GetDlgItem(ID))->AddString(strTemp);
			}
			else
			{
				strTemp.Format("%s  %.1fG", &Drive[i], usb_allSize.QuadPart / (1024.0 * 1024.0 * 1024.0));
				((CComboBox*)dialog->GetDlgItem(ID))->AddString(strTemp);
			}	
			num++;
			IsFind = TRUE;
		}

		i += strlen(&Drive[i]) + 1;
	}
	if (IsFind)
	{
		((CComboBox*)dialog->GetDlgItem(ID))->AddString("");	//sel==0;
		((CComboBox*)dialog->GetDlgItem(ID))->SetCurSel(1);
	}
	else
	{
		((CComboBox*)dialog->GetDlgItem(ID))->AddString("找不到U盘");	//sel == 0;
		((CComboBox*)dialog->GetDlgItem(ID))->SetCurSel(0);
	}

	return num;
}


//每当选择的U盘改变时候, 自动更新 U盘的 分区表显示
void CUSB_ViewerDlg::OnCbnSelchangeCombo1()
{
	CString str;
	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	CListCtrl *m_ListCtrl = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	CComboBox *m_CComboBox = ((CComboBox*)GetDlgItem(IDC_COMBO1));
	m_ListCtrl->DeleteAllItems();	//清楚所有列表项

	m_CComboBox->GetWindowText(str);
	INT pos = str.Find(":\\");	//因为U盘的显示格式是"G:\ 16G"
	if (pos == -1)return;	//针对  空白的选项 和 找不到U盘 情况
	str = str.Mid(pos - 1, 2);		//现在str表示"G:"

	CCreateStartDlg m_CCreateStartDlg;
	UCHAR* m_sector = NULL;
	HANDLE hDrv = INVALID_HANDLE_VALUE;
	if (!m_CCreateStartDlg.GetDiskHandle(str.GetBuffer(), NULL, &hDrv))
		goto FINAL;

	Partition_Table list[4] = { 0 };
	INT num = 0;
	if (!(num = this->GetUSBInfo(&list, &hDrv, NULL, NULL)))
		goto FINAL;

	int nIndex;
	CHAR strSize[20] = { 0 };
	for (int i = 0; i < num; i++)
	{
		if (list[i].size < 1000)
			sprintf_s(strSize, sizeof(strSize), "(第%d分区)   %dM", i + 1, list[i].size);
		else
			sprintf_s(strSize, sizeof(strSize), "(第%d分区)   %.1fG", i + 1, (list[i].size / 1024.0));
		nIndex = m_ListCtrl->InsertItem(i, strSize);
		if (nIndex < 0)
			goto FINAL;

		m_ListCtrl->SetItemText(nIndex, 1, _T(list[i].type));
	}

	CloseHandle(hDrv);
	((CButton*)GetDlgItem(IDOK))->EnableWindow(TRUE);
	return;


FINAL:
	if (hDrv != INVALID_HANDLE_VALUE)
		CloseHandle(hDrv);

	m_CComboBox->GetWindowText(str);
	str.Format("读取(%s)U盘错误", str);
	m_ListCtrl->DeleteAllItems();	//清楚所有列表项
	m_ListCtrl->InsertItem(0, _T(str));
	return;
}


#include <Dbt.h>
//检测设备变动, 并更新界面
BOOL CUSB_ViewerDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	//nEventType就是WM_DEVICECHANGE消息的wParam参数，具体的值参考msdn
	switch (nEventType)
	{
		case 0x8000:		// DBT_DEVICEARRIVAL==0x8000
		{
			this->GetUSB(this, IDC_COMBO1);
			this->OnCbnSelchangeCombo1();		//更新列表框中 分区表 的显示

			/*DEV_BROADCAST_VOLUME* pDisk = (DEV_BROADCAST_VOLUME*)dwData;
			DWORD mask = pDisk->dbcv_unitmask;
			TCHAR diskname[MAX_PATH];
			int i = 0;
			for (i = 0; i < 32; i++)
			{
				if ((mask >> i) == 1)
				{
					diskname[0] = 'A' + i;//diskname就是盘符
					diskname[1] = '\0';
					_tcscat_s(diskname, TEXT(":\\  插入U盘"));
					AfxMessageBox(diskname);
					break;
				}
			}
			if (i == 32)
			{
				AfxMessageBox(TEXT("无效的分区名称!"));
			}*/
			return TRUE;
		}
		case 0x8004:		//DBT_DEVICEREMOVECOMPLETE==0x8004
			//::AfxMessageBox(TEXT("卸载了设备"), 1, 0);
			this->GetUSB(this, IDC_COMBO1);
			this->OnCbnSelchangeCombo1();		//更新列表框中 分区表 的显示
			return TRUE;
	}
	return 0;
}


/*
INT CUSB_ViewerDlg::FindDeviceInstanceID(LPCSTR path , PSTR* DeviceInstanceID)
{
	//path=="G:"
	CString str(path);
	str = "\\\\.\\" + str;
	HANDLE hDrv = CreateFile(str, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hDrv)
		goto ERROR1;

	STORAGE_DEVICE_NUMBER vde;
	DWORD dwBytes = 0;
	BOOL isOK = DeviceIoControl(hDrv, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &vde, sizeof(STORAGE_DEVICE_NUMBER), &dwBytes, NULL);
	if (!isOK)
		goto ERROR1;

	str.Format("\\\\.\\PhysicalDrive%d", INT(vde.DeviceNumber));
	AfxMessageBox(str);












	HDEVINFO hDevInfoSet;    //设备信息集句柄；
	SP_DEVICE_INTERFACE_DATA ifdata;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;
	int nCount = 0;
	BOOL bResult;

	GUID UsbClassGuid =
	{
		/ * //disk  * /0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b
		/// * //usb * / 0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed
	};


	// 取得一个该GUID相关的设备信息集句柄
	hDevInfoSet = ::SetupDiGetClassDevs((const GUID*)&UsbClassGuid,     // class GUID
		NULL,                    // 无关键字
		NULL,                    // 不指定父窗口句柄
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);    // 目前存在的设备

												   // 失败...
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// 申请设备接口数据空间

	pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, MAX_PATH);
	pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	nCount = 0;
	bResult = TRUE;

	// 设备序号=0,1,2... 逐一测试设备接口，到失败为止
	while (bResult)
	{
		ifdata.cbSize = sizeof(ifdata);

		// 枚举符合该GUID的设备接口

		bResult = ::SetupDiEnumDeviceInterfaces(
			hDevInfoSet,     // 设备信息集句柄
			NULL,            // 不需额外的设备描述
			&UsbClassGuid,          // GUID
			(ULONG)nCount,   // 设备信息集里的设备序号
			&ifdata);        // 设备接口信息

		if (bResult)
		{
			// 取得该设备接口的细节(设备路径)
			bResult = SetupDiGetInterfaceDeviceDetail(
				hDevInfoSet,    // 设备信息集句柄
				&ifdata,        // 设备接口信息
				pDetail,        // 设备接口细节(设备路径)
				MAX_PATH,   // 输出缓冲区大小
				NULL,           // 不需计算输出缓冲区大小(直接用设定值)
				NULL);          // 不需额外的设备描述

			
			if (bResult)
			{

				HANDLE hDrive = CreateFile(pDetail->DevicePath,
					0,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, NULL, NULL);
				if (hDrive != INVALID_HANDLE_VALUE)
				{
					STORAGE_DEVICE_NUMBER sdn;
					DWORD dwBytesReturned = 0;
					//通過這樣一個句柄，用同樣的方法也可以得到一個設備號。
					BOOL res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER,
						NULL, 0, &sdn, sizeof(sdn),
						&dwBytesReturned, NULL);
					if (res)
					{
						//這句是關鍵，通過這兩种方法獲得的.DeviceNumber，進行比較，以DeviceNumber作//爲橋梁，找到了對應的設備
						if (vde.DeviceNumber == (long)sdn.DeviceNumber)
						{

							SP_DEVINFO_DATA DeviceInfoData;
							DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
							SetupDiEnumDeviceInfo(hDevInfoSet, nCount, &DeviceInfoData);
							TCHAR DeviceInstanceId[MAX_PATH] = { 0 };
							DWORD nSize = 0;
							if (SetupDiGetDeviceInstanceId(
								hDevInfoSet,    // 设备信息集句柄
								&DeviceInfoData,        // 设备接口信息);
								DeviceInstanceId,
								MAX_PATH,
								&nSize
								))
							{
								this->FindDeviceInstanceID_by_USB(DeviceInstanceId, strlen(DeviceInstanceId), *DeviceInstanceID);
							}
							
								
							   
						}
					}
					CloseHandle(hDrive);

				}




				pDetail->DevicePath;
				// 调整计数值
				nCount++;
			}
		}
	}
	// 释放设备接口数据空间
	::GlobalFree(pDetail);
	// 关闭设备信息集句柄
	::SetupDiDestroyDeviceInfoList(hDevInfoSet);




	CloseHandle(hDrv);
	return TRUE;

ERROR1:
	CloseHandle(hDrv);
	return FALSE;
}


INT CUSB_ViewerDlg::FindDeviceInstanceID_by_USB(PSTR InstanceID_by_disk, INT sizeSrc,PSTR InstanceID_by_usb)
{
	//InstanceID_by_disk=="USBSTOR\DISK&VEN_MASS&PROD_STORAGE_DEVICE&REV__\125C20100726&0"

	CHAR m_InstanceID_by_disk[20] = { 0 };
	CHAR m_InstanceID_by_usb[MAX_PATH] = { 0 };
	int i;
	for (i = sizeSrc - 1; InstanceID_by_disk[i] != '\\'; i--) {};
	int j = 0;
	i++;
	do
	{
		m_InstanceID_by_disk[j++] = InstanceID_by_disk[i];
	} while (InstanceID_by_disk[i++]);
	i = 0;
	while (m_InstanceID_by_disk[i++] != '&'){}
	m_InstanceID_by_disk[--i] = '\0';


	HDEVINFO hDevInfoSet;    //设备信息集句柄；
	int nCount = 0;
	BOOL bResult;

	GUID UsbClassGuid =
	{
		/// * //disk* /0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b
		/ * //usb* / 0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed
	};

	// 取得一个该GUID相关的设备信息集句柄
	hDevInfoSet = ::SetupDiGetClassDevs((const GUID*)&UsbClassGuid,     // class GUID
		NULL,                    // 无关键字
		NULL,                    // 不指定父窗口句柄
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);    // 目前存在的设备
												   // 失败...
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	nCount = 0;
	bResult = TRUE;

	// 设备序号=0,1,2... 逐一测试设备接口，到失败为止
	while (bResult)
	{
		// 枚举符合该GUID的设备接口
		SP_DEVINFO_DATA DeviceInfoData;
		DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		bResult = SetupDiEnumDeviceInfo(hDevInfoSet, nCount, &DeviceInfoData);
		if (bResult)
		{
			DWORD nSize = 0;
			if (SetupDiGetDeviceInstanceId(
					hDevInfoSet,    // 设备信息集句柄
					&DeviceInfoData,        // 设备接口信息);
					m_InstanceID_by_usb,
					MAX_PATH,
					&nSize
					))
			{
				if (strstr(m_InstanceID_by_usb, m_InstanceID_by_disk) > 0)
				{
					strcpy_s(InstanceID_by_usb, MAX_PATH, m_InstanceID_by_usb);
				}

			}

		}
		nCount++;
	}
	// 关闭设备信息集句柄
	::SetupDiDestroyDeviceInfoList(hDevInfoSet);

	return TRUE;


}*/







/*



// TODO:  在此添加控件通知处理程序代码

INT Argc = 3; PTCHAR Argv[3] = { "0", "1", "0" };
TCHAR szName[MAX_PATH] = { 0 };
HANDLE hDisk;

//
// 命令行参数可在Project Settings->Debug->Program arguments下指定 如：0 1
//
if (Argc != 3) {

	_tprintf(_T("Reads a sector on the disk/n/n"));
	_tprintf(_T("%s [disk number] [sector]/n"), Argv[0]);
	return;
}

_sntprintf(szName, sizeof(szName) / sizeof(szName[0]) - 1,
	_T("////.//Physicaldrive%d"), _ttoi(Argv[1]));

//
// 打开磁盘
//
hDisk = CreateFile(szName,
	GENERIC_READ,
	FILE_SHARE_READ,
	NULL,
	OPEN_EXISTING,
	NULL,
	0);

if (hDisk != INVALID_HANDLE_VALUE)
{

	// 磁盘的结构信息存在此结束中。
	/ *
	typedef struct _DISK_GEOMETRY
	{
	LARGE_INTEGER Cylinders;    // 柱面数
	MEDIA_TYPE MediaType;       // 磁盘类型，见MSDN
	DWORD TracksPerCylinder;    // 每道柱面数
	DWORD SectorsPerTrack;      // 每道扇区数
	DWORD BytesPerSector;       // 每扇区字节数
	} DISK_GEOMETRY;
	* /

	DISK_GEOMETRY diskGeometry;


	DWORD dwBytes = 0;

	//
	// 获得磁盘结构信息
	//
	if (DeviceIoControl(hDisk,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,    // 调用了CTL_CODE macro宏
		NULL,
		0,
		&diskGeometry,
		sizeof(DISK_GEOMETRY),
		&dwBytes,
		NULL))
	{

		DWORD dwSize = diskGeometry.BytesPerSector;    // 每sector字节数
		PVOID lpBuffer = new BYTE[dwSize];

		if (lpBuffer)
		{

			/ *
			typedef struct _PARTITION_INFORMATION
			{
			LARGE_INTEGER StartingOffset;     // 启动分区偏移
			LARGE_INTEGER PartitionLength;    // 分区长度(字节)
			DWORD HiddenSectors;              // 分区中隐藏扇区数
			DWORD PartitionNumber;            // 分区数
			BYTE PartitionType;               // 分区类型
			BOOLEAN BootIndicator;            // 是否为引导分区，TRUE是
			BOOLEAN RecognizedPartition;      // 验证过的分区。TRUE是
			BOOLEAN RewritePartition;         // 分区是否可改变。TRUE
			} PARTITION_INFORMATION, *PPARTITION_INFORMATION;
			* /
			// 提供关于磁盘分区的信息
			PARTITION_INFORMATION partitionInfo;

			//
			// 获得磁盘大小
			//
			if (DeviceIoControl(hDisk,
				IOCTL_DISK_GET_PARTITION_INFO,
				NULL,
				0,
				&partitionInfo,
				sizeof(PARTITION_INFORMATION),
				&dwBytes,
				NULL))
			{

				// 获取总扇区数
				LONGLONG sectorCount = partitionInfo.PartitionLength.QuadPart / diskGeometry.BytesPerSector;

				_tprintf(_T("磁盘空间为 %.2fGB 每扇区 %ld字节 共%ld个扇区/r/n"),
					partitionInfo.PartitionLength.QuadPart / 1024. / 1024. / 1024.    //  磁盘空间
					, diskGeometry.BytesPerSector                               //  每扇区字节数
					, partitionInfo.PartitionLength.QuadPart / diskGeometry.BytesPerSector);  // 总扇区数

				LONGLONG nIndex = _ttoi64(Argv[2]);

				// 以16进制输出
				_tprintf(_T("Disk %d has 0x%I64x sectors with 0x%x bytes in every sector/n"), _ttoi(Argv[1]), sectorCount, diskGeometry.BytesPerSector);

				//
				// 读取被请求的sector
				//
				if (nIndex < sectorCount) {

					// 有符号的64位整型表示
					LARGE_INTEGER offset;

					// sector数所占字节
					offset.QuadPart = (nIndex)* diskGeometry.BytesPerSector;

					// 从打开的文件（磁盘）中移动文件指针。offset.LowPart低32位为移动字节数
					SetFilePointer(hDisk, offset.LowPart, &offset.HighPart, FILE_BEGIN);

					// 读取扇区的数据
					if (ReadFile(hDisk, lpBuffer, dwSize, &dwBytes, NULL))
					{

						//
						// The dwBytes field holds the number of bytes that were actually read [ <= dwSize ]
						//
						for (ULONG nOffset = 0; nOffset < dwBytes; nOffset += 0x10) {

							ULONG nBytes, nIdx;

							//
							// 显示地址
							//
							_tprintf(_T("%011I64x "), (offset.QuadPart) + nOffset);

							//
							// 显示16进制数据
							//
							nBytes = min(0x10, dwBytes - nOffset);

							for (nIdx = 0; nIdx < nBytes; nIdx++) {
								_tprintf(_T("%02x %s"), ((PUCHAR)lpBuffer)[nOffset + nIdx], ((nIdx + 1) % 0x8) ? _T("") : _T(" "));
							}

							for (; nIdx < 0x10; nIdx++) {
								_tprintf(_T(" %s"), ((nIdx + 1) % 0x8) ? _T("") : _T(" "));
							}

							//
							// 显示ascii格式数据
							//
							for (nIdx = 0; nIdx < nBytes; nIdx++) {
								_tprintf(_T("%c"), isprint(((PUCHAR)lpBuffer)[nOffset + nIdx]) ? ((PUCHAR)lpBuffer)[nOffset + nIdx] : _T('.'));
							}

							_tprintf(_T("/n"));
						}

					} // end ReadFile
					else
					{
						_tprintf(_T("ReadFile() on sector 0x%I64x failed with error code: %d/n"), nIndex, GetLastError());
					}

				} // end if (nIndex < sectorCount) 
				else
				{
					_tprintf(_T("The requested sector is out-of-bounds/n"));
				}

			} // end 1 if (DeviceIoControl   
			else
			{
				_tprintf(_T("IOCTL_DISK_GET_PARTITION_INFO failed with error code %d/n"), GetLastError());
			}

			delete[] lpBuffer;

		} // if (lpBuffer) 
		else
		{
			_tprintf(_T("Unable to allocate resources, exiting/n"));
		}

	} // end 2 if (DeviceIoControl
	else
	{
		_tprintf(_T("IOCTL_DISK_GET_DRIVE_GEOMETRY failed with error code %d/n"), GetLastError());
	}

	CloseHandle(hDisk);

} // if (hDisk != INVALID_HANDLE_VALUE) 
else
{
	_tprintf(_T("CreateFile() on %s failed with error code %d/n"), szName, GetLastError());
}
_tprintf(_T("/n"));*/

void CUSB_ViewerDlg::OnCeatestart()
{
	// TODO:  在此添加命令处理程序代码
	CCreateStartDlg dlg;
	INT_PTR nResponse = dlg.DoModal();
	this->GetUSB(this, IDC_COMBO1);		// 更新所有U盘设备,并显示到组合框中
	this->OnCbnSelchangeCombo1();		//更新列表框中 分区表 的显示
}


void CUSB_ViewerDlg::OnExit()
{
	// TODO:  在此添加命令处理程序代码
	exit(0);
}


INT CUSB_ViewerDlg::GetUSBInfo(
	Partition_Table(*list)[4],
	/*
	UCHAR sector[] / *= NULL* /,
	INT sectorSize/ * = 0* /,*/
	HANDLE *hDevice /*= NULL*/,
	LPCSTR drive /*= NULL*/,
	LPCSTR disk /*= NULL*/)
{
	if (!list)
		return FALSE;

	INT m_sectorSize = 0;
	INT res = FALSE;
	UCHAR *m_sector = NULL;
	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	DISK_GEOMETRY diskGeometry;
	CCreateStartDlg m_CreateStartDlg;
	Partition_Table m_list = { 0 };

	if (hDevice && hDevice != INVALID_HANDLE_VALUE)
	{
		m_hDevice = *hDevice;
	}
	else if (!m_CreateStartDlg.GetDiskHandle(drive, disk, &m_hDevice))
	{
		return FALSE;
	}

	//获得U盘Geometry结构
	if (!m_CreateStartDlg.GetDiskGeometry(&m_hDevice, NULL, NULL, &diskGeometry))
		goto FINAL;

	//获得U盘MBR扇区
	m_sector = new UCHAR[diskGeometry.BytesPerSector]();
	if (!(m_sectorSize = m_CreateStartDlg.GetDiskSector(m_sector, &m_hDevice, NULL, NULL, &diskGeometry)))
		goto FINAL;
	//CheckMbrPbr()有可能返回false
	INT type = m_CreateStartDlg.CheckMbrPbr(m_sector, m_sectorSize, &m_list);
	if (NO_MBR_PBR == type)
		goto FINAL;
	else if (PBR_FAT32 == type 
		|| PBR_NTFS == type
		|| PBR_FAT == type
		|| PBR_EXFAT == type)
	{
		memcpy_s(&(*list)[0], sizeof((*list)[0]), &m_list, sizeof(m_list));
		res = 1;	//即只找到一个分区
		goto FINAL;
	}
	else if (MBR == type)
	{
		INT num = 0;
		for (int i = 0; i < 4; i++)
		{
			if (!m_CreateStartDlg.GetOnePartitionInfo(&m_list, &m_hDevice, i + 1, &diskGeometry))
				continue;

			//经过多重检测, 则表示有效, 故赋值
			memcpy_s(&(*list)[num], sizeof((*list)[num]), &m_list, sizeof(m_list));
			num++;
		}
		res = num;
		goto FINAL;
	}
	else
		goto FINAL;


FINAL:
	if (!hDevice || *hDevice == INVALID_HANDLE_VALUE)
		CloseHandle(m_hDevice);
	if (m_sector)
		delete[] m_sector;

	return res;
}


void CUSB_ViewerDlg::OnClickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	*pResult = 1;		//古月猜测	1表示告诉系统,已经做处理了,不用系统再处理该事件

	CListCtrl* m_CalList = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	POSITION pos = m_CalList->GetFirstSelectedItemPosition();	
	static int prepos = 0;		//记录上一次选中的项
	if (pos <= 0)
	{
		if (prepos > 0)
			m_CalList->SetItemState(prepos - 1, LVIS_SELECTED , LVIS_SELECTED | LVS_SHOWSELALWAYS);
	}
	else
	{
		prepos = (int)pos;
	}

	//CString str;
	//str.Format("%d", pos);
	//MessageBox(str);

/*
	if (pNMItemActivate->iItem >= 0)
	{
		CListCtrl* m_CalList = ((CListCtrl*)GetDlgItem(IDC_LIST2));
		
		m_CalList->SetItemState(pNMItemActivate->iItem, LVIS_SELECTED || LVS_SHOWSELALWAYS, LVIS_SELECTED || LVS_SHOWSELALWAYS);	//LVS_SHOWSELALWAYS
		for (int i = 0; i < m_CalList->GetItemCount(); i++)
		{
			if (i == pNMItemActivate->iItem)
			{
				m_CalList->SetCheck(pNMItemActivate->iItem, TRUE);
				m_CalList->SetItemState(pNMItemActivate->iItem, LVIS_SELECTED || LVS_SHOWSELALWAYS, LVIS_SELECTED || LVS_SHOWSELALWAYS);	//LVS_SHOWSELALWAYS
			}
			else
			{
				m_CalList->SetCheck(i, FALSE);
				m_CalList->SetItemState(pNMItemActivate->iItem, 0, LVIS_SELECTED || LVS_SHOWSELALWAYS);	//LVS_SHOWSELALWAYS
			}
		}
		

	}*/

	
}

void CUSB_ViewerDlg::OnAbout()
{
	// TODO:  在此添加命令处理程序代码
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CUSB_ViewerDlg::OnBnClickedCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}



void CUSB_ViewerDlg::OnPartition()
{
	// TODO:  在此添加命令处理程序代码
	CPartitionDlg dlgPartition;
	dlgPartition.DoModal();
	this->GetUSB(this, IDC_COMBO1);		// 更新所有U盘设备,并显示到组合框中
	this->OnCbnSelchangeCombo1();		//更新列表框中 分区表 的显示
}
