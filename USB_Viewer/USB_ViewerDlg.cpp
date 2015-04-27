
// USB_ViewerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "afxdialogex.h"

#include "setupapi.h"
//#include "hidsdi.h"		//����guyue
#include "USB_Viewer.h"
#include "USB_ViewerDlg.h"
#include "CreateStartDlg.h"	//guyue
#include "PartitionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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
	ON_WM_DEVICECHANGE()		//���ڽ����豸(U��)�䶯, �Ѹ��½������Ϣ
	ON_COMMAND(ID_CeateStart, &CUSB_ViewerDlg::OnCeatestart)
	ON_COMMAND(ID_EXIT, &CUSB_ViewerDlg::OnExit)
	ON_NOTIFY(NM_CLICK, IDC_LIST2, &CUSB_ViewerDlg::OnClickList2)
	ON_COMMAND(ID_ABOUT, &CUSB_ViewerDlg::OnAbout)
	ON_BN_CLICKED(IDCANCEL, &CUSB_ViewerDlg::OnBnClickedCancel)
	ON_COMMAND(ID_Partition, &CUSB_ViewerDlg::OnPartition)
END_MESSAGE_MAP()
// CUSB_ViewerDlg �Ի���



CUSB_ViewerDlg::CUSB_ViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUSB_ViewerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUSB_ViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}













// SetupDiGetInterfaceDeviceDetail����Ҫ��������ȣ������㹻��
#define INTERFACE_DETAIL_SIZE    (1024)
//DEFINE_GUID(UsbClassGuid, 0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed);
// ����GUID����豸·��
// lpGuid: GUIDָ��
// pszDevicePath: �豸·��ָ���ָ��
// ����: �ɹ��õ����豸·�����������ܲ�ֹ1��
int GetDevicePath(LPGUID lpGuid, LPTSTR* pszDevicePath)
{
	
	HDEVINFO hDevInfoSet;    //�豸��Ϣ�������
	SP_DEVICE_INTERFACE_DATA ifdata;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;
	int nCount;
	BOOL bResult;
	
	GUID UsbClassGuid = 
	{
		/*//disk  */0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b
		///*//usb */ 0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed
	};


	// ȡ��һ����GUID��ص��豸��Ϣ�����
	hDevInfoSet = ::SetupDiGetClassDevs((const GUID*)&UsbClassGuid,     // class GUID
		NULL,                    // �޹ؼ���
		NULL,                    // ��ָ�������ھ��
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);    // Ŀǰ���ڵ��豸

												   // ʧ��...
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// �����豸�ӿ����ݿռ�
	pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, INTERFACE_DETAIL_SIZE);

	pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	nCount = 0;
	bResult = TRUE;

	// �豸���=0,1,2... ��һ�����豸�ӿڣ���ʧ��Ϊֹ
	while (bResult)
	{
		ifdata.cbSize = sizeof(ifdata);

		// ö�ٷ��ϸ�GUID���豸�ӿ�
		bResult = ::SetupDiEnumDeviceInterfaces(
			hDevInfoSet,     // �豸��Ϣ�����
			NULL,            // ���������豸����
			lpGuid,          // GUID
			(ULONG)nCount,   // �豸��Ϣ������豸���
			&ifdata);        // �豸�ӿ���Ϣ

		if (bResult)
		{
			// ȡ�ø��豸�ӿڵ�ϸ��(�豸·��)
			bResult = SetupDiGetInterfaceDeviceDetail(
				hDevInfoSet,    // �豸��Ϣ�����
				&ifdata,        // �豸�ӿ���Ϣ
				pDetail,        // �豸�ӿ�ϸ��(�豸·��)
				INTERFACE_DETAIL_SIZE,   // �����������С
				NULL,           // ������������������С(ֱ�����趨ֵ)
				NULL);          // ���������豸����
			if (bResult)
			{
				// �����豸·�������������
				CString strShow;
				strShow.Format("%08x-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n %s",
					lpGuid->Data1, lpGuid->Data2, lpGuid->Data3, lpGuid->Data4[0],
					lpGuid->Data4[1], lpGuid->Data4[2], lpGuid->Data4[3], lpGuid->Data4[4],
					lpGuid->Data4[5], lpGuid->Data4[6], lpGuid->Data4[7], pDetail->DevicePath);
				//printf("%s\n", strShow);
				//AfxMessageBox(strShow);

				//::strcpy(pszDevicePath[nCount], pDetail->DevicePath);
				// ��������ֵ
				nCount++;



				//////////////////////////////////////////////////////////////////////////
				SP_DEVINFO_DATA DeviceInfoData;
				DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
				nCount--;
				SetupDiEnumDeviceInfo(hDevInfoSet, nCount,&DeviceInfoData);
				TCHAR DeviceInstanceId[100] = {0};
				DWORD nSize = 0;
				int isOK = SetupDiGetDeviceInstanceId(
					hDevInfoSet,    // �豸��Ϣ�����
					&DeviceInfoData,        // �豸�ӿ���Ϣ);
					DeviceInstanceId,
					100,
					&nSize
					);
				nCount++;
				strShow.Append("\n��ʼ ", 4);
				strShow.Append(DeviceInstanceId,100);
				if (AfxMessageBox(strShow, 1, 0) == IDOK)
				{
					SP_PROPCHANGE_PARAMS spPropChangeParams;
					spPropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
					spPropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
					spPropChangeParams.Scope = DICS_FLAG_GLOBAL;
					spPropChangeParams.HwProfile = 0; // current hardware profile
					spPropChangeParams.StateChange = DICS_PROPCHANGE;
					/*	1 ��SP_PROPCHANGE_PARAMS�ṹ�帳����ȷ��ֵ
						2 �����渳��ֵ��SP_PROPCHANGE_PARAMS��Ϊ�������뵽SetupDiSetClassInstallParams����
						3 ����SetupDiCallClassInstaller()�����ݲ���DIF_PROPEFRTYCHANGE
						ʵ���ϣ�DIFҲ�ǰ�λ�����������ݵģ���Ҳ����ȥ���ݲ�ͬ��DIF����������SetupDiSetClassInstallParams()�� ������Ϣ����ο�MSDN��Handling DIF Codes��*/
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
							AfxMessageBox("�ɹ�ok");
							// ok, show disable success dialog
							// note, after that, the OS will post DBT_DEVICEREMOVECOMPLETE for the disabled device
						}
				}
				
			}
		}
	}
	// �ͷ��豸�ӿ����ݿռ�
	::GlobalFree(pDetail);
	// �ر��豸��Ϣ�����
	::SetupDiDestroyDeviceInfoList(hDevInfoSet);
	return nCount;
}




