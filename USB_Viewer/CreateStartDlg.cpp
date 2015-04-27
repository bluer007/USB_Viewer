#include "stdafx.h"
#include "CreateStartDlg.h"
#include "BootData.h"

CCreateStartDlg::CCreateStartDlg() : CDialogEx(CCreateStartDlg::IDD)
{
	this->m_UnZipArg = NULL;
	this->m_FormatPartitionArg = NULL;
	this->m_FormatState = FALSE;
	this->m_needUnzip = FALSE;
	for (int i = sizeof(this->m_PartitionBootSector) / sizeof(this->m_PartitionBootSector[0]); i; i--)
	{
		this->m_PartitionBootSector[i - 1] = NULL;
	}
	this->m_hVolume = INVALID_HANDLE_VALUE;
	//this->m_USB_ViewerDlg
}


CCreateStartDlg::~CCreateStartDlg()
{
	if (this->m_UnZipArg)
		delete this->m_UnZipArg;

	if (this->m_FormatPartitionArg)
	{
		if (this->m_FormatPartitionArg->list)
			delete[] this->m_FormatPartitionArg->list;

		delete[] this->m_FormatPartitionArg;
	}

	for (int i = sizeof(this->m_PartitionBootSector) / sizeof(this->m_PartitionBootSector[0]); i; i--)
	{
		if (this->m_PartitionBootSector[i - 1])
			delete[] this->m_PartitionBootSector[i - 1];
	}

}

BOOL CCreateStartDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	this->m_USB_ViewerDlg->GetUSB(this, IDC_COMBO1);			//����Ͽ�����ʾ���ҵ���U��
	((CButton*)this->GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);		//��ѡ�� ����Ϊ �Ƽ�ѡ��--��ʽ������������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}
BEGIN_MESSAGE_MAP(CCreateStartDlg, CDialogEx)
ON_BN_CLICKED(IDOK, &CCreateStartDlg::OnBnClickedOk)
ON_MESSAGE(WM_MYMSG, &CCreateStartDlg::OnGetResult)
ON_WM_DEVICECHANGE()		//���ڽ����豸(U��)�䶯, �Ѹ��½������Ϣ
ON_WM_TIMER()
ON_BN_CLICKED(IDCANCEL, &CCreateStartDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDC_BOOT, &CCreateStartDlg::OnBnClickedBoot)
END_MESSAGE_MAP()


void CCreateStartDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//CDialogEx::OnOK();
	if (this->m_FormatState || this->m_needUnzip)
	{
		AfxMessageBox(TEXT("����������ing, ���Ժ�"));
		return;
	}

	CString str;
	CComboBox *m_CComboBox = ((CComboBox*)GetDlgItem(IDC_COMBO1));
	m_CComboBox->GetWindowText(str);
	INT pos = str.Find(":\\");	//��ΪU�̵���ʾ��ʽ��"G:\ 16G"
	if (pos == -1)
	{
		AfxMessageBox(TEXT("��ѡ��U�̲��ܽ��в�����"));
		return;		//���  �հ׵�ѡ��
	}
	str = str.Mid(pos - 1, 2);		//����str��ʾ"G:"

	CHAR m_isoPath[MAX_PATH] = {0};	
	GetCurrentDirectory(MAX_PATH, m_isoPath);		//��E:\Projects\test\test

	//����˳���������PE�����˳��, �ҵ���һ�����ھ������˳�ѭ��
	CHAR* tempPath[4] =
	{
		TEXT("\\Data\\�����Э��PE.ISO"),
		TEXT("\\�����Э��PE.ISO"),
		TEXT("\\Data\\CA.iso"),
		TEXT("\\CA.iso"),
	};
	HANDLE hfile = INVALID_HANDLE_VALUE;
	for (int i = 0; i < 4; i++)
	{
		strcat_s(m_isoPath, MAX_PATH, tempPath[i]);		//m_isoPath��iso·��

		hfile = CreateFile(TEXT(m_isoPath),
			GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL);

		if (INVALID_HANDLE_VALUE != hfile)
			break;
		m_isoPath[0] = '0';								//Ϊʹstrcat_s()��������,��������һ�ε�·��
		GetCurrentDirectory(MAX_PATH, m_isoPath);		//��E:\Projects\test\test;
	}
	
	if (INVALID_HANDLE_VALUE == hfile)
	{
		CloseHandle(hfile);
		AfxMessageBox(TEXT("�Ҳ��������ļ�, ����д�� T_T"));
		return;
	}
	LARGE_INTEGER fileSize = {0};
	GetFileSizeEx(hfile, &fileSize);		//���iso�ļ��Ĵ�С, �ֽ�Byte����λ

	LARGE_INTEGER usbSize = this->GetUSBAllSize(TEXT(str.GetBuffer()));		//���u�̵��ܴ�С, �ֽ�Byte����λ
	if (!fileSize.QuadPart || !usbSize.QuadPart)
	{
		CloseHandle(hfile);
		AfxMessageBox(TEXT("�����ļ� ��С �� U������ �쳣"));
		return;
	}
	
	
	//���� ѡ�� �� ��ͬ����������, ������Ӧ���ж�
	if (((CButton*)this->GetDlgItem(IDC_RADIO1))->GetCheck())		//��ѡ�� ����Ϊ �Ƽ�ѡ��--��ʽ������������
	{
		if (!this->m_FormatState && !this->m_needUnzip )
		{
			if (fileSize.QuadPart >= usbSize.QuadPart)
			{
				CloseHandle(hfile);
				AfxMessageBox(TEXT("U����������, ����д�뾵���ļ���"));
				return;
			}

			fileSize.QuadPart = (fileSize.QuadPart / (1024 * 1024)) + 1;	//��iso�ļ���Сת��ΪMB, +1�Ǳ������

			LARGE_INTEGER fat32size, ntfsSize;			//�ֱ��ʾfat32, ntfs�����Ĵ�С
			//�������, �����iso�ļ���50M��fat32����
			//iso����fat32��ʹ�СҪ��, �ͷ�����ʹ�С+100�Ĵ�С
			fileSize.QuadPart >= FAT32_SIZE_LIMIT ?			\
				fat32size.QuadPart = fileSize.QuadPart + 100 : fat32size.QuadPart = FAT32_SIZE_LIMIT + 100;
				
			usbSize.QuadPart = (usbSize.QuadPart / (1024 * 1024)) + 1;	//��U���ܴ�Сת��ΪMB, +1�Ǳ������
			ntfsSize.QuadPart = usbSize.QuadPart - fat32size.QuadPart;		//ʣ�µ�U�����������ntfs����
			Partition_Table table[] =
			{
				{ "ntfs", (ULONG64)ntfsSize.QuadPart },//{"ntfs", 1024 },		//1024MB == 2097152����	2097152
				{ "fat32", (ULONG64)fat32size.QuadPart },		//2097152mb == 4194304	4194304			ntfs	fat32
																//{"ntfs", 400 },		//1024	2048	3072	4096
																//{"ntfs", 2048 }
			};
			this->m_FormatState = TRUE;
			this->m_needUnzip = TRUE;
			if (!this->m_UnZipArg)
				this->m_UnZipArg = new UnZipArg();
			else
				ZeroMemory(this->m_UnZipArg, sizeof(*(this->m_UnZipArg)));
			
			//��д ��ѹ ����
			strcpy_s(this->m_UnZipArg->desPath, MAX_PATH, TEXT(str.GetBuffer()));
			strcpy_s(this->m_UnZipArg->isoPath, MAX_PATH, TEXT(m_isoPath));
			
			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, FALSE);		//��ʼ���ֶ�̬Ч��

			if (!this->Partition(TEXT(str.GetBuffer()), sizeof(table) / sizeof(Partition_Table), table, 2,
				this->GetSafeHwnd(), NULL))
			{
				//������ʧ��, ȡ�����ֶ�̬Ч��
				this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
				((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("��ʼ����!"));
				this->m_FormatState = FALSE;
				this->m_needUnzip = FALSE;
			}
	
		}
		else
		{
			CloseHandle(hfile);
			AfxMessageBox(TEXT("����������ing��, ���Ժ�~"));
			return;
		}

	}
	else if (((CButton*)this->GetDlgItem(IDC_RADIO2))->GetCheck())		//��ѡ�� ����Ϊ ֱ����������(����ʽ��)
	{
		HANDLE hDevice = INVALID_HANDLE_VALUE;
		DISK_GEOMETRY diskGeometry;
		
		if (!this->m_needUnzip)
		{
			if (!this->GetDiskHandle(str.GetBuffer(), NULL, &hDevice))
			{
				CloseHandle(hfile);
				//����U�̵�һ����������pbr�ķ���, �ǲ������������̵�
				AfxMessageBox(TEXT("���ź�, ��ȡU�̲���ʧ����T_T\n���� ѡ�� �Ƽ�ѡ��--��ʽ������������"));
				return;
			}
			if (!this->GetDiskGeometry(&hDevice, NULL, NULL, &diskGeometry))
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				//����U�̵�һ����������pbr�ķ���, �ǲ������������̵�
				AfxMessageBox(TEXT("���ź�, ��ȡU�̲���ʧ����T_T\n���� ѡ�� �Ƽ�ѡ��--��ʽ������������"));
				return;
			}

			INT sector_type = this->CheckMbrPbr(NULL, 0, NULL, &hDevice, NULL, NULL);
			if (PBR_FAT32 == sector_type 
				|| PBR_NTFS == sector_type
				|| PBR_FAT == sector_type
				|| PBR_EXFAT == sector_type)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				//����U�̵�һ����������pbr�ķ���, �ǲ������������̵�
				AfxMessageBox(TEXT("���ź�, U�̲����� ������������������ ����T_T\n���� ѡ�� �Ƽ�ѡ��--��ʽ������������"));
				return;
			}
			else if (FALSE == sector_type)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("���ź�, ��ȡU����Ϣ������"));
				return;
			}

			//�� MBR == sector_type
			//��Ȼ������������������, ������ÿռ��Ƿ��㹻
			unsigned long long i64FreeBytesToCaller = 0;
			unsigned long long i64TotalBytes = 0;

			GetDiskFreeSpaceEx(TEXT(str.GetBuffer()),
				(PULARGE_INTEGER)&i64FreeBytesToCaller,
				(PULARGE_INTEGER)&i64TotalBytes,
				NULL);
			//������  �ֽ� ����λ      10MB==10485760.0�ֽ�  �ÿ��ÿռ�Ⱦ����С>10M, ���յ�
			if (unsigned long long(fileSize.QuadPart + (LONGLONG)10485760.0) >= i64FreeBytesToCaller)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("�Ǻ�, ���U�̵� ���ÿռ� ����Ŷ~~\n����, ѡ���ʽ������U��, ������������㹻�ռ��ټ���^_^"));
				return;
			}

			//��ʼ����������, ����ʽ��
			this->m_needUnzip = TRUE;
			this->m_FormatState = FALSE;
			//���� �������һ����Ч�� Ϊ�����
			INT res_active = this->SetActivePartitionNum(5, &hDevice, NULL, NULL);
			if (-1 == res_active)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("���ź�, U�̵ķ����� ��֧������������ �� ��չ����\n���� ѡ�� �Ƽ�ѡ��--��ʽ������������"));
				this->m_needUnzip = FALSE;
				return;
			}
			else if(FALSE == res_active)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("���ź�, ���û����������T_T"));
				this->m_needUnzip = FALSE;
				return;
			}
			
			//��ʼд��mbr,pbr,�����ļ�
			//AfxMessageBox("������װ�������mbr�� pbr����");
			if (!this->InstallMBR(&hDevice, NULL, NULL, &diskGeometry))		//��װMBR����
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("���ź�, д��MBR����������T_T"));
				this->m_needUnzip = FALSE;
				return;
			}
			if (!this->InstallPBR(NULL, str.GetBuffer(), NULL, TRUE,&diskGeometry))		//��װ�������pbr����
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("���ź�, д��PBR����������T_T"));
				this->m_needUnzip = FALSE;
				return;
			}
			
			//��д д�뾵���  ����
			if (!this->m_UnZipArg)
				this->m_UnZipArg = new UnZipArg();
			else
				ZeroMemory(this->m_UnZipArg, sizeof(*(this->m_UnZipArg)));

			//��д ��ѹ ����
			strcpy_s(this->m_UnZipArg->desPath, MAX_PATH, TEXT(str.GetBuffer()));
			strcpy_s(this->m_UnZipArg->isoPath, MAX_PATH, TEXT(m_isoPath));

			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, FALSE);		//��ʼ���ֶ�̬Ч��

			if (!this->StartUnZip())	//��ʼд�뾵��(���̵߳�), ����Ҫ��д��mbr,pbr����
			{
				//����ѹʧ��, ȡ�����ֶ�̬Ч��
				this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
				((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("��ʼ����!"));
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("���ź�, д�뾵�������T_T"));
				this->m_needUnzip = FALSE;
				return;
			}

			//ִ�е����� ��ʾ,���ɹ�, ��������
			CloseHandle(hDevice);
			CloseHandle(hfile);
			return;
		}
		else
		{
			AfxMessageBox(TEXT("д��U��ing��, ���Ժ��~"));
			return;
		}

	}
	else
	{
		CloseHandle(hfile);
		AfxMessageBox(TEXT("��ѡ�� ���������̵ķ�ʽ ��^_^"));		//��ѡ��ѡ��
		return;
	}
}

#include <winioctl.h>
INT CCreateStartDlg::Partition(TCHAR* drive, INT num, Partition_Table* list, INT activeNum, HWND msgWnd, LPCSTR eventName)
{
	//num��list��������ЧԪ�صĸ���
	//drive��ʽ"G:"
	CString str(drive);
	DISK_GEOMETRY diskGeometry;
	DWORD dwBytes = 0;
	UCHAR *sector = NULL;

	if (num > 4 || num <= 0 || list == NULL || activeNum > num || activeNum < 0 || drive == NULL)
		goto ERROR1;
	
	HANDLE hDrv = INVALID_HANDLE_VALUE;
	if (!this->GetDiskHandle(drive, NULL, &hDrv))
		goto ERROR1;

	//if (!this->LockVolume(&hDrv, NULL))
	//	goto ERROR1;

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
		sector = new UCHAR[dwSize]();
		if (sector)
		{
			SetFilePointer(hDrv, 0, 0, FILE_BEGIN);
			//��ȡU�̵�һ������, ��mbr����
			ReadFile(hDrv, sector, dwSize, &dwBytes, NULL);
			if (dwBytes == dwSize)
			{
				//�ж�mbr�����ǲ��Ƿ�����pbr����,  ��������
				INT type = this->CheckMbrPbr(sector, dwSize, NULL, NULL, NULL, NULL);

				this->CreatePartitionTable(sector, dwSize, num, list, activeNum, &diskGeometry);

				dwBytes = 0;
				SetFilePointer(hDrv, 0, NULL, FILE_BEGIN);
				this->LockVolume(NULL, drive);
				//д���µķ�����
				WriteFile(hDrv, sector, dwSize, &dwBytes, NULL);
				this->UnLockVolume();
				if (dwBytes == dwSize)
				{
					//AfxMessageBox(_T(" U�̷��������óɹ�!"));
					if (type && type != MBR)	//������һ��������pbr����ʱ
					{
						//���mbr�������Ƿ���pbr�����Ļ�, ͨ���ı�������������İ취�ͻ�ʧ��, ����Ҫ��mbr����Ϊ��pbr����
						if (!this->InstallMBR(&hDrv, NULL, NULL, &diskGeometry))		//��װMBR����
							goto ERROR2;
					}
					//����������Ӧ��������������		���������  RemountDrive()ʱ��, ����window�ĸ�ʽ��U�̴���
					this->LockVolume(NULL, drive);
					INT deleteRes = this->DeletePartitionBootSector(&hDrv, num, sector, dwSize, &diskGeometry);	
					this->UnLockVolume();
					if (!this->RemountDrive(1, FALSE, drive))
						goto ERROR2;
					if (!deleteRes)
					{
						this->LockVolume(NULL, drive);
						//����������Ӧ��������������		���������  RemountDrive()ʱ��, ����window�ĸ�ʽ��U�̴���
						if (!this->DeletePartitionBootSector(&hDrv, num, sector, dwSize, &diskGeometry))
						{
							this->UnLockVolume();
							goto ERROR2;
						}
						this->UnLockVolume();
					}

					union
					{
						INT(__stdcall CCreateStartDlg::*FormatPartition)();
						unsigned int(__stdcall* ThreadAdress)(void*);
					}ThreadFun;
					ThreadFun.FormatPartition = &CCreateStartDlg::FormatPartition;
					/*struct FormatPartitionArg
					{
						HANDLE hDrv;
						CHAR drive[3];
						INT num;
						Partition_Table* list;
						HWND msgWnd;
						CHAR eventName[10];
					};*/
					FormatPartitionArg arg = {0};
					arg.hDrv = hDrv;
					strcpy_s(arg.drive, sizeof(arg.drive), drive);
					arg.num = num;
					arg.list = list;
					arg.msgWnd = msgWnd;
					if(eventName)
						strcpy_s(arg.eventName, sizeof(arg.eventName), eventName);
					else
						strcpy_s(arg.eventName, sizeof(arg.eventName), "");

					SetFormatPartitionArg(&arg);
					if (0 >= _beginthreadex(NULL, 0, ThreadFun.ThreadAdress, this, 0, NULL))
						goto ERROR2;

				}
				else
					goto ERROR2;
			}
			else
				goto ERROR2;
		}
		else
		{
			goto ERROR2;
		}
	}
	else
		goto ERROR2;

	//���ﲻҪCloseHandle(hDrv);   �����߳� CCreateStartDlg::FormatPartition()��Ϊִ��
	//����յ�	WM_MYMSG(FORMAT_OK)	��AfxMessageBox("U�̷����ɹ�");
	if (sector)
		delete[] sector;
	return TRUE;


ERROR2:
	::FlushFileBuffers(hDrv);
	CloseHandle(hDrv);
	
ERROR1:
	this->DismountVolume();
	//this->UnLockVolume();
	if (sector)
		delete[] sector;

	this->m_FormatState = FALSE;
	AfxMessageBox("U�̷���ʧ��(��û�������߳�)");
	return FALSE;

}

INT __stdcall CCreateStartDlg::FormatPartition()
{
	//drive��ʽ"G:"
	HANDLE m_hDrv = INVALID_HANDLE_VALUE;

	if (this->m_FormatPartitionArg->hDrv && this->m_FormatPartitionArg->hDrv != INVALID_HANDLE_VALUE)
	{
		m_hDrv = this->m_FormatPartitionArg->hDrv;
	}
	else if (this->m_FormatPartitionArg->drive)
	{
		this->GetDiskHandle(this->m_FormatPartitionArg->drive, NULL, &m_hDrv);
	}
	else
		return FALSE;

	LPSTR order[4] = { 0 };
	switch (this->m_FormatPartitionArg->num)
	{
	case 1:
		order[0] = "1234";
		break;
	case 2:
		order[0] = "2134";
		order[1] = "2134";
		break;
	case 3:
		order[0] = "2134";
		order[1] = "3124";
		order[2] = "3214";
		break;
	case 4:
		order[0] = "2134";
		order[1] = "3124";
		order[2] = "4312";
		order[3] = "2431";
		break;
	default:
		return FALSE;
	}

	CString str;
	DISK_GEOMETRY m_diskGeometry;
	if (!this->GetDiskGeometry(&m_hDrv, NULL, NULL, &m_diskGeometry))
		goto ERROR2;
	if (!this->DismountVolume(this->m_FormatPartitionArg->drive))
		goto ERROR2;
	for (int i = 0; i < this->m_FormatPartitionArg->num; i++)
	{

		if (!this->UpdateDisk(&m_hDrv, NULL, NULL))
			goto ERROR2;

		Sleep(1000);
		
		str.Format("%c: /q /fs:%s /y", *this->m_FormatPartitionArg->drive, this->m_FormatPartitionArg->list[i].type);
		
		HANDLE hProcess = INVALID_HANDLE_VALUE;
		int res = this->ShellExe(TEXT("Open"), TEXT("format.com"), TEXT("%system32%"), 
				str.GetBuffer(), SW_HIDE, 40000, &hProcess);		//40�� == 40000ms 	�ú����������ͺ���
		
		if (WAIT_TIMEOUT == res || WAIT_FAILED == res)
		{
			//Ҳ����ֱ�ӹر�������̣�ֻҪ����sei.hProcess���� 
			TerminateProcess(hProcess, 0);
			goto ERROR2;
		}
		else if (WAIT_OBJECT_0 == res)
		{
			//AfxMessageBox("��ʽ����,��������");
			Sleep(1000);
			//if (!this->BackupPartitionBootSector(&m_hDrv, i + 1, &m_diskGeometry))
			//goto ERROR2;

			if (this->ChangePartitionTable(&m_hDrv, this->m_FormatPartitionArg->drive, NULL, order[i]))		//��ȡ���ķ�������Ŀ˳������, 
			{
				if (!this->DismountVolume(this->m_FormatPartitionArg->drive))
					goto ERROR2;
				//CloseHandle(m_hDrv);
				//if (!this->GetDiskHandle(this->m_FormatPartitionArg->drive, NULL, &m_hDrv))
				//	goto ERROR1;
				continue;
			}
			else
				goto ERROR2;
		}
		else
		{
			AfxMessageBox("WaitForSingleObject �·���ֵ");
			goto ERROR2;
		}

/*
		SHELLEXECUTEINFO sei;
		// �������� 
		ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
		TCHAR Path[MAX_PATH + 1];//TCHARȡ��char  MAX_PATH�·��
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.hwnd = NULL;
		sei.lpVerb = NULL;
		sei.lpFile = "format.com";	//"cmd.exe ";		"format"
		GetWindowsDirectory(Path, MAX_PATH);	//ȡ��windowsĿ¼
		strcat_s(Path, MAX_PATH, "\\System32");
		sei.lpParameters = str;
		sei.lpDirectory = Path;
		sei.nShow = SW_HIDE;//SW_HIDE;SW_NORMAL
		sei.hInstApp = NULL;
		ShellExecuteEx(&sei);
		//   �������������ǵȴ��ý��̽��� 
		int res = WaitForSingleObject(sei.hProcess, 40000);		//40�� == 40000ms 
	*/
	}
	//AfxMessageBox("�����ָ���������");
	//if (!this->RestorePartitionBootSector(&m_hDrv, this->m_FormatPartitionArg->num, &m_diskGeometry))
	//	goto ERROR2;
	if (this->m_needUnzip)
	{
		//�����Ҫ��ѹ, ֤�����η������������������̲�����һ����, ��д��mbr,pbr,�����ļ�
		//AfxMessageBox("������װ�������mbr�� pbr����");
		if (!this->InstallMBR(&m_hDrv, NULL, NULL, &m_diskGeometry))		//��װMBR����
			goto ERROR2;
		if (!this->InstallPBR(&m_hDrv, NULL, NULL, FALSE, &m_diskGeometry))		//��װ�������pbr����
			goto ERROR2;
		if (!this->UpdateDisk(&m_hDrv, NULL, NULL))
			goto ERROR2;
		//AfxMessageBox("������ʼд�뾵��.");
		if(!this->StartUnZip())
			goto ERROR2;
	}


	if (this->m_FormatPartitionArg->msgWnd)
	{
		//�ɹ�����, ������Ϣ���ݳ�ȥ
/*
		if (!this->DismountVolume())
			goto ERROR2;
		if (!this->UnLockVolume())
			goto ERROR2;*/
		if (!::PostMessage(this->m_FormatPartitionArg->msgWnd, WM_MYMSG, FORMAT_OK, 0))
			goto ERROR2;
	}
	else if (this->m_FormatPartitionArg->eventName)
	{
		HANDLE eventHandle = INVALID_HANDLE_VALUE;
		if (!(eventHandle = OpenEvent(EVENT_MODIFY_STATE, FALSE, this->m_FormatPartitionArg->eventName)))
		{
			if (!SetEvent(eventHandle))
				goto ERROR2;
		}
		else
			goto ERROR2;
	}
	else
		goto ERROR2;

	::FlushFileBuffers(m_hDrv);
	//��if (!this->m_FormatPartitionArg->hDrv)	�����̴߳�ΪCloseHandle(m_hDrv);
	CloseHandle(m_hDrv);
	
	return TRUE;


ERROR2:
	::FlushFileBuffers(m_hDrv);
	//if (!this->m_FormatPartitionArg->hDrv)	�����̴߳�ΪCloseHandle(m_hDrv);
		CloseHandle(m_hDrv);
//ERROR1:
	//this->DismountVolume();
	//this->UnLockVolume();
	if (this->m_FormatPartitionArg->msgWnd)
	{
		::PostMessage(this->m_FormatPartitionArg->msgWnd, WM_MYMSG, FORMAT_ERROR, 0);
	}
	else if (this->m_FormatPartitionArg->eventName)
	{
		HANDLE eventHandle = INVALID_HANDLE_VALUE;
		if (!(eventHandle = OpenEvent(EVENT_MODIFY_STATE, FALSE, this->m_FormatPartitionArg->eventName)))
		{
			SetEvent(eventHandle);
		}
	}

	return FALSE;
}