// CUSB_ViewerDlg ��Ϣ�������
BOOL CUSB_ViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//////////////////////////////////////////////////////////////////////////

	CListCtrl *m_wndPath = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	m_wndPath->SetExtendedStyle(m_wndPath->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );	//LVS_EX_CHECKBOXES
	m_wndPath->InsertColumn(0, _T("U���еķ�����С"));//�����
	m_wndPath->InsertColumn(1, _T("�ļ�ϵͳ"));
	m_wndPath->SetColumnWidth(0, 150);//�����п�
	m_wndPath->SetColumnWidth(1, 163);

	m_wndPath->SetRedraw(TRUE);//��ʾ

	//�ؼ����ݳ�ʼ��
	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	this->GetUSB(this, IDC_COMBO1);		// ��ȡ��ǰ����U���豸,����ʾ����Ͽ���
	this->OnCbnSelchangeCombo1();		//�����б���� ������ ����ʾ

	//m_wndPath->SetRedraw(FALSE);//��ֹ�ػ�

/*
	int nIndex;
	//char|TCHAR��Ŀ����->�ַ�����ʹ�ö��ֽ��ַ���
	TCHAR Path[MAX_PATH + 1];//TCHARȡ��char  MAX_PATH�·��
	nIndex = m_wndPath->InsertItem(0, _T("WindowsĿ¼"));
	if (nIndex < 0) return TRUE;
	GetWindowsDirectory(Path, MAX_PATH);//ȡ��windowsĿ¼
	m_wndPath->SetItemText(nIndex, 1, Path);

	LPITEMIDLIST pidl;//����CSIDL_DESKTOPDIRECTORY
					  //�����õ�ϵͳ��ĳЩ�ض��ļ��е�λ����Ϣ
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidl)))
	{
		if (SHGetPathFromIDList(pidl, Path))//�����ǰ���Ŀ��־���б�ת��Ϊ�ĵ�ϵͳ·��
		{
			nIndex = m_wndPath->InsertItem(0, _T("����"));//�ɹ��򷵻�0
			if (nIndex < 0)
			{
				return TRUE;
			}
			m_wndPath->SetItemText(nIndex, 1, Path);
		}
	}*/

	//m_wndPath->SetRedraw(TRUE);//��ʾ
	
	
	//////////////////////////////////////////////////////////////////////////






	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_NORMAL);
	
	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CUSB_ViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	CListCtrl *m_ListCtrl = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	CComboBox *m_CComboBox = ((CComboBox*)GetDlgItem(IDC_COMBO1));

	m_CComboBox->GetWindowText(str);
	INT pos = str.Find(":\\");	//��ΪU�̵���ʾ��ʽ��"G:\ 16G"
	if (pos == -1)return;		//���	���õĿհ�ѡ��
	str = str.Mid(pos - 1, 2);		//����str��ʾ"G:"
	

	//��ȡҪ�л���ʾ�ķ������, ps: m_ListCtrl->GetFirstSelectedItemPosition();  ��ѡ���򷵻� >0, ���� 0
	INT num = (INT)m_ListCtrl->GetFirstSelectedItemPosition();
	if (0 == num)
	{
		AfxMessageBox(TEXT("��, ��ѡ��Ҫ��ϵͳ��ʾ��U�̷�����^_^"));
		return;
	}

	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	CCreateStartDlg m_CreateStartDlg;
	if (m_CreateStartDlg.RemountDrive(num, FALSE, TEXT(str.GetBuffer())))
	{
		m_ListCtrl->DeleteAllItems();	//��������б���
		AfxMessageBox(TEXT("��, ��ϲ��, �����ɹ��� (0_0)\n  �쿴��U����~"));
	}
	else
	{
		AfxMessageBox(TEXT("��������, ����ʧ�ܰ�~��~��~~ T_T"));
	}
	((CButton*)GetDlgItem(IDOK))->EnableWindow(TRUE);
	this->GetUSB(this, IDC_COMBO1);		// ��ȡ��ǰ����U���豸,����ʾ����Ͽ���
	this->OnCbnSelchangeCombo1();		//ÿ��ѡ���U�̸ı�ʱ��, �Զ����� U�̵� �����б���ʾ