INT CCreateStartDlg::BackupPartitionBootSector(HANDLE *hDevice, INT partitionNum, DISK_GEOMETRY* diskGeometry)
{
	//partitionNum ��1 ��ʼ, ��ʾU���ϵڼ������� , 1 , 2, 3, 4
	//BackupPartitionBootSector������ֻ�ǽ�sector�з�����ĵ�һ���Ӧ��U�̷����ĵ�һ�������������ݵ�this->m_PartitionBootSector[partitionNum - 1]��,
	//���ҽ��÷����ĵ�һ�������������

	if (!*hDevice || INVALID_HANDLE_VALUE == *hDevice || partitionNum <= 0 || partitionNum	> 4)
		return FALSE;

	DISK_GEOMETRY	m_diskGeometry;
	if (!diskGeometry)
	{
		if (!this->GetDiskGeometry(hDevice, NULL, NULL, &m_diskGeometry))
			return FALSE;
	}
	else
		m_diskGeometry = *diskGeometry;

	if (!this->m_PartitionBootSector[partitionNum - 1])
	{
		if (!(this->m_PartitionBootSector[partitionNum - 1] = new UCHAR[m_diskGeometry.BytesPerSector]()))
			return FALSE;
	}
	
	UCHAR* sector = new UCHAR[m_diskGeometry.BytesPerSector]();
	INT sector_size = this->GetDiskSector(sector, hDevice, NULL, NULL, &m_diskGeometry);
	if (!sector_size)
		return FALSE;

	INT sys_start_pos = 0;
	DWORD dwbytes = 0;
	LARGE_INTEGER offset;		// �з��ŵ�64λ���ͱ�ʾ
	UCHAR* newBootSector = new UCHAR[m_diskGeometry.BytesPerSector]();

	memcpy_s(&sys_start_pos, 4, &sector[sector_size - 66 + 8], 4);		//����ǰԤ������, ��������ʼ����

	offset.QuadPart = ((ULONG64)sys_start_pos) * ((ULONG64)m_diskGeometry.BytesPerSector);
	// �Ӵ򿪵��ļ������̣����ƶ��ļ�ָ�롣offset.LowPart��32λΪ�ƶ��ֽ���
	SetFilePointer(*hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
	ReadFile(*hDevice, this->m_PartitionBootSector[partitionNum - 1], m_diskGeometry.BytesPerSector, &dwbytes, NULL);
	if (m_diskGeometry.BytesPerSector == dwbytes)
	{
		SetFilePointer(*hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
		::WriteFile(*hDevice, newBootSector, m_diskGeometry.BytesPerSector, &dwbytes, NULL);
		if (m_diskGeometry.BytesPerSector == dwbytes)
		{
			delete[] sector;
			delete[] newBootSector;
			return TRUE;
		}
		else
		{
			delete[] sector;
			delete[] newBootSector;
			return FALSE;
		}
	}
	else
	{
		delete[] sector;
		delete[] newBootSector;
		return FALSE;
	}

}

INT CCreateStartDlg::RestorePartitionBootSector(HANDLE *hDevice, INT num, DISK_GEOMETRY* diskGeometry)
{
	//num��ʾ�������������Ч�����, 1,2, 3, 4
	DISK_GEOMETRY	m_diskGeometry;
	if (!diskGeometry)
	{
		if (!this->GetDiskGeometry(hDevice, NULL, NULL, &m_diskGeometry))
			return FALSE;
	}
	else
		m_diskGeometry = *diskGeometry;

	UCHAR* sector = new UCHAR[m_diskGeometry.BytesPerSector]();
	INT sector_size = this->GetDiskSector(sector, hDevice, NULL, NULL, &m_diskGeometry);
	if (!sector_size)
	{
		delete[] sector;
		return FALSE;
	}
		

	INT sys_start_pos[4] = {0};
	DWORD dwbytes = 0;
	LARGE_INTEGER offset;		// �з��ŵ�64λ���ͱ�ʾ
	for (int i = 0; i < num; i++)
	{
		memcpy_s(&sys_start_pos[i], 4, &sector[sector_size - 66 + 8 + i * 16], 4);		//����ǰԤ������, ��������ʼ����

		offset.QuadPart = ((ULONG64)sys_start_pos[i])* ((ULONG64)m_diskGeometry.BytesPerSector);
		// �Ӵ򿪵��ļ������̣����ƶ��ļ�ָ�롣offset.LowPart��32λΪ�ƶ��ֽ���
		SetFilePointer(*hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
		WriteFile(*hDevice, this->m_PartitionBootSector[i], m_diskGeometry.BytesPerSector, &dwbytes, NULL);
		if (m_diskGeometry.BytesPerSector != dwbytes)
		{
			delete[] sector;
			return FALSE;
		}
	}
	delete[] sector;
	return TRUE;
}

INT CCreateStartDlg::GetPartitionBootSector(UCHAR sector[], HANDLE *hDevice, INT partitionNum, DISK_GEOMETRY* diskGeometry)
{
	//partitionNum��ʾҪ��ȡ������������ �������partitionNum��������, ��Ϊ 1,2, 3, 4
	if (partitionNum > 4 || partitionNum <= 0 || !hDevice || !diskGeometry || !sector)
		return FALSE;

	INT sector_size = 0;
	if (!(sector_size = this->GetDiskSector(sector, hDevice, NULL, NULL, diskGeometry)))
		return FALSE;
	
	INT sector_type = this->CheckMbrPbr(sector, sector_size, NULL);	//�п��ܷ���false
	if (NO_MBR_PBR == sector_type || !sector_type)
		return FALSE;
	else if (PBR_NTFS == sector_type 
		|| PBR_FAT32 == sector_type
		|| PBR_FAT == sector_type
		|| PBR_EXFAT == sector_type)
	{
		if (1 == partitionNum)	//���pbr�ǵ�һ������, ��϶�U��ֻ��һ������,Ҳ���ǵ�һ������
			return sector_size;
		else
			return FALSE;
	}
	
	//����� ��MBR == sector_type
	DWORD sys_start_pos = 0;
	memcpy_s(&sys_start_pos, 4, &sector[sector_size - 66 + 8 + (partitionNum - 1) * 16], 4);		//����ǰԤ������, ��������ʼ����
	if (sys_start_pos <= 0)		//�Է���ǰԤ��������־λ����Ч�Լ��
		return FALSE;

	LARGE_INTEGER offset;
	DWORD dwBytes = 0;
	offset.QuadPart = (ULONG64)sys_start_pos * (ULONG64)diskGeometry->BytesPerSector;		// �з��ŵ�64λ���ͱ�ʾ 
	SetFilePointer(*hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
	ReadFile(*hDevice, sector, diskGeometry->BytesPerSector, &dwBytes, NULL);		//��ȡ�������һ����������
	if (dwBytes == diskGeometry->BytesPerSector)
	{
		return dwBytes;		//return TRUE;
	}
	else
		return FALSE;
}


INT CCreateStartDlg::GetDiskSector(UCHAR sector[], HANDLE *hDevice, LPCSTR drive, LPCSTR disk, DISK_GEOMETRY* diskGeometry)
{
	if (!sector || !diskGeometry)
		return FALSE;

	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	if (hDevice && *hDevice != INVALID_HANDLE_VALUE)
	{
		m_hDevice = *hDevice;
	}
	else if (drive || disk)
	{
		if (!this->GetDiskHandle(drive, disk, &m_hDevice))
			return FALSE;
	}
	else
	{
		return FALSE;
	}

	DWORD dwByte = 0;
	SetFilePointer(m_hDevice, 0, 0, FILE_BEGIN);
	ReadFile(m_hDevice, sector, diskGeometry->BytesPerSector, &dwByte, NULL);
	if (!hDevice)
		CloseHandle(m_hDevice);
	if (dwByte == diskGeometry->BytesPerSector)
	{
		return dwByte;		//return TRUE;
	}
	else
		return FALSE;

}

INT CCreateStartDlg::GetOnePartitionInfo(
	Partition_Table *list, 
	HANDLE *hDevice, 
	INT partitionNum, 
	DISK_GEOMETRY* diskGeometry, 
	UCHAR bootSector[] /*= NULL*/, INT sector_size /*= 0*/)
{
	//partitionNumΪ ������ڼ��� , ���ڼ�������(�����Ƿ���Ч), ��1, 2,3 4
	//��bootSector��Ч,��ֻ�����ļ�ϵͳ����������,������������������
	//��hDevice���ȼ���bootSector��

	INT res = FALSE;
	UCHAR* sector = NULL;
	if (!list || !hDevice || partitionNum > 4 || partitionNum <= 0 || !diskGeometry)
	{
		if (!bootSector || sector_size <= 0)
			return FALSE;
		else
		{
			sector = bootSector;
		}
	}
	else
	{
		sector = new UCHAR[diskGeometry->BytesPerSector]();
		if (!this->GetPartitionBootSector(sector, hDevice, partitionNum, diskGeometry))
			goto FINAL;
	}
	
	enum TYPE
	{
		NTFS,
		FAT32,
		FAT16,
		exFAT,
		UNKNOW
	};
	TYPE type = UNKNOW;
	CHAR name[6] = { 0 };
	memcpy_s(name, sizeof(name), &sector[3], 4);		//sector[3,4,5,6]��NTFS��File system ID ("NTFS")
	
	//�жϷ������ļ�ϵͳ
	if (strcmp(name, TEXT("NTFS")) == 0)
	{
		type = NTFS;
	}
	else
	{
		ZeroMemory(name, sizeof(name));
		memcpy_s(name, sizeof(name), &sector[82], 5);		//sector[82,83,84,85,86]��FAT32��File system��־λ("FAT32")
		if (strcmp(name, TEXT("FAT32")) == 0)
			type = FAT32;
		else
		{
			ZeroMemory(name, sizeof(name));
			memcpy_s(name, sizeof(name), &sector[54], 5);		//sector��FAT16��File system��־λ("FAT16")
			if (strcmp(name, TEXT("FAT16")) == 0)
				type = FAT16;
			else
			{
				ZeroMemory(name, sizeof(name));
				memcpy_s(name, sizeof(name), &sector[3], 5);		//sector��exFAT��File system��־λ("EXFAT")
				if (strcmp(name, TEXT("EXFAT")) == 0)
					type = exFAT;
				else
					goto FINAL;		//type = UNKNOW;
			}
				
		}
	}

	//���÷�����  �ļ�ϵͳtype �� ��Сsize ����
	DWORD partitionSize = 0;
	if (NTFS == type)
	{
		memcpy_s(&partitionSize, sizeof(DWORD), &sector[40], 4);		//ntfs���������� �����ܹ����� ��־λ
		if (partitionSize <= 0)
			goto FINAL;
		else
			partitionSize++;		//��Ϊntfs�� �����ܹ����� �������˵�һ������������ , ��fat32����û�����ٵ�

		strcpy_s(list->type, sizeof(list->type), TEXT("NTFS"));
		list->size = ULONG64((ULONG64)partitionSize * (ULONG64)diskGeometry->BytesPerSector / (1024.0 * 1024.0));
	}
	else if (FAT32 == type)
	{
		memcpy_s(&partitionSize, sizeof(DWORD), &sector[32], 4);		//fat32���������� �����ܹ����� ��־λ	Sectors (on large volumes)
		if (partitionSize <= 0)
			goto FINAL;

		strcpy_s(list->type, sizeof(list->type), TEXT("FAT32"));
		list->size = ULONG64((ULONG64)partitionSize * (ULONG64)diskGeometry->BytesPerSector / (1024.0 * 1024.0));
	}
	else if (FAT16 == type)
	{
		memcpy_s(&partitionSize, sizeof(DWORD), &sector[32], 4);		//fat16(fat)���������� �����ܹ����� ��־λ
		if (partitionSize <= 0)
			goto FINAL;

		strcpy_s(list->type, sizeof(list->type), TEXT("FAT"));
		list->size = ULONG64((ULONG64)partitionSize * (ULONG64)diskGeometry->BytesPerSector / (1024.0 * 1024.0));
	}
	else if (exFAT == type)
	{
		//������������Ŀ��־λ
		memcpy_s(&partitionSize, sizeof(DWORD), &sector[92], 4);
		//ÿ������С���ֽ�����2�ĳ˷���ʽ, һ����9, ��2^9=512�ֽ�, �����12
		UCHAR sectorSize = sector[108];
		//ÿ����������Ҳ��2�ĳ˷���ʽ, �����25, ��32M, �����ϲ�����06, ��2^6=64
		UCHAR sectorInCluster = sector[109];
		//partitionSize *= (2 ^ (sectorSize + sectorInCluster));
		if (sectorInCluster <= 0|| sectorSize <= 0|| partitionSize <= 0)
			goto FINAL;
		strcpy_s(list->type, sizeof(list->type), TEXT("exFAT"));
		list->size = ULONG64((ULONG64)partitionSize * (ULONG64)(pow(2, (sectorSize + sectorInCluster))) / (1024.0 * 1024.0));
	}
	else	
		goto FINAL;		//type == UNKNOW
	
	res = TRUE;
	goto FINAL;

FINAL:
	if (!bootSector)
	{
		if (sector)
			delete[] sector;
	}
	return res;
}

INT CCreateStartDlg::GetActivePartitionNum(HANDLE *hDevice, LPCSTR drive, LPCSTR disk)
{
	//����ֵ ��ʾ U�̷������ϵڼ����� Ϊ�����,��1,2,3,4,   NO_Active_Partition��ʾû�л����    FALSE�����
	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	UCHAR* sector = NULL;
	INT res = FALSE;
	if (hDevice && *hDevice != INVALID_HANDLE_VALUE)
	{
		m_hDevice = *hDevice;
	}
	else if (!this->GetDiskHandle(drive, disk, &m_hDevice))
	{
		return FALSE;
	}
		
	DISK_GEOMETRY diskGeometry = {0};
	if (!this->GetDiskGeometry(&m_hDevice, NULL, NULL, &diskGeometry))
		goto FINAL;

	sector = new UCHAR[diskGeometry.BytesPerSector]();
	INT sector_size = 0;
	if (!(sector_size = this->GetDiskSector(sector, &m_hDevice, NULL, NULL, &diskGeometry)))
		goto FINAL;

	INT m_ActivePartitionNum = 0;
	//Ѱ�һ����, ���ݻ������־λ
	for (m_ActivePartitionNum = 1; 
			m_ActivePartitionNum <= 4 
			&& sector[sector_size - 66 + (m_ActivePartitionNum - 1) * 16] != 0x80; 
			m_ActivePartitionNum++) {};
	if (m_ActivePartitionNum > 4)
	{
		res = NO_Active_Partition;
		goto FINAL;		//�Ҳ��������
	}
	else
	{
		res = m_ActivePartitionNum;
		goto FINAL;		//�Ҳ��������
	}

FINAL:
	if (sector)
		delete[] sector;
	if (!hDevice)
		CloseHandle(m_hDevice);
	return res;
}

INT CCreateStartDlg::SetActivePartitionNum(INT activePartitionNum, HANDLE *hDevice, LPCSTR drive, LPCSTR disk)
{
	//activePartitionNum ��ʾ ����U�̷������ϵڼ����� Ϊ�����,
	//��1,2,3,4,   0��ʾ��������,  5��ʾ��������ĵ�һ�� ��Ч�������� (��һ���ŵ�һ��)��Ϊ�����
	//����ֵFALSE�����, -1��Ҫ���õ�activePartitionNum��Ӧ�ķ�������չ����, TRUE��ɹ�����(����activePartitionNum == 0)
	if (activePartitionNum < 0 && activePartitionNum > 5)
		return FALSE;

	INT m_activePartitionNum = activePartitionNum;
	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	UCHAR* sector = NULL;
	INT res = FALSE;
	if (hDevice)
	{
		m_hDevice = *hDevice;
	}
	else if (!this->GetDiskHandle(drive, disk, &m_hDevice))
	{
		return FALSE;
	}

	DISK_GEOMETRY diskGeometry = { 0 };
	if (!this->GetDiskGeometry(&m_hDevice, NULL, NULL, &diskGeometry))
		goto FINAL;

	sector = new UCHAR[diskGeometry.BytesPerSector]();
	INT sector_size = 0;
	if (!(sector_size = this->GetDiskSector(sector, &m_hDevice, NULL, NULL, &diskGeometry)))
		goto FINAL;

	//ֻ�д���MBR������U�̲ſ������û����		CheckMbrPbr()�п��ܷ���false
	if (MBR != this->CheckMbrPbr(sector, sector_size, NULL, NULL, NULL, NULL))
		goto FINAL;

	if (5 == activePartitionNum)
	{
		BOOL isFindMain = FALSE;	//�Ƿ��ҵ���Ч������
		BOOL isFindExtend = FALSE;	//�Ƿ��ҵ� ��չ����
		DWORD sys_start_pos = 0;
		for (INT j = 0; j < 4; j++)
		{
			//����Ƿ�������,  --������ ,������չ����0F/05   ͨ���ļ�ϵͳ��־λ
			if (sector[sector_size - 66 + 4 + j * 16] != 0x0F
				&& sector[sector_size - 66 + 4 + j * 16] != 0x05)
			{
				//��������Ч��, ͨ������ǰ Ԥ������  ��־λ
				memcpy_s(&sys_start_pos, sizeof(sys_start_pos),
					&sector[sector_size - 66 + 8 + j * 16], 4);
				if (sys_start_pos > 0)
				{
					//�ҵ������� ��һ����Ч��
					m_activePartitionNum = j + 1;
					isFindMain = TRUE;		//�ҵ�������
					break;
				}
			}
			else
			{	//�ҵ� ��չ����
				isFindExtend = TRUE;
			}
		}

		if (!isFindMain && !isFindExtend)
		{	//û�������� �� ��չ����, ��U��ʲô������û��
			res = FALSE;
			goto FINAL;
		}
		else if (!isFindMain && isFindExtend)
		{	//û�������� �� ֻ����չ����, ��������������Ҫ��
			res = -1;
			goto FINAL;
		}
		//ʣ�µľ����ҵ���������
	}

	for (INT i = 1; i <= 4; i++)
	{
		if (m_activePartitionNum == i)
		{
			//���  �ļ�ϵͳ����  ��־λ �Ƿ�Ϊ ��չ����0F/05
			if (sector[sector_size - 66 + 4 + (i - 1) * 16] != 0x0F
				&& sector[sector_size - 66 + 4 + (i - 1) * 16] != 0x05)
			{
				sector[sector_size - 66 + (i - 1) * 16] = 0x80;
				//��ѭ������ִ��, ʹ �ǻ���� ��Ϊ0x00
			}
			else
			{
				res = -1;	//��չ������������������, ֻ���������ſ���
				goto FINAL;
			}
		}
		else
			sector[sector_size - 66 + (i - 1) * 16] = 0x00;
	}

	//��ʼд������
	DWORD dwBytes = 0;
	SetFilePointer(m_hDevice, 0, 0, FILE_BEGIN);
	WriteFile(m_hDevice, sector, sector_size, &dwBytes, NULL);
	if (dwBytes == sector_size)
	{
		res = TRUE;
		goto FINAL;
	}
	else
	{
		goto FINAL;		//res = FALSE;
	}


FINAL:
	if (sector)
		delete[] sector;
	if (!hDevice)
		CloseHandle(m_hDevice);
	return res;
}

INT CCreateStartDlg::ChangePartitionTable(HANDLE *hDisk, LPCSTR drive, LPCSTR disk, LPCSTR order/* = NULL*/)
{
	//drive��ʽ"G:"
	//disk��ʽ"1"
	//order��ʽ"1234"�� "2134" ��, ��ΪNULL , ���ʾĬ��"2134"

	HANDLE hDrv = INVALID_HANDLE_VALUE;
	if (hDisk && *hDisk != INVALID_HANDLE_VALUE)
	{
		hDrv = *hDisk;
	}
	else
	{
		if (!this->GetDiskHandle(drive, disk, &hDrv))
			return FALSE;
	}

	DWORD dwBytes = 0;
	DISK_GEOMETRY diskGeometry;
	UCHAR *sector = NULL;
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
		sector = new UCHAR[dwSize]();
		if (sector)
		{
			SetFilePointer(hDrv, 0, 0, FILE_BEGIN);
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

				CString m_order(order);
				if (NULL == order)
				{
					m_order.SetString("2134");
				}
				for (int i = 0; i < 4; i++)
				{
					switch (m_order[i])
					{
					case '1':
						memcpy_s(&sector[dwBytes - 66 + i * 16], 16, DPT1, 16);
						break;
					case '2':
						memcpy_s(&sector[dwBytes - 66 + i * 16], 16, DPT2, 16);
						break;
					case '3':
						memcpy_s(&sector[dwBytes - 66 + i * 16], 16, DPT3, 16);
						break;
					case '4':
						memcpy_s(&sector[dwBytes - 66 + i * 16], 16, DPT4, 16);
						break;
					default:
						goto ERROR2;
					}
				}

				dwBytes = 0;
				SetFilePointer(hDrv, 0, NULL, FILE_BEGIN);
				this->LockVolume(NULL, drive);
				WriteFile(hDrv, sector, dwSize, &dwBytes, NULL);
				this->UnLockVolume();
				if (dwBytes == dwSize)
				{
					delete[] sector;
					if (!hDisk)
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
			goto ERROR2;
		}
	}
	else
		goto ERROR2;


	return TRUE;


ERROR2:
	if (sector)
		delete[] sector;

	if (!hDisk)
		CloseHandle(hDrv);
	return FALSE;
}

INT CCreateStartDlg::CreatePartitionTable(UCHAR sector[], INT sector_size, INT num, Partition_Table* list, INT activeNum, DISK_GEOMETRY* diskGeometry)
{
	unsigned int temp_pre = 0;

	memset(&sector[sector_size - 66], 0x00, 64);		//16 * 4 ==64  ��ԭ���ķ�����ɾ��,׼���½�������
	for (int i = 0; i < num; i++)
	{
		unsigned char sys_active = { 0 };				//�������־
		unsigned char sys_pre_start[4] = { 0 };			//����ǰ Ԥ������ ��־
		unsigned char sys_size[4] = { 0 };				//������С ��־
		unsigned char sys_start_pos[3] = { 0 };			//������ʼλ�ñ�־
		unsigned char sys_filesystem = { 0 };			//�ļ�ϵͳ��־
		unsigned char sys_end_pos[3] = { 0 };			//��������λ�ñ�־


		if (activeNum == 0 || activeNum != i + 1)		//activeNum == =0 ��û�л����
		{
			sys_active = 0x00;		//����� ��־λ
		}
		else	//activeNum>0ʱ��,���activeNum����Ϊ�����
		{
			sys_active = 0x80;		//����� ��־λ
		}

		if (0 == i)
		{
			temp_pre = unsigned int(1024.0 * 1024.0 / diskGeometry->BytesPerSector);		//���������� ����ǰԤ������ ��־λ(�ָ���diskgenius����,�ڵ�һ����ǰԤ��1MB��С)
		}
		else
		{
			temp_pre = unsigned int(temp_pre + (list[i - 1].size) * 1024.0 * 1024.0 / diskGeometry->BytesPerSector);		//���������� ����ǰԤ������ ��־λ
		}

		for (int j = 0; j < 4; j++)
		{
			sys_pre_start[j] = *(((unsigned char*)&temp_pre) + j);		//���������� ����ǰԤ������ ��־λ		(ע���С��)
		}

		unsigned int temp_size = unsigned int((list[i].size) * 1024.0 * 1024.0 / diskGeometry->BytesPerSector);		// list[i].size��ָMBΪ��λ��,����*1024.0 * 1024.0
		for (int j = 0; j < 4; j++)
		{
			sys_size[j] = *(((unsigned char*)&temp_size) + j);		//���������� ������С ��־λ	(ע���С��)
		}

		sys_start_pos[2] = unsigned char(temp_pre / (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack));	//������ʼ����
		sys_start_pos[1] = unsigned char(temp_pre % (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) % diskGeometry->SectorsPerTrack + 1);	//������ʼ����
		sys_start_pos[0] = unsigned char(temp_pre % (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) / diskGeometry->SectorsPerTrack);	//������ʼ��ͷ

		if (strcmp(list[i].type, _T("ntfs")) == 0
			|| strcmp(list[i].type, _T("NTFS")) == 0)
		{
			sys_filesystem = 0x07;		//�ļ�ϵͳ��־λ
		}
		else if (strcmp(list[i].type, _T("fat32")) == 0
			|| strcmp(list[i].type, _T("FAT32")) == 0)
		{
			sys_filesystem = 0x0c;
		}
		else
		{
			sys_filesystem = 0x00;			//�ļ�ϵͳΪδʹ��
		}

		if ((temp_pre + temp_size - 1) / (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) > 255)
		{
			sys_end_pos[2] = 0xff;
			sys_end_pos[1] = 0xff;
			sys_end_pos[0] = 0xfe;
		}
		else
		{
			sys_end_pos[2] = unsigned char((temp_pre + temp_size - 1) / (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack));	//������������ (ע��������һ���ֽ����洢��С,���Կ϶��������255, ��391�ḳֵ��Ϊ135)
			sys_end_pos[1] = unsigned char((temp_pre + temp_size - 1) % (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) % diskGeometry->SectorsPerTrack + 1);	//������������
			sys_end_pos[0] = unsigned char((temp_pre + temp_size - 1) % (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) / diskGeometry->SectorsPerTrack);	//����������ͷ
		}

		unsigned char table[16] = 				//�������еĵ�����Ŀ
		{
			/*
			[0]				�������־
			[1]				������ʼ��ͷ
			[2]				������ʼ����
			[3]				������ʼ����
			[4]				�ļ�ϵͳ��־
			[5]				����������ͷ
			[6]				������������
			[7]				������������
			[8,9,10,11]		����ǰ Ԥ������
			[12,13,14,15]	������С*/
			sys_active,
			sys_start_pos[0], sys_start_pos[1], sys_start_pos[2],
			sys_filesystem,
			sys_end_pos[0], sys_end_pos[1], sys_end_pos[2],
			sys_pre_start[0], sys_pre_start[1], sys_pre_start[2], sys_pre_start[3],
			sys_size[0], sys_size[1], sys_size[2], sys_size[3]
		};
		memcpy_s(&sector[sector_size - 66 + i * 16], sizeof(table) / sizeof(table[0]),
										table, sizeof(table) / sizeof(table[0]));
	}

	return TRUE;
}

INT CCreateStartDlg::DeletePartitionBootSector(HANDLE *hDevice, INT num, UCHAR sector[], INT sector_size, DISK_GEOMETRY* diskGeometry)
{
	//sector[]��ָdisksector, �����̵ĵ�һ����, ��Ϊ�����з�������Ϣ
	//num��ָdisksector�з���������Ч���������Ŀ, ���ڷ���������Ч����ǰ�˳�����еļ���
	if (!*hDevice || INVALID_HANDLE_VALUE == *hDevice || !diskGeometry)
		return FALSE;
	//CloseHandle(*hDevice);
	//this->GetDriveHandle(TEXT("G:"), hDevice);
	DWORD sys_start_pos[4] = {0};
	UCHAR* newBootSector = NULL;

	DWORD dwbytes = 0;
	LARGE_INTEGER offset;		// �з��ŵ�64λ���ͱ�ʾ
	for (int i = 0; i < 4; i++)
	{
		memcpy_s(&sys_start_pos[i], 4, &sector[sector_size - 66 + 8 + i * 16], 4);		//����ǰԤ��������־λ, ��������ʼ����
		if (!sys_start_pos[i])		//����ǰԤ��������־λΪ0, ˵���÷���������Ч
			continue;
		//���ݷ������ͱ�־λȷ����������
		if (0x0c == sector[sector_size - 66 + 4 + i * 16] 
			|| 0x1c == sector[sector_size - 66 + 4 + i * 16])
		{
			newBootSector = pbr_fat32_data;		//fat32����
			//���� ����ǰԤ��������־λ, ��������ʼ����  �� ������С��־λ
			memcpy_s(&newBootSector[28], 8, &sector[sector_size - 66 + 8 + i * 16], 8);
		}
		else if (0x07 == sector[sector_size - 66 + 4 + i * 16]
			|| 0x17 == sector[sector_size - 66 + 4 + i * 16])
		{
			newBootSector = pbr_ntfs_data;		//ntfs����
			//���� ����ǰԤ��������־λ, ��������ʼ����
			memcpy_s(&newBootSector[28], 4, &sector[sector_size - 66 + 8 + i * 16], 4);
			//���� ������С��־λ
			DWORD ntfs_size = 0;
			memcpy_s(&ntfs_size, 4, &sector[sector_size - 66 + 12 + i * 16],4);
			ntfs_size--;
			//��Ϊntfs������������, �ļ���С��־λ(Total sectors)�Ǽ�1��
			memcpy_s(&newBootSector[40], 4, &ntfs_size, 4);
		}

		offset.QuadPart = ((ULONG64)sys_start_pos[i]) * ((ULONG64)diskGeometry->BytesPerSector);
		// �Ӵ򿪵��ļ������̣����ƶ��ļ�ָ�롣offset.LowPart��32λΪ�ƶ��ֽ���
		SetFilePointer(*hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
		::WriteFile(*hDevice, newBootSector, diskGeometry->BytesPerSector, &dwbytes, NULL);
		if (diskGeometry->BytesPerSector != dwbytes)
		{
			//DWORD err = GetLastError();
			//CString str;
			//str.Format("DeletePartitionBootSector()�д���,�������%d", err);
			//AfxMessageBox(str);
			goto ERROR1;
		}
	}
	//CloseHandle(*hDevice);
	//this->GetDiskHandle(TEXT("G:"), NULL,hDevice);
	return TRUE;

ERROR1:
	//CloseHandle(*hDevice);
	//this->GetDiskHandle(TEXT("G:"), NULL, hDevice);
	return FALSE;
}


INT CCreateStartDlg::UpdateDisk(HANDLE *hDevice, LPCSTR drive, LPCSTR disk)
{
	//drive��ʽ"G:"
	//disk��ʽ"1"

	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	
	if (hDevice && *hDevice != INVALID_HANDLE_VALUE)
	{
		m_hDevice = *hDevice;
	}
	else if (drive || disk)
	{
		if (!this->GetDiskHandle(drive, disk, &m_hDevice))
			return FALSE;
	}
	else
	{
		return FALSE;
	}
		
	DWORD readed = 0;
	//fresh the partition table
	if (!DeviceIoControl(
		m_hDevice,
		IOCTL_DISK_UPDATE_PROPERTIES,
		NULL,
		0,
		NULL,
		0,
		&readed,
		NULL
		))
	{
		if (hDevice && *hDevice != INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		else
		{
			CloseHandle(m_hDevice);
			return FALSE;
		}

	}

	if (hDevice && *hDevice != INVALID_HANDLE_VALUE)
	{
		return TRUE;
	}
	else
	{
		CloseHandle(m_hDevice);
		return TRUE;
	}
}

INT CCreateStartDlg::GetDiskHandle(LPCSTR drive, LPCSTR disk, HANDLE *hDevice)
{
	//drive��ʽ"G:"
	//disk��ʽ"1"
	if (!drive && !disk || !hDevice)
	{
		if(hDevice)
			*hDevice = INVALID_HANDLE_VALUE;
		
		return FALSE;
	}
		

	HANDLE m_hDevice = *hDevice = INVALID_HANDLE_VALUE;
	CString str;
	if (drive)
	{
		str.Format("\\\\.\\%c:", *drive);
		m_hDevice = CreateFile(str, 
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, 0, NULL);

		if (INVALID_HANDLE_VALUE == m_hDevice)
			return FALSE;

		VOLUME_DISK_EXTENTS vde;
		DWORD dwBytes = 0;
		if (!DeviceIoControl(m_hDevice,
			IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
			NULL, 0,
			&vde, sizeof(VOLUME_DISK_EXTENTS),
			&dwBytes, NULL))
			goto ERROR1;

		CloseHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;

		str.Format("\\\\.\\PhysicalDrive%d", vde.Extents->DiskNumber);
	}
	else if(disk)
	{
		str.Format("\\\\.\\PhysicalDrive%c", *disk);
	}

	m_hDevice = CreateFile(str,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, 0, NULL);

	if (INVALID_HANDLE_VALUE == m_hDevice)
		goto ERROR1;
	else
	{
		*hDevice = m_hDevice;
		return TRUE;
	}
		

ERROR1:
	CloseHandle(m_hDevice);
	return FALSE;
}

INT CCreateStartDlg::SetFormatPartitionArg(FormatPartitionArg* arg)
{
	if (arg)
	{	
		if (this->m_FormatPartitionArg)
		{
			if (this->m_FormatPartitionArg->list)
				delete[] this->m_FormatPartitionArg->list;

			delete[] this->m_FormatPartitionArg;
		}
		/*
		struct FormatPartitionArg
		{
		HANDLE hDrv;
		CHAR drive[3];
		INT num;
		Partition_Table* list;
		HWND msgWnd;
		CHAR eventName[10];
		};
		*/
		this->m_FormatPartitionArg = new FormatPartitionArg();
		this->m_FormatPartitionArg->hDrv = arg->hDrv;
		strcpy_s(this->m_FormatPartitionArg->drive, 
			sizeof(this->m_FormatPartitionArg->drive), arg->drive);
		
		this->m_FormatPartitionArg->num = arg->num;
		this->m_FormatPartitionArg->list = new Partition_Table[arg->num] ();
		for (int i = 0; i < arg->num; i++)
		{
			this->m_FormatPartitionArg->list[i].size = arg->list[i].size;
			char a[8] = {0};
			strcpy_s(a,
				sizeof(this->m_FormatPartitionArg->list[i].type), arg->list[i].type);
			strcpy_s(this->m_FormatPartitionArg->list[i].type, 
				sizeof(this->m_FormatPartitionArg->list[i].type), arg->list[i].type);
		}

		this->m_FormatPartitionArg->msgWnd = arg->msgWnd;
		strcpy_s(this->m_FormatPartitionArg->eventName,
			sizeof(this->m_FormatPartitionArg->eventName), arg->eventName);
	
		return TRUE;
	}
	else
		return FALSE;
}


afx_msg LRESULT CCreateStartDlg::OnGetResult(WPARAM wParam, LPARAM lParam)
{
	if (this->m_FormatState)
	{
		if (FORMAT_OK == wParam)
		{
			//AfxMessageBox("U�̷����ɹ�");
			//�Ȳ����� this->m_FormatState = FALSE; ��Ϊд�뾵���ʱ����Ҫ�õ��ж�
		}
		else if(FORMAT_ERROR == wParam)
		{
			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
			((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("��ʼ����!"));
			this->m_FormatState = FALSE;
			if (this->m_needUnzip)		//����ʧ��,  �����Ҫ��ѹ, ֤��������������, ����ֹͣд�뾵��
				this->m_needUnzip = FALSE;
			AfxMessageBox("U�̷���ʧ��");
		}
	}
	
	if (this->m_needUnzip)
	{
		if (UNZIP_OK == wParam)
		{
			this->m_needUnzip = FALSE;
			this->m_FormatState = FALSE;
			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
			((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("��ʼ����!"));
			AfxMessageBox("����д�����--��ϲ, �󹦸��!");
		}
		else if (UNZIP_ERROR == wParam)
		{
			this->m_needUnzip = FALSE;
			this->m_FormatState = FALSE;
			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
			((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("��ʼ����!"));
			AfxMessageBox("����д��ʧ��");
		}
	}

	this->m_USB_ViewerDlg->GetUSB(this, IDC_COMBO1);			//����Ͽ��ϸ������ҵ���U��
	return 0;
}

INT CCreateStartDlg::GetDiskGeometry(HANDLE *hDevice, LPCSTR drive, LPCSTR disk, DISK_GEOMETRY* diskGeometry)
{
	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	if (hDevice && *hDevice != INVALID_HANDLE_VALUE)
	{
		m_hDevice = *hDevice;
	}
	else if (drive || disk)
	{
		if (!this->GetDiskHandle(drive, disk, &m_hDevice))
			return FALSE;
	}
	else
	{
		return FALSE;
	}

	//
	// ��ô��̽ṹ��Ϣ
	//
	DWORD dwBytes = 0;
	BOOL res = DeviceIoControl(m_hDevice,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,    // ������CTL_CODE macro��
		NULL,
		0,
		diskGeometry,
		sizeof(DISK_GEOMETRY),
		&dwBytes,
		NULL);
	
	if (!hDevice)
		CloseHandle(m_hDevice);

	return res;
}

INT CCreateStartDlg::InstallMBR(HANDLE *hDevice, LPCSTR drive, LPCSTR disk, DISK_GEOMETRY* diskGeometry)
{
	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	unsigned char* sector = NULL;
	if (hDevice && *hDevice != INVALID_HANDLE_VALUE)
	{
		m_hDevice = *hDevice;
	}
	else if (drive || disk)
	{
		if (!this->GetDiskHandle(drive, disk, &m_hDevice))
			return FALSE;
	}
	else
	{
		return FALSE;
	}

	DISK_GEOMETRY m_diskGeometry;
	if (!diskGeometry)
	{
		if (!this->GetDiskGeometry(&m_hDevice, NULL, NULL, &m_diskGeometry))
			goto ERROR1;
	}
	else
	{
		m_diskGeometry = *diskGeometry;
	}

	if (m_diskGeometry.BytesPerSector != (sizeof(mbr_data) / sizeof(mbr_data[0])))
		goto ERROR1;

	sector = new unsigned char[m_diskGeometry.BytesPerSector]();
	if (!this->GetDiskSector(sector, &m_hDevice, NULL, NULL, &m_diskGeometry))
		goto ERROR1;

	//��sector�еķ������Ƶ�mbr_data�ķ�����λ����
	memcpy_s(&mbr_data[sizeof(mbr_data) - 66], 66, &sector[m_diskGeometry.BytesPerSector - 66], 66);
	DWORD dwBytes = 0;
	SetFilePointer(m_hDevice, 0, 0, FILE_BEGIN);
	this->LockVolume(NULL, drive);
	WriteFile(m_hDevice, mbr_data, (sizeof(mbr_data) / sizeof(mbr_data[0])), &dwBytes, NULL);
	this->UnLockVolume();
	if (dwBytes == m_diskGeometry.BytesPerSector)
	{
		if (!hDevice)
			CloseHandle(m_hDevice);
		if (sector)
			delete[] sector;
		return TRUE;
	}
	else
		goto ERROR1;

ERROR1:
	if (!hDevice)
		CloseHandle(m_hDevice);
	if(sector)
		delete[] sector;
	return FALSE;
}

INT CCreateStartDlg::InstallPBR(
	HANDLE *hDevice, LPCSTR drive /*= NULL*/, LPCSTR disk /*= NULL*/, 
	BOOL use_hDrive /*= FALSE*/, DISK_GEOMETRY* diskGeometry /*= NULL*/)
{
	//use_hDrive ��ʾ �Ƿ�ʹ�� �����ľ�� ��������̾��, ��TRUEʱ��, ��Ҫ������Ч��drive,��hDevice ==NULL
	//ֻ�е���װPBR�ķ��� ��ϵͳ��ǰ��ʾ�ķ���, �Ұ�װ��PBR��NTFS_PBRʱ, ��[����]�õ�
	//��Ϊ������β���, ������������ʱ, д���NTFS_PBR��ʧ��, ��ϵͳò�Ʊ�����U����ʾ�����ĵ�һ����������֮��ļ�������
	//����ͨ��hDrive�����ľ�� ȴ����д��ɹ�.   ��Ȼ����FAT32_PBR==512Byte==��һ������������С, Ӧ��û������, ��˳��д����

	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	unsigned char* sector = NULL;

	if (hDevice && *hDevice != INVALID_HANDLE_VALUE)
	{
		m_hDevice = *hDevice;
	}
	else if (drive || disk)
	{
		if (!this->GetDiskHandle(drive, disk, &m_hDevice))
			return FALSE;
	}
	else
	{
		return FALSE;
	}

	DISK_GEOMETRY m_diskGeometry;
	if (!diskGeometry)
	{
		if (!this->GetDiskGeometry(&m_hDevice, NULL, NULL, &m_diskGeometry))
			goto ERROR1;
	}
	else
	{
		m_diskGeometry = *diskGeometry;
	}

	//U��������С��Ч�Լ��, ��ֻ֧��512�ֽڴ�С������U��
	if (m_diskGeometry.BytesPerSector != (sizeof(pbr_fat32_data) / sizeof(pbr_fat32_data[0])))
		goto ERROR1;

	//��û�����ǵ�m_ActivePartitionNum����
	INT m_ActivePartitionNum = this->GetActivePartitionNum(&m_hDevice, NULL, NULL);
	if (!m_ActivePartitionNum || NO_Active_Partition == m_ActivePartitionNum)
		goto ERROR1;

	//��û������type����, size��Ϣ
	Partition_Table list;		// = { 0 };
	if (!this->GetOnePartitionInfo(&list, &m_hDevice, m_ActivePartitionNum, &m_diskGeometry))
		goto ERROR1;
	
	//��û��������������
	sector = new unsigned char[m_diskGeometry.BytesPerSector]();
	if (!this->GetPartitionBootSector(sector, &m_hDevice, m_ActivePartitionNum, &m_diskGeometry))
		goto ERROR1;
	DWORD pbr_data_size = 0;
	unsigned char* pbr_data = NULL;
	if (strcmp(list.type, TEXT("ntfs")) == 0	\
		||strcmp(list.type, TEXT("NTFS")) == 0)
	{
		//�������(ntfs)�������������в���(ǰ84���ֽ�)���Ƶ�pbr_ntfs_data��Ӧλ����
		memcpy_s(&pbr_ntfs_data[0], 84, &sector[0], 84);
		pbr_data = pbr_ntfs_data;
		pbr_data_size = sizeof(pbr_ntfs_data);
	}
	else if (strcmp(list.type, TEXT("fat32")) == 0	\
		|| strcmp(list.type, TEXT("FAT32")) == 0)
	{
		//�������(fat32)�������������в���(ǰ90���ֽ�)���Ƶ�pbr_fat32_data��Ӧλ����
		memcpy_s(&pbr_fat32_data[0], 90, &sector[0], 90);
		pbr_data = pbr_fat32_data;
		pbr_data_size = sizeof(pbr_fat32_data);
	}
	else if (strcmp(list.type, TEXT("fat")) == 0	\
		|| strcmp(list.type, TEXT("FAT")) == 0	\
		|| strcmp(list.type, TEXT("exFAT")) == 0	\
		|| strcmp(list.type, TEXT("EXFAT")) == 0)
	{
		AfxMessageBox(TEXT("��ǸѾ~~~\n��ʱ����֧��fat��exfat�ļ�ϵͳ\n��ʱֻ֧��fat32��ntfs�ļ�ϵͳ\n����ѡ���Ƽ�ѡ��--��ʽ��������������"));
		goto ERROR1;
	}
	else
		goto ERROR1;

	//////////////////////////////////////////////////////////////////////////

	LARGE_INTEGER offset;
	DWORD dwBytes = 0;
	if (use_hDrive)
	{
		if (!drive)
			goto ERROR1;
		if (m_hDevice)
			CloseHandle(m_hDevice);
		if (!this->GetDriveHandle(drive, &m_hDevice))
			goto ERROR1;
		
		SetFilePointer(m_hDevice, 0, 0, FILE_BEGIN);
	}
	else
	{
		INT sector_size = 0;
		if (!(sector_size = this->GetDiskSector(sector, &m_hDevice, NULL, NULL, &m_diskGeometry)))
			goto ERROR1;

		DWORD sys_start_pos = 0;
		//��ȡ������Ŀ�ʼλ��
		memcpy_s(&sys_start_pos, sizeof(sys_start_pos), &sector[sector_size - 66 + 8 + (m_ActivePartitionNum - 1) * 16], 4);
		if (sys_start_pos <= 0)
			goto ERROR1;
		offset.QuadPart = (ULONG64)sys_start_pos * (ULONG64)m_diskGeometry.BytesPerSector;
		
		SetFilePointer(m_hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
	}

	this->LockVolume(NULL, drive);
	//д����������������
	WriteFile(m_hDevice, pbr_data, pbr_data_size, &dwBytes, NULL);
	this->UnLockVolume();
	if (dwBytes == dwBytes)
	{
		if (!hDevice)
			CloseHandle(m_hDevice);
		if (sector)
			delete[] sector;
		return TRUE;
	}
	else
	{
		goto ERROR1;
	}


ERROR1:
	if (!hDevice || use_hDrive)
		CloseHandle(m_hDevice);
	if (sector)
		delete[] sector;
	return FALSE;
}

INT CCreateStartDlg::LockVolume(HANDLE *hDisk, LPCSTR drive /*= NULL*/)
{
	if (hDisk && *hDisk != INVALID_HANDLE_VALUE)
		this->m_hVolume = *hDisk;
	else
	{
		if (!drive)
			return FALSE;

		CString str;
		str.Format("\\\\.\\%c:", *drive);
		this->m_hVolume = CreateFile(str.GetBuffer(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);
	}

	DWORD dwBytesReturned;
	BOOL res = DeviceIoControl(this->m_hVolume, 
		FSCTL_LOCK_VOLUME, 
		NULL, 0, NULL, 0, 
		&dwBytesReturned, NULL);
	return res;
}

INT CCreateStartDlg::DeleteFolder(LPCSTR path)
{
	//ɾ���ļ�,��G:\[BOOT], ���[BOOT]�ļ��м�������ļ���ɾ��
	CHAR m_path[MAX_PATH] = {0};
	strcpy_s(m_path, MAX_PATH, TEXT(path));

	SHFILEOPSTRUCT FileOp;
	ZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));

	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI;
	FileOp.hNameMappings = NULL;
	FileOp.hwnd = NULL;
	FileOp.lpszProgressTitle = NULL;
	FileOp.pFrom = m_path;
	FileOp.pTo = NULL;
	FileOp.wFunc = FO_DELETE;

	return  SHFileOperation(&FileOp) == 0;
}

INT CCreateStartDlg::UnLockVolume()
{
	if (this->m_hVolume == INVALID_HANDLE_VALUE)
		return FALSE;
	DWORD dwBytesReturned;
	BOOL res = DeviceIoControl(this->m_hVolume, 
		FSCTL_UNLOCK_VOLUME, 
		NULL, 0, NULL, 0, 
		&dwBytesReturned, NULL);

	CloseHandle(this->m_hVolume);
	this->m_hVolume = INVALID_HANDLE_VALUE;
	return res;
}

INT CCreateStartDlg::DismountVolume(LPCSTR drive /*= NULL*/)
{
	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	if (drive)
	{
		if (!this->GetDriveHandle(drive, &m_hDevice))
			return FALSE;
	}
	else
	{
		if (this->m_hVolume != INVALID_HANDLE_VALUE)
			m_hDevice = this->m_hVolume;
		else
			return FALSE;
	}

	DWORD dwBytesReturned;
	BOOL res = DeviceIoControl(m_hDevice, 
		FSCTL_DISMOUNT_VOLUME, 
		NULL, 0, NULL, 0, 
		&dwBytesReturned, NULL);

	if (drive && res)
		CloseHandle(m_hDevice);

	return res;
}

INT CCreateStartDlg::GetDriveHandle(LPCSTR drive, HANDLE *hDrive)
{
	if (!drive || !hDrive)
	{
		if (hDrive)
			*hDrive = INVALID_HANDLE_VALUE;

		return FALSE;
	}
	
	*hDrive = INVALID_HANDLE_VALUE;
	CString str;
	str.Format("\\\\.\\%c:", *drive);
	*hDrive = CreateFile(str.GetBuffer(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING, 0, NULL);
	
	return *hDrive != INVALID_HANDLE_VALUE ? TRUE : FALSE;
}

INT CCreateStartDlg::GetDiskNumber(LPCSTR drive, HANDLE *hDrive, INT* diskNumber)
{
	HANDLE m_hDrive = INVALID_HANDLE_VALUE;
	if (drive)
	{
		if (!this->GetDriveHandle(drive, &m_hDrive))
			return FALSE;
	}
	else if (hDrive)
	{
		m_hDrive = *hDrive;
	}
	else
		return FALSE;

	VOLUME_DISK_EXTENTS vde;
	DWORD dwBytes = 0;
	if (!DeviceIoControl(m_hDrive,
		IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
		NULL, 0,
		&vde, sizeof(VOLUME_DISK_EXTENTS),
		&dwBytes, NULL))
		return FALSE;
	
	if (!hDrive)
		CloseHandle(m_hDrive);

	if (diskNumber)
		*diskNumber = vde.Extents->DiskNumber;
	else
		return FALSE;

	return TRUE;
}

INT CCreateStartDlg::StartUnZip()
{
	union
	{
		INT(__stdcall CCreateStartDlg::*UnZip)();
		unsigned int(__stdcall* ThreadAdress)(void*);
	}ThreadFun;
	ThreadFun.UnZip = &CCreateStartDlg::UnZip;
	/*struct UnZipArg
	{
	CHAR desPath[MAX_PATH];
	CHAR exePath[MAX_PATH];
	CHAR isoPath[MAX_PATH];
	HWND msgWnd;
	};*/
	if (!this->m_UnZipArg)
		this->m_UnZipArg = new UnZipArg();		//new���г�ʼ��

	//this->m_UnZipArg->desPath =		//��CCreateStartDlg::OnBnClickedOk()��д
	//this->m_UnZipArg->exePath = ;		//�Ѿ�����ΪNULL, CCreateStartDlg::UnZip()�Ჹ��
	//this->m_UnZipArg->isoPath = ;		//��CCreateStartDlg::OnBnClickedOk()��д
	this->m_UnZipArg->msgWnd = this->GetSafeHwnd();

	if (0 >= _beginthreadex(NULL, 0, ThreadFun.ThreadAdress, this, 0, NULL))
		return FALSE;
	else
		return TRUE;
}

INT CCreateStartDlg::UnZip()
{
	if (this->m_FormatState)	//��Ҫ��ʽ��, ������  �Ƽ�ѡ��--��ʽ��������������
	{
		if (!this->RemountDrive(2, FALSE))	//����U�̵�2����
			goto ERROR2;
	}
	else	//��Ҫ��ʽ��, ������ ��������������������
	{
		if (!this->RemountDrive(1, FALSE))	//�൱�����¹���U��
			goto ERROR2;
	}

	if (!this->FreeFile(TEXT("7z.exe"), TEXT("UNZIP"), IDR_7z, NULL, FALSE))		//�ͷ�7z
		goto ERROR1;

	/*struct UnZipArg
	{
		CHAR desPath[MAX_PATH];		 == "g:" ��  "g:\" ����
		CHAR exePath[MAX_PATH];		 == "C:\Users\hello\LOCALS~1\TMP\"  ���� NULL
		CHAR isoPath[MAX_PATH];		 == "C:\Data\CA.iso"   �� NULL
		HWND msgWnd;
	};*/
	TCHAR m_exePath[MAX_PATH];
	TCHAR Parameters[MAX_PATH] = {0};
	TCHAR m_isoPath[MAX_PATH] = {0};
	if (*(this->m_UnZipArg->exePath))
	{
		strcpy_s(m_exePath, MAX_PATH, this->m_UnZipArg->exePath);
	}
	else
	{
		GetTempPath(MAX_PATH, m_exePath);
	}
	//strcat_s(m_exePath, MAX_PATH, _T("7z.exe"));		//7z·��

	if (*(this->m_UnZipArg->isoPath))
	{
		strcpy_s(m_isoPath, MAX_PATH, this->m_UnZipArg->isoPath);
	}
	else
	{
		GetCurrentDirectory(MAX_PATH, m_isoPath);		//��E:\Projects\test\test
		strcat_s(m_isoPath, MAX_PATH, "\\Data\\CA.iso");		//iso·��
	}

	sprintf_s(Parameters, _T("x \"%s\" -o\"%s\" -y"), m_isoPath, this->m_UnZipArg->desPath);		//Parameters����

	HANDLE hProcess = INVALID_HANDLE_VALUE;
	int res = this->ShellExe(TEXT("Open"), TEXT("7z.exe"), TEXT("%temp%"),
		Parameters, SW_HIDE, 1800000, &hProcess);		//30min == 1800s == 1800000ms 	�ú����������ͺ���

	if (WAIT_TIMEOUT == res || WAIT_FAILED == res)
	{
		//Ҳ����ֱ�ӹر�������̣�ֻҪ����sei.hProcess���� 
		TerminateProcess(hProcess, 0);
		goto ERROR1;
	}
	else if (WAIT_OBJECT_0 == res)
	{
		HANDLE hDrv = INVALID_HANDLE_VALUE;
		//����this->m_UnZipArg->desPath�ǳ�·��, this->GetDriveHandle()�����Ի�ȡ�÷����ľ��
		this->GetDriveHandle(this->m_UnZipArg->desPath, &hDrv);

		CString boolFolder(this->m_UnZipArg->desPath);
		if (boolFolder.Right(1).CompareNoCase(TEXT("\\")) == 0)
			boolFolder.Append(TEXT("[BOOT]"));
		else
			boolFolder.Append(TEXT("\\[BOOT]"));
		this->DeleteFolder(boolFolder.GetBuffer());

		::FlushFileBuffers(hDrv);		//ˢ�˻����е����ݵ�U��
		Sleep(2000);
		this->FreeFile(TEXT("7z.exe"), TEXT("UNZIP"), IDR_7z, NULL, TRUE);		//���7z.exe
		if (this->m_FormatState)	//��Ҫ��ʽ��, ������  �Ƽ�ѡ��--��ʽ��������������
			this->RemountDrive(2, TRUE);	//��ԭU�̷�������
		::PostMessage(this->m_UnZipArg->msgWnd, WM_MYMSG, UNZIP_OK, 0);
		return TRUE;
	}
	else
	{
		AfxMessageBox("UnZip()�ȴ���δ֪����?");
		goto ERROR1;
	}
/*

	SHELLEXECUTEINFO sei;
	// �������� 
	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = NULL;
	sei.lpVerb = TEXT("Open");
	sei.lpFile = TEXT("7z.exe");
	sei.lpParameters = Parameters;
	GetTempPath(MAX_PATH, m_exePath);
	sei.lpDirectory = m_exePath;
	sei.nShow = SW_HIDE;		//SW_HIDE;SW_NORMAL
	sei.hInstApp = NULL;
	ShellExecuteEx(&sei);
	//   �������������ǵȴ��ý��̽��� 
	int res = WaitForSingleObject(sei.hProcess, 1800000);	//30min == 1800s == 1800000ms*/
	


ERROR1:
	if (this->m_FormatState)	//��Ҫ��ʽ��, ������  �Ƽ�ѡ��--��ʽ��������������
		this->RemountDrive(2, TRUE);	//��ԭU�̷�������
	//else	//����Ҫ��ʽ��, ������ ��������������������
	//{	����Ҫ������
	//}

ERROR2:
	this->FreeFile(TEXT("7z.exe"), TEXT("UNZIP"), IDR_7z, NULL, TRUE);		//���7z.exe
	::PostMessage(this->m_UnZipArg->msgWnd, WM_MYMSG, UNZIP_ERROR, 0);
	return FALSE;
}

/*
INT CCreateStartDlg::Free7z(LPCSTR path / *= NULL* /, BOOL isDelete / *= FALSE* /)
{
	//����������������: �ͷ�7z.exe �� ɾ��7z.exe
	//path == "C:\Users\hello\LOCALS~1\TMP\" ��(��Ҫ\��׺)
	//��pathΪnull, ���Զ���дΪ%TMP%
	TCHAR m_path[MAX_PATH] = { 0 }; 
	if (path)
	{
		strcpy_s(m_path, MAX_PATH, path);
	}
	else
	{
		GetTempPath(MAX_PATH, m_path);		//���� %TMP% Ŀ¼, ��C:\Users\hello\LOCALS~1\TMP
	}
	strcat_s(m_path, MAX_PATH, _T("7z.exe"));

	if (isDelete)
		return ::DeleteFile(m_path);	//ɾ��7z.exe

	HMODULE hMod = GetModuleHandle(NULL);
	HRSRC hRsc = FindResource(hMod, MAKEINTRESOURCE(IDR_7z), _T("UNZIP"));
	HGLOBAL hGlobal = LoadResource(hMod, hRsc);
	LPCTSTR h7z = (LPCTSTR)LockResource(hGlobal);
	DWORD fileSize = SizeofResource(hMod, hRsc);

	HANDLE hfile = CreateFile(m_path,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, CREATE_ALWAYS, 0, NULL);

	DWORD dwBytes = 0;
	WriteFile(hfile, h7z, fileSize, &dwBytes, NULL);		//�ͷ�7z.exe�� ��ʱ�ļ���
	CloseHandle(hfile);
	if (dwBytes == fileSize && hfile)
		return TRUE;
	else
		return FALSE;
}*/

INT CCreateStartDlg::ShellExe(
	CHAR* operate, CHAR* file, CHAR* path, 
	CHAR* parameters, INT show, INT timeout, 
	_Out_ HANDLE* hProcess)
{
	//�ú����������ͺ���
	CHAR m_path[MAX_PATH + 1] = {0};
	if (!strcmp(path, TEXT("%temp%"))
		|| !strcmp(path, TEXT("%tmp%")))
	{
		GetTempPath(MAX_PATH, m_path);			//���� %TMP% Ŀ¼, ��C:\Users\hello\LOCALS~1\TMP
	}
	else if (!strcmp(path, TEXT("system32")))
	{
		GetWindowsDirectory(m_path, MAX_PATH + 1);		//ȡ��windowsĿ¼,��C:\Windows
		strcat_s(m_path, MAX_PATH + 1, "\\System32");
	}
	else
	{
		strcpy_s(m_path, MAX_PATH + 1, path);
	}

	SHELLEXECUTEINFO sei;
	// �������� 
	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = NULL;
	sei.lpVerb = TEXT(operate);
	sei.lpFile = TEXT(file);
	sei.lpParameters = parameters;
	sei.lpDirectory = m_path;
	sei.nShow = show;		//SW_HIDE;SW_NORMAL
	sei.hInstApp = NULL;
	ShellExecuteEx(&sei);
	//   �������������ǵȴ��ý��̽��� 
	*hProcess = sei.hProcess;
	return WaitForSingleObject(sei.hProcess, timeout);
}

INT CCreateStartDlg::FreeFile(CHAR* outName, CHAR* resType, INT resID, LPCSTR path /*= NULL*/, BOOL isDelete /*= FALSE*/)
{
	//����������������: �ͷ�resType���resID��path�µ�outName�ļ��� �� ɾ��path�µ�outName�ļ�
	//path == "C:\Users\hello\LOCALS~1\TMP\" ��(��Ҫ\��׺)
	//��pathΪnull, ���Զ���дΪ%TMP%

	TCHAR m_path[MAX_PATH] = { 0 };
	if (path)
	{
		strcpy_s(m_path, MAX_PATH, path);
	}
	else
	{
		GetTempPath(MAX_PATH, m_path);		//���� %TMP% Ŀ¼, ��C:\Users\hello\LOCALS~1\TMP
	}
	strcat_s(m_path, MAX_PATH, TEXT(outName));

	if (isDelete)
		return ::DeleteFile(m_path);	//ɾ���ļ�

	HMODULE hMod = GetModuleHandle(NULL);
	HRSRC hRsc = FindResource(hMod, MAKEINTRESOURCE(resID), TEXT(resType));
	HGLOBAL hGlobal = LoadResource(hMod, hRsc);
	LPCTSTR pfile = (LPCTSTR)LockResource(hGlobal);
	DWORD fileSize = SizeofResource(hMod, hRsc);

	HANDLE hfile = CreateFile(m_path,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, CREATE_ALWAYS, 0, NULL);

	DWORD dwBytes = 0;
	WriteFile(hfile, pfile, fileSize, &dwBytes, NULL);		//�ͷŵ� Ŀ���ļ���
	CloseHandle(hfile);
	return dwBytes == fileSize;
}
/*

INT CCreateStartDlg::FreeQemu(LPCSTR path = NULL, BOOL isDelete = FALSE)
{
	//����������������: �ͷ�MyQemu.exe �� ɾ��MyQemu.exe
	//path == "C:\Users\hello\LOCALS~1\TMP\" ��(��Ҫ\��׺)
	//��pathΪnull, ���Զ���дΪ%TMP%
	TCHAR m_path[MAX_PATH] = { 0 };
	if (path)
	{
		strcpy_s(m_path, MAX_PATH, path);
	}
	else
	{
		GetTempPath(MAX_PATH, m_path);		//���� %TMP% Ŀ¼, ��C:\Users\hello\LOCALS~1\TMP
	}
	strcat_s(m_path, MAX_PATH, _T("MyQemu.exe"));

	if (isDelete)
		return ::DeleteFile(m_path);	//ɾ��MyQemu.exe

	HMODULE hMod = GetModuleHandle(NULL);
	HRSRC hRsc = FindResource(hMod, MAKEINTRESOURCE(IDR_Qemu), _T("UNZIP"));
	HGLOBAL hGlobal = LoadResource(hMod, hRsc);
	LPCTSTR h7z = (LPCTSTR)LockResource(hGlobal);
	DWORD fileSize = SizeofResource(hMod, hRsc);

	HANDLE hfile = CreateFile(m_path,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, CREATE_ALWAYS, 0, NULL);

	DWORD dwBytes = 0;
	WriteFile(hfile, h7z, fileSize, &dwBytes, NULL);		//�ͷ�MyQemu.exe�� ��ʱ�ļ���
	CloseHandle(hfile);
	return dwBytes == fileSize;
}*/

INT CCreateStartDlg::RemountDrive(INT remountNum /*= 2*/, BOOL isRestore /*= FALSE*/, LPCSTR desPath /*= NULL*/)
{
	//������ʹϵͳ����U���е�remountNum������(1, 2, 3, 4)
	//isRestore��ʾ�Ƿ�U�̵ķ������������ԭ
	//һ���һ�ε�������remountNumֵ , �ڶ��ε��� ��������isRestoreֵ
	//desPath ���Ա���Ϊ"g:"��,�ﵽ���¹��ظ���
	CString order;
	switch (remountNum)
	{
	case 1:
		order.SetString("1234");
		break;
	case 2:
		order.SetString("2134");
		break;
	case 3:
		order.SetString("3214");
		break;
	case 4:
		order.SetString("4231");
		break;
	default:
		return FALSE;
	}
	
	CHAR m_desPath[MAX_PATH] = {0};
	if (desPath)
	{
		if (!*(desPath))
			return FALSE;

		strcpy_s(m_desPath, MAX_PATH, desPath);
	}
	else
	{
		if (!*(this->m_UnZipArg->desPath))
			return FALSE;

		strcpy_s(m_desPath, MAX_PATH, this->m_UnZipArg->desPath);
	}

	if (!this->ChangePartitionTable(NULL, m_desPath, NULL, order))
		return FALSE;
	if (!this->DismountVolume(m_desPath))
		return FALSE;
	if (!this->UpdateDisk(NULL, m_desPath, NULL))
		return FALSE;

	return TRUE;
}

LARGE_INTEGER CCreateStartDlg::GetUSBAllSize(LPCSTR drive)
{
	//����U�� ������  , �ֽ�����λ

	LARGE_INTEGER size = {0};
	DISK_GEOMETRY diskGeometry;
	if (!this->GetDiskGeometry(NULL, drive, NULL, &diskGeometry))
		return size;

	size.QuadPart = (ULONG64)diskGeometry.Cylinders.QuadPart * (ULONG64)diskGeometry.TracksPerCylinder *
		(ULONG64)diskGeometry.SectorsPerTrack * (ULONG64)diskGeometry.BytesPerSector;
	return size;
}

BOOL CCreateStartDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	//nEventType����WM_DEVICECHANGE��Ϣ��wParam�����������ֵ�ο�msdn
	switch (nEventType)
	{
	case 0x8000:		// DBT_DEVICEARRIVAL==0x8000		u�̲���
	case 0x8004:		//DBT_DEVICEREMOVECOMPLETE==0x8004	u�̰γ�
	{
		this->m_USB_ViewerDlg->GetUSB(this, IDC_COMBO1);	//���¼���U��
		return TRUE;
	}
	}
	return 0;
}

INT CCreateStartDlg::CheckMbrPbr(
	UCHAR sector[], INT sector_size, Partition_Table* list /*= NULL*/, 
	HANDLE *hDevice/* = NULL*/, LPCSTR drive /*= NULL*/, LPCSTR disk /*= NULL*/)
{
	INT res = FALSE;
	HANDLE m_hDevice = INVALID_HANDLE_VALUE;
	DISK_GEOMETRY diskGeometry = {0};
	UCHAR* m_sector = NULL;
	Partition_Table temp_list = { 0 };


	if (sector_size > 0 && sector)
	{
		m_sector = sector;
	}
	else if(hDevice || drive || disk)
	{
		if (hDevice)
			m_hDevice = *hDevice;
		else
		{
			if (!this->GetDiskHandle(drive, disk, &m_hDevice))
				goto FINAL;
		}
		if (!this->GetDiskGeometry(&m_hDevice, NULL, NULL, &diskGeometry))
			goto FINAL;
		m_sector = new UCHAR[diskGeometry.BytesPerSector]();
		if (!(sector_size = this->GetDiskSector(m_sector, &m_hDevice, NULL, NULL, &diskGeometry)))
			goto FINAL;
	}
	else
		goto FINAL;


	res = NO_MBR_PBR;
	//���ħ�� 55AA
	if (m_sector[sector_size - 2] != 0x55
		|| m_sector[sector_size - 1] != 0xAA)
	{
		res = NO_MBR_PBR;
		goto FINAL;
	}

	//�����Ƿ�Ϊmbr
	BOOL isMBR = TRUE;
	for (int i = 0; i < 4; i++)
	{
		//���������־λ
		if (m_sector[sector_size - 66 + i * 16] != 0x80
			&& m_sector[sector_size - 66 + i * 16] != 0x00)
		{
			isMBR = FALSE;
			break;
		}
	}
	if (isMBR)
	{
		res = MBR;		//��MBR����
		goto FINAL;
	}
	
	if (0 >= diskGeometry.BytesPerSector)
		diskGeometry.BytesPerSector = sector_size;
	//������ ֻ��pbr�������
	if (!this->GetOnePartitionInfo(&temp_list, NULL, NULL, &diskGeometry, m_sector, sector_size))
	{
		//GetOnePartitionInfo()����false, ��m_sectorҲ����pbr
		res = NO_MBR_PBR;
		goto FINAL;
	}

	if (strcmp(temp_list.type, TEXT("NTFS")) == 0)
		res = PBR_NTFS;
	else if (strcmp(temp_list.type, TEXT("FAT32")) == 0)
		res = PBR_FAT32;
	else if (strcmp(temp_list.type, TEXT("FAT")) == 0)
		res = PBR_FAT;
	else if (strcmp(temp_list.type, TEXT("exFAT")) == 0)
		res = PBR_EXFAT;
	else
	{
		res = NO_MBR_PBR;
		goto FINAL;
	}
	if (list)
		memcpy_s(list, sizeof(temp_list), &temp_list, sizeof(temp_list));
	goto FINAL;
	/*//����Ƿ�ΪNTFS_PBR
	CHAR name[6] = { 0 };
	memcpy_s(name, sizeof(name), &m_sector[3], 4);		//m_sector[3,4,5,6]��NTFS��File system ID ("NTFS")
	//�жϷ������ļ�ϵͳ
	if (strcmp(name, TEXT("NTFS")) == 0)
	{
		res = PBR_NTFS;
	}
	else
	{
		//����Ƿ�ΪFAT32_PBR
		ZeroMemory(name, sizeof(name));
		memcpy_s(name, sizeof(name), &m_sector[82], 5);		//m_sector[82,83,84,85,86]��FAT32��File system��־λ("FAT32")
		if (strcmp(name, TEXT("FAT32")) == 0)
			res = PBR_FAT32;
		else
		{
			res = NO_MBR_PBR;
			goto FINAL;
		}
	}

	//�������Ҫ��, ���÷�����  �ļ�ϵͳtype �� ��Сsize ����
	if (list)
	{
		DWORD partitionSize = 0;
		if (PBR_NTFS == res)
		{
			memcpy_s(&partitionSize, sizeof(DWORD), &m_sector[40], 4);		//ntfs���������� �����ܹ����� ��־λ
			if (partitionSize <= 0)
			{
				res = NO_MBR_PBR;
				goto FINAL;
			}
			else
				partitionSize++;		//��Ϊntfs�� �����ܹ����� �������˵�һ������������ , ��fat32����û�����ٵ�

			strcpy_s(list->type, sizeof(list->type), TEXT("NTFS"));
			list->size = ULONG64((ULONG64)partitionSize * (ULONG64)sector_size / (1024.0 * 1024.0));
			//û��diskGeometry->BytesPerSector, ����sector_size����
		}
		else if (PBR_FAT32 == res)
		{
			memcpy_s(&partitionSize, sizeof(DWORD), &m_sector[32], 4);		//fat32���������� �����ܹ����� ��־λ	Sectors (on large volumes)
			if (partitionSize <= 0)
			{
				res = NO_MBR_PBR;
				goto FINAL;
			}

			strcpy_s(list->type, sizeof(list->type), TEXT("FAT32"));
			list->size = ULONG64((ULONG64)partitionSize * (ULONG64)sector_size / (1024.0 * 1024.0));
			//û��diskGeometry->BytesPerSector, ����sector_size����
		}
		else
		{
			res = NO_MBR_PBR;
			goto FINAL;
		}
	}
	goto FINAL;		//������PBR_FAT32 ���� PBR_NTFS
	*/

FINAL:
	if (!hDevice)
		CloseHandle(m_hDevice);		//CloseHandle()�Ѿ���m_hDevice == 0xfffff �з�����
	if (sector_size <= 0 || !sector)
	{
		if (m_sector)
			delete[] m_sector;
	}

	return res;
}

INT CCreateStartDlg::SetMyTimer(HWND hWnd, UINT timerID, UINT time, BOOL isKill /*= FALSE*/)
{
	//time ��΢������λ
	if (!isKill)
	{
		return ::SetTimer(hWnd, timerID, time, NULL);
	}
	else
	{
		return ::KillTimer(hWnd, timerID);
	}
}

void CCreateStartDlg::OnTimer(UINT_PTR nIDEvent)
{
	static int num = 1;
	switch (num)
	{
	case 1:
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT(" ����ing."));
		break;
	case 2:
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("  ����ing.."));
		break;
	case 3:
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("   ����ing..."));
		break;
	case 4:
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("����ing"));
		break;
	}
	num = ++num > 4 ? 1 : num;
	
	CDialogEx::OnTimer(nIDEvent);
}


void CCreateStartDlg::OnBnClickedCancel()
{
	if (this->m_FormatState || this->m_needUnzip)
	{
		AfxMessageBox(TEXT("����������ing, ���Ժ�"));
		return;
	}

	CDialogEx::OnCancel();
}


void CCreateStartDlg::OnBnClickedBoot()
{
	if (this->m_FormatState || this->m_needUnzip)
	{
		AfxMessageBox(TEXT("����������ing, ���Ժ�"));
		return;
	}
	CString drive;
	((CComboBox*)GetDlgItem(IDC_COMBO1))->GetWindowText(drive);
	//��ΪU�̵���ʾ��ʽ��"G:\ 16G"
	if (drive.Find(":\\") == -1)
	{
		AfxMessageBox(TEXT("��ѡ��U�̲��ܽ��в�����"));
		return;		//���  �հ׵�ѡ��
	}

	union
	{
		INT(__stdcall CCreateStartDlg::*BootPE)();
		unsigned int(__stdcall* ThreadAdress)(void*);
	}ThreadFun;
	ThreadFun.BootPE = &CCreateStartDlg::BootPE;

	if (0 >= _beginthreadex(NULL, 0, ThreadFun.ThreadAdress, this, 0, NULL))
	{
		AfxMessageBox(TEXT("ģ������������������Ŷ T_T\n\n������˼��, ��Ҫ��ȥ������"));
		return;
	}
}

INT __stdcall CCreateStartDlg::BootPE()
{
	CString drive;
	((CComboBox*)GetDlgItem(IDC_COMBO1))->GetWindowText(drive);
	INT pos = drive.Find(":\\");	//��ΪU�̵���ʾ��ʽ��"G:\ 16G"
	if (pos == -1)
	{
		AfxMessageBox(TEXT("��ѡ��U�̲��ܽ��в�����"));
		return FALSE;		//���  �հ׵�ѡ��
	}
	drive = drive.Mid(pos - 1, 2);		//����str��ʾ"G:"

	AfxMessageBox(TEXT("����: ��Ctrl + Alt����������ģ�����з��ص���ʵϵͳ��^_^\n\nģ����ֻ��������֤�ܷ�����U�̵�ϵͳ\n\n������ģ������Ĺ���, ���ܻᵼ����ʵϵͳ�Ĳ��ȶ�Ӵ~~~"));

	LPCSTR fileName[] =
	{	//MyQemu��ѹ�����������ļ�
		"MyQemu.exe",
		"qemu.exe",
		"bios.bin",
		"libusb0.dll",
		"libz-1.dll",
		"SDL.dll",
		"vgabios.bin"
	};

	CString str(TEXT("x -y"));		//��Ϊ����, ��ʾ ����������ԭ�ļ��Ľ�ѹ
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	INT diskNumber = -1;

	if (!this->GetDiskNumber(drive.GetBuffer(), NULL, &diskNumber))
		goto ERROR1;

	if (!this->FreeFile(TEXT("MyQemu.exe"), TEXT("UNZIP"), IDR_Qemu, NULL, FALSE))		//�ͷ�MyQemu.exe
		goto ERROR1;

	int res = this->ShellExe(TEXT("Open"), TEXT("MyQemu.exe"), TEXT("%temp%"),
		str.GetBuffer(), SW_HIDE, 5000, &hProcess);		//5s == 5000ms ������������ѹ	�ú����������ͺ���

														//ȷ��qemu����
	str.Format(TEXT("-L . -boot c -m 300 -hda //./PhysicalDrive%d -localtime -vga std -snapshot"), diskNumber);

	res = this->ShellExe(TEXT("Open"), TEXT("qemu.exe"), TEXT("%temp%"),
		str.GetBuffer(), SW_HIDE, 1800000, &hProcess);		//30min == 1800s == 1800000ms ���ڿ�ʼ����ģ����	�ú����������ͺ���
	if (WAIT_OBJECT_0 == res)
	{
		for (INT i = sizeof(fileName) / sizeof(fileName[0]); i; )
		{
			//�����ļ�
			this->FreeFile((CHAR*)(fileName[--i]), NULL, NULL, NULL, TRUE);
		}
		return TRUE;
	}
	else if (WAIT_TIMEOUT == res || WAIT_FAILED == res)
	{
		//Ҳ����ֱ�ӹر�������̣�ֻҪ����sei.hProcess���� 
		TerminateProcess(hProcess, 0);
		goto ERROR1;
	}
	else
	{
		goto ERROR1;
	}


ERROR1:
	for (INT i = sizeof(fileName) / sizeof(fileName[0]); i; )
	{
		//�����ļ�
		this->FreeFile((CHAR*)(fileName[--i]), NULL, NULL, NULL, TRUE);
	}
	AfxMessageBox(TEXT("ģ������������������Ŷ T_T\n\n������˼��, ��Ҫ��ȥ������"));
	return FALSE;
}