/*
	CString str;
	CListCtrl *m_ListCtrl = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	CComboBox *m_CComboBox = ((CComboBox*)GetDlgItem(IDC_COMBO1));
	m_ListCtrl->DeleteAllItems();	//��������б���

	m_CComboBox->GetWindowText(str);
	INT pos = str.Find("\\");	//��ΪU�̵���ʾ��ʽ��"G:\ 16G"
	str = str.Mid(0, pos);		//����str��ʾ"G:"

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
	// ��ô��̽ṹ��Ϣ
	//
	if (DeviceIoControl(hDrv,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,    // ������CTL_CODE macro��
		NULL,
		0,
		&diskGeometry,
		sizeof(DISK_GEOMETRY),
		&dwBytes,
		NULL))
	{

		DWORD dwSize = diskGeometry.BytesPerSector;    // ÿsector�����ֽ���
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
					AfxMessageBox(_T(" U�̷��������ɹ�!"));
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
	str.Format("��ȡ(%s)U�̴���", str);
	m_ListCtrl->InsertItem(0, _T(str));
	return;*/
}


// ��ȡ��ǰ����U���豸,����ʾ����Ͽ���
INT CUSB_ViewerDlg::GetUSB(CDialogEx* dialog, INT ID)
{
	CString strTemp;
	((CComboBox*)dialog->GetDlgItem(ID))->ResetContent();//����������������
	
	TCHAR Drive[MAX_PATH] = { 0 };
	DWORD len = ::GetLogicalDriveStrings(sizeof(Drive) / sizeof(TCHAR) , Drive);

	INT i = 0, num = 0;
	BOOL IsFind = FALSE;
	while (strlen(&Drive[i]) > 0)
	{
		if (::GetDriveType(&Drive[i]) == DRIVE_REMOVABLE)
		{
			//����ǿ��ƶ��豸, ��U�� , ����ʾ������, ��ʽ��"G:\ 16G"

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
				//��������1000M ����mb����ʾ��λ
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
		((CComboBox*)dialog->GetDlgItem(ID))->AddString("�Ҳ���U��");	//sel == 0;
		((CComboBox*)dialog->GetDlgItem(ID))->SetCurSel(0);
	}

	return num;
}


//ÿ��ѡ���U�̸ı�ʱ��, �Զ����� U�̵� ��������ʾ
void CUSB_ViewerDlg::OnCbnSelchangeCombo1()
{
	CString str;
	((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
	CListCtrl *m_ListCtrl = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	CComboBox *m_CComboBox = ((CComboBox*)GetDlgItem(IDC_COMBO1));
	m_ListCtrl->DeleteAllItems();	//��������б���

	m_CComboBox->GetWindowText(str);
	INT pos = str.Find(":\\");	//��ΪU�̵���ʾ��ʽ��"G:\ 16G"
	if (pos == -1)return;	//���  �հ׵�ѡ�� �� �Ҳ���U�� ���
	str = str.Mid(pos - 1, 2);		//����str��ʾ"G:"

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
			sprintf_s(strSize, sizeof(strSize), "(��%d����)   %dM", i + 1, list[i].size);
		else
			sprintf_s(strSize, sizeof(strSize), "(��%d����)   %.1fG", i + 1, (list[i].size / 1024.0));
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
	str.Format("��ȡ(%s)U�̴���", str);
	m_ListCtrl->DeleteAllItems();	//��������б���
	m_ListCtrl->InsertItem(0, _T(str));
	return;
}


#include <Dbt.h>
//����豸�䶯, �����½���
BOOL CUSB_ViewerDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	//nEventType����WM_DEVICECHANGE��Ϣ��wParam�����������ֵ�ο�msdn
	switch (nEventType)
	{
		case 0x8000:		// DBT_DEVICEARRIVAL==0x8000
		{
			this->GetUSB(this, IDC_COMBO1);
			this->OnCbnSelchangeCombo1();		//�����б���� ������ ����ʾ

			/*DEV_BROADCAST_VOLUME* pDisk = (DEV_BROADCAST_VOLUME*)dwData;
			DWORD mask = pDisk->dbcv_unitmask;
			TCHAR diskname[MAX_PATH];
			int i = 0;
			for (i = 0; i < 32; i++)
			{
				if ((mask >> i) == 1)
				{
					diskname[0] = 'A' + i;//diskname�����̷�
					diskname[1] = '\0';
					_tcscat_s(diskname, TEXT(":\\  ����U��"));
					AfxMessageBox(diskname);
					break;
				}
			}
			if (i == 32)
			{
				AfxMessageBox(TEXT("��Ч�ķ�������!"));
			}*/
			return TRUE;
		}
		case 0x8004:		//DBT_DEVICEREMOVECOMPLETE==0x8004
			//::AfxMessageBox(TEXT("ж�����豸"), 1, 0);
			this->GetUSB(this, IDC_COMBO1);
			this->OnCbnSelchangeCombo1();		//�����б���� ������ ����ʾ
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












	HDEVINFO hDevInfoSet;    //�豸��Ϣ�������
	SP_DEVICE_INTERFACE_DATA ifdata;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;
	int nCount = 0;
	BOOL bResult;

	GUID UsbClassGuid =
	{
		/ * //disk  * /0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b
		/// * //usb * / 0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed
	};


	// ȡ��һ����GUID��ص��豸��Ϣ�����
	hDevInfoSet = ::SetupDiGetClassDevs((const GUID*)&UsbClassGuid,     // class GUID
		NULL,                    // �޹ؼ���
		NULL,                    // ��ָ�������ھ��
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);    // Ŀǰ���ڵ��豸

												   // ʧ��...
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// �����豸�ӿ����ݿռ�

	pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, MAX_PATH);
	pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	nCount = 0;
	bResult = TRUE;

	// �豸���=0,1,2... ��һ�����豸�ӿڣ���ʧ��Ϊֹ
	while (bResult)
	{
		ifdata.cbSize = sizeof(ifdata);

		// ö�ٷ��ϸ�GUID���豸�ӿ�

		bResult = ::SetupDiEnumDeviceInterfaces(
			hDevInfoSet,     // �豸��Ϣ�����
			NULL,            // ���������豸����
			&UsbClassGuid,          // GUID
			(ULONG)nCount,   // �豸��Ϣ������豸���
			&ifdata);        // �豸�ӿ���Ϣ

		if (bResult)
		{
			// ȡ�ø��豸�ӿڵ�ϸ��(�豸·��)
			bResult = SetupDiGetInterfaceDeviceDetail(
				hDevInfoSet,    // �豸��Ϣ�����
				&ifdata,        // �豸�ӿ���Ϣ
				pDetail,        // �豸�ӿ�ϸ��(�豸·��)
				MAX_PATH,   // �����������С
				NULL,           // ������������������С(ֱ�����趨ֵ)
				NULL);          // ���������豸����

			
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
					//ͨ�^�@��һ���������ͬ�ӵķ���Ҳ���Եõ�һ���O��̖��
					BOOL res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER,
						NULL, 0, &sdn, sizeof(sdn),
						&dwBytesReturned, NULL);
					if (res)
					{
						//�@�����P�I��ͨ�^�@���ַ����@�õ�.DeviceNumber���M�б��^����DeviceNumber��//���������ҵ��ˌ������O��
						if (vde.DeviceNumber == (long)sdn.DeviceNumber)
						{

							SP_DEVINFO_DATA DeviceInfoData;
							DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
							SetupDiEnumDeviceInfo(hDevInfoSet, nCount, &DeviceInfoData);
							TCHAR DeviceInstanceId[MAX_PATH] = { 0 };
							DWORD nSize = 0;
							if (SetupDiGetDeviceInstanceId(
								hDevInfoSet,    // �豸��Ϣ�����
								&DeviceInfoData,        // �豸�ӿ���Ϣ);
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
				// ��������ֵ
				nCount++;
			}
		}
	}
	// �ͷ��豸�ӿ����ݿռ�
	::GlobalFree(pDetail);
	// �ر��豸��Ϣ�����
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


	HDEVINFO hDevInfoSet;    //�豸��Ϣ�������
	int nCount = 0;
	BOOL bResult;

	GUID UsbClassGuid =
	{
		/// * //disk* /0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b
		/ * //usb* / 0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed
	};

	// ȡ��һ����GUID��ص��豸��Ϣ�����
	hDevInfoSet = ::SetupDiGetClassDevs((const GUID*)&UsbClassGuid,     // class GUID
		NULL,                    // �޹ؼ���
		NULL,                    // ��ָ�������ھ��
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);    // Ŀǰ���ڵ��豸
												   // ʧ��...
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	nCount = 0;
	bResult = TRUE;

	// �豸���=0,1,2... ��һ�����豸�ӿڣ���ʧ��Ϊֹ
	while (bResult)
	{
		// ö�ٷ��ϸ�GUID���豸�ӿ�
		SP_DEVINFO_DATA DeviceInfoData;
		DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		bResult = SetupDiEnumDeviceInfo(hDevInfoSet, nCount, &DeviceInfoData);
		if (bResult)
		{
			DWORD nSize = 0;
			if (SetupDiGetDeviceInstanceId(
					hDevInfoSet,    // �豸��Ϣ�����
					&DeviceInfoData,        // �豸�ӿ���Ϣ);
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
	// �ر��豸��Ϣ�����
	::SetupDiDestroyDeviceInfoList(hDevInfoSet);

	return TRUE;


}*/







/*



// TODO:  �ڴ���ӿؼ�֪ͨ����������

INT Argc = 3; PTCHAR Argv[3] = { "0", "1", "0" };
TCHAR szName[MAX_PATH] = { 0 };
HANDLE hDisk;

//
// �����в�������Project Settings->Debug->Program arguments��ָ�� �磺0 1
//
if (Argc != 3) {

	_tprintf(_T("Reads a sector on the disk/n/n"));
	_tprintf(_T("%s [disk number] [sector]/n"), Argv[0]);
	return;
}

_sntprintf(szName, sizeof(szName) / sizeof(szName[0]) - 1,
	_T("////.//Physicaldrive%d"), _ttoi(Argv[1]));

//
// �򿪴���
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

	// ���̵Ľṹ��Ϣ���ڴ˽����С�
	/ *
	typedef struct _DISK_GEOMETRY
	{
	LARGE_INTEGER Cylinders;    // ������
	MEDIA_TYPE MediaType;       // �������ͣ���MSDN
	DWORD TracksPerCylinder;    // ÿ��������
	DWORD SectorsPerTrack;      // ÿ��������
	DWORD BytesPerSector;       // ÿ�����ֽ���
	} DISK_GEOMETRY;
	* /

	DISK_GEOMETRY diskGeometry;


	DWORD dwBytes = 0;

	//
	// ��ô��̽ṹ��Ϣ
	//
	if (DeviceIoControl(hDisk,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,    // ������CTL_CODE macro��
		NULL,
		0,
		&diskGeometry,
		sizeof(DISK_GEOMETRY),
		&dwBytes,
		NULL))
	{

		DWORD dwSize = diskGeometry.BytesPerSector;    // ÿsector�ֽ���
		PVOID lpBuffer = new BYTE[dwSize];

		if (lpBuffer)
		{

			/ *
			typedef struct _PARTITION_INFORMATION
			{
			LARGE_INTEGER StartingOffset;     // ��������ƫ��
			LARGE_INTEGER PartitionLength;    // ��������(�ֽ�)
			DWORD HiddenSectors;              // ����������������
			DWORD PartitionNumber;            // ������
			BYTE PartitionType;               // ��������
			BOOLEAN BootIndicator;            // �Ƿ�Ϊ����������TRUE��
			BOOLEAN RecognizedPartition;      // ��֤���ķ�����TRUE��
			BOOLEAN RewritePartition;         // �����Ƿ�ɸı䡣TRUE
			} PARTITION_INFORMATION, *PPARTITION_INFORMATION;
			* /
			// �ṩ���ڴ��̷�������Ϣ
			PARTITION_INFORMATION partitionInfo;

			//
			// ��ô��̴�С
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

				// ��ȡ��������
				LONGLONG sectorCount = partitionInfo.PartitionLength.QuadPart / diskGeometry.BytesPerSector;

				_tprintf(_T("���̿ռ�Ϊ %.2fGB ÿ���� %ld�ֽ� ��%ld������/r/n"),
					partitionInfo.PartitionLength.QuadPart / 1024. / 1024. / 1024.    //  ���̿ռ�
					, diskGeometry.BytesPerSector                               //  ÿ�����ֽ���
					, partitionInfo.PartitionLength.QuadPart / diskGeometry.BytesPerSector);  // ��������

				LONGLONG nIndex = _ttoi64(Argv[2]);

				// ��16�������
				_tprintf(_T("Disk %d has 0x%I64x sectors with 0x%x bytes in every sector/n"), _ttoi(Argv[1]), sectorCount, diskGeometry.BytesPerSector);

				//
				// ��ȡ�������sector
				//
				if (nIndex < sectorCount) {

					// �з��ŵ�64λ���ͱ�ʾ
					LARGE_INTEGER offset;

					// sector����ռ�ֽ�
					offset.QuadPart = (nIndex)* diskGeometry.BytesPerSector;

					// �Ӵ򿪵��ļ������̣����ƶ��ļ�ָ�롣offset.LowPart��32λΪ�ƶ��ֽ���
					SetFilePointer(hDisk, offset.LowPart, &offset.HighPart, FILE_BEGIN);

					// ��ȡ����������
					if (ReadFile(hDisk, lpBuffer, dwSize, &dwBytes, NULL))
					{

						//
						// The dwBytes field holds the number of bytes that were actually read [ <= dwSize ]
						//
						for (ULONG nOffset = 0; nOffset < dwBytes; nOffset += 0x10) {

							ULONG nBytes, nIdx;

							//
							// ��ʾ��ַ
							//
							_tprintf(_T("%011I64x "), (offset.QuadPart) + nOffset);

							//
							// ��ʾ16��������
							//
							nBytes = min(0x10, dwBytes - nOffset);

							for (nIdx = 0; nIdx < nBytes; nIdx++) {
								_tprintf(_T("%02x %s"), ((PUCHAR)lpBuffer)[nOffset + nIdx], ((nIdx + 1) % 0x8) ? _T("") : _T(" "));
							}

							for (; nIdx < 0x10; nIdx++) {
								_tprintf(_T(" %s"), ((nIdx + 1) % 0x8) ? _T("") : _T(" "));
							}

							//
							// ��ʾascii��ʽ����
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
	// TODO:  �ڴ���������������
	CCreateStartDlg dlg;
	INT_PTR nResponse = dlg.DoModal();
	this->GetUSB(this, IDC_COMBO1);		// ��������U���豸,����ʾ����Ͽ���
	this->OnCbnSelchangeCombo1();		//�����б���� ������ ����ʾ
}


void CUSB_ViewerDlg::OnExit()
{
	// TODO:  �ڴ���������������
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

	//���U��Geometry�ṹ
	if (!m_CreateStartDlg.GetDiskGeometry(&m_hDevice, NULL, NULL, &diskGeometry))
		goto FINAL;

	//���U��MBR����
	m_sector = new UCHAR[diskGeometry.BytesPerSector]();
	if (!(m_sectorSize = m_CreateStartDlg.GetDiskSector(m_sector, &m_hDevice, NULL, NULL, &diskGeometry)))
		goto FINAL;
	//CheckMbrPbr()�п��ܷ���false
	INT type = m_CreateStartDlg.CheckMbrPbr(m_sector, m_sectorSize, &m_list);
	if (NO_MBR_PBR == type)
		goto FINAL;
	else if (PBR_FAT32 == type 
		|| PBR_NTFS == type
		|| PBR_FAT == type
		|| PBR_EXFAT == type)
	{
		memcpy_s(&(*list)[0], sizeof((*list)[0]), &m_list, sizeof(m_list));
		res = 1;	//��ֻ�ҵ�һ������
		goto FINAL;
	}
	else if (MBR == type)
	{
		INT num = 0;
		for (int i = 0; i < 4; i++)
		{
			if (!m_CreateStartDlg.GetOnePartitionInfo(&m_list, &m_hDevice, i + 1, &diskGeometry))
				continue;

			//�������ؼ��, ���ʾ��Ч, �ʸ�ֵ
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
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	*pResult = 1;		//���²²�	1��ʾ����ϵͳ,�Ѿ���������,����ϵͳ�ٴ�����¼�

	CListCtrl* m_CalList = ((CListCtrl*)GetDlgItem(IDC_LIST2));
	POSITION pos = m_CalList->GetFirstSelectedItemPosition();	
	static int prepos = 0;		//��¼��һ��ѡ�е���
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
	// TODO:  �ڴ���������������
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CUSB_ViewerDlg::OnBnClickedCancel()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnCancel();
}



void CUSB_ViewerDlg::OnPartition()
{
	// TODO:  �ڴ���������������
	CPartitionDlg dlgPartition;
	dlgPartition.DoModal();
	this->GetUSB(this, IDC_COMBO1);		// ��������U���豸,����ʾ����Ͽ���
	this->OnCbnSelchangeCombo1();		//�����б���� ������ ����ʾ
}
