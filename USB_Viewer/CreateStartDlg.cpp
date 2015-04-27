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

	// TODO:  在此添加额外的初始化代码
	this->m_USB_ViewerDlg->GetUSB(this, IDC_COMBO1);			//在组合框上显示所找到的U盘
	((CButton*)this->GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);		//单选框 设置为 推荐选项--格式化再做启动盘

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
BEGIN_MESSAGE_MAP(CCreateStartDlg, CDialogEx)
ON_BN_CLICKED(IDOK, &CCreateStartDlg::OnBnClickedOk)
ON_MESSAGE(WM_MYMSG, &CCreateStartDlg::OnGetResult)
ON_WM_DEVICECHANGE()		//用于接收设备(U盘)变动, 已更新界面的消息
ON_WM_TIMER()
ON_BN_CLICKED(IDCANCEL, &CCreateStartDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDC_BOOT, &CCreateStartDlg::OnBnClickedBoot)
END_MESSAGE_MAP()


void CCreateStartDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	if (this->m_FormatState || this->m_needUnzip)
	{
		AfxMessageBox(TEXT("制作启动盘ing, 请稍后"));
		return;
	}

	CString str;
	CComboBox *m_CComboBox = ((CComboBox*)GetDlgItem(IDC_COMBO1));
	m_CComboBox->GetWindowText(str);
	INT pos = str.Find(":\\");	//因为U盘的显示格式是"G:\ 16G"
	if (pos == -1)
	{
		AfxMessageBox(TEXT("先选择U盘才能进行操作哈"));
		return;		//针对  空白的选项
	}
	str = str.Mid(pos - 1, 2);		//现在str表示"G:"

	CHAR m_isoPath[MAX_PATH] = {0};	
	GetCurrentDirectory(MAX_PATH, m_isoPath);		//如E:\Projects\test\test

	//声明顺序决定搜索PE镜像的顺序, 找到第一个存在镜像则退出循环
	CHAR* tempPath[4] =
	{
		TEXT("\\Data\\计算机协会PE.ISO"),
		TEXT("\\计算机协会PE.ISO"),
		TEXT("\\Data\\CA.iso"),
		TEXT("\\CA.iso"),
	};
	HANDLE hfile = INVALID_HANDLE_VALUE;
	for (int i = 0; i < 4; i++)
	{
		strcat_s(m_isoPath, MAX_PATH, tempPath[i]);		//m_isoPath是iso路径

		hfile = CreateFile(TEXT(m_isoPath),
			GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL);

		if (INVALID_HANDLE_VALUE != hfile)
			break;
		m_isoPath[0] = '0';								//为使strcat_s()工作正常,不叠加上一次的路径
		GetCurrentDirectory(MAX_PATH, m_isoPath);		//如E:\Projects\test\test;
	}
	
	if (INVALID_HANDLE_VALUE == hfile)
	{
		CloseHandle(hfile);
		AfxMessageBox(TEXT("找不到镜像文件, 不能写入 T_T"));
		return;
	}
	LARGE_INTEGER fileSize = {0};
	GetFileSizeEx(hfile, &fileSize);		//获得iso文件的大小, 字节Byte做单位

	LARGE_INTEGER usbSize = this->GetUSBAllSize(TEXT(str.GetBuffer()));		//获得u盘的总大小, 字节Byte做单位
	if (!fileSize.QuadPart || !usbSize.QuadPart)
	{
		CloseHandle(hfile);
		AfxMessageBox(TEXT("镜像文件 大小 或 U盘容量 异常"));
		return;
	}
	
	
	//根据 选择 的 不同制作启动盘, 进行相应的判断
	if (((CButton*)this->GetDlgItem(IDC_RADIO1))->GetCheck())		//单选框 设置为 推荐选项--格式化再做启动盘
	{
		if (!this->m_FormatState && !this->m_needUnzip )
		{
			if (fileSize.QuadPart >= usbSize.QuadPart)
			{
				CloseHandle(hfile);
				AfxMessageBox(TEXT("U盘容量不足, 不够写入镜像文件咧"));
				return;
			}

			fileSize.QuadPart = (fileSize.QuadPart / (1024 * 1024)) + 1;	//将iso文件大小转换为MB, +1是保险起见

			LARGE_INTEGER fat32size, ntfsSize;			//分别表示fat32, ntfs分区的大小
			//保险起见, 分配比iso文件大50M的fat32分区
			//iso低于fat32最低大小要求, 就分配最低大小+100的大小
			fileSize.QuadPart >= FAT32_SIZE_LIMIT ?			\
				fat32size.QuadPart = fileSize.QuadPart + 100 : fat32size.QuadPart = FAT32_SIZE_LIMIT + 100;
				
			usbSize.QuadPart = (usbSize.QuadPart / (1024 * 1024)) + 1;	//将U盘总大小转换为MB, +1是保险起见
			ntfsSize.QuadPart = usbSize.QuadPart - fat32size.QuadPart;		//剩下的U盘容量分配给ntfs分区
			Partition_Table table[] =
			{
				{ "ntfs", (ULONG64)ntfsSize.QuadPart },//{"ntfs", 1024 },		//1024MB == 2097152扇区	2097152
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
			
			//填写 解压 参数
			strcpy_s(this->m_UnZipArg->desPath, MAX_PATH, TEXT(str.GetBuffer()));
			strcpy_s(this->m_UnZipArg->isoPath, MAX_PATH, TEXT(m_isoPath));
			
			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, FALSE);		//开始文字动态效果

			if (!this->Partition(TEXT(str.GetBuffer()), sizeof(table) / sizeof(Partition_Table), table, 2,
				this->GetSafeHwnd(), NULL))
			{
				//若分区失败, 取消文字动态效果
				this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
				((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("开始制作!"));
				this->m_FormatState = FALSE;
				this->m_needUnzip = FALSE;
			}
	
		}
		else
		{
			CloseHandle(hfile);
			AfxMessageBox(TEXT("制作启动盘ing哈, 请稍后~"));
			return;
		}

	}
	else if (((CButton*)this->GetDlgItem(IDC_RADIO2))->GetCheck())		//单选框 设置为 直接做启动盘(不格式化)
	{
		HANDLE hDevice = INVALID_HANDLE_VALUE;
		DISK_GEOMETRY diskGeometry;
		
		if (!this->m_needUnzip)
		{
			if (!this->GetDiskHandle(str.GetBuffer(), NULL, &hDevice))
			{
				CloseHandle(hfile);
				//对于U盘第一个扇区就是pbr的分区, 是不能做成启动盘的
				AfxMessageBox(TEXT("很遗憾, 读取U盘操作失败了T_T\n建议 选择 推荐选项--格式化再做启动盘"));
				return;
			}
			if (!this->GetDiskGeometry(&hDevice, NULL, NULL, &diskGeometry))
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				//对于U盘第一个扇区就是pbr的分区, 是不能做成启动盘的
				AfxMessageBox(TEXT("很遗憾, 读取U盘操作失败了T_T\n建议 选择 推荐选项--格式化再做启动盘"));
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
				//对于U盘第一个扇区就是pbr的分区, 是不能做成启动盘的
				AfxMessageBox(TEXT("很遗憾, U盘不符合 保留数据制作启动盘 条件T_T\n建议 选择 推荐选项--格式化再做启动盘"));
				return;
			}
			else if (FALSE == sector_type)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("很遗憾, 读取U盘信息出错了"));
				return;
			}

			//即 MBR == sector_type
			//既然符合制作启动盘条件, 则检查可用空间是否足够
			unsigned long long i64FreeBytesToCaller = 0;
			unsigned long long i64TotalBytes = 0;

			GetDiskFreeSpaceEx(TEXT(str.GetBuffer()),
				(PULARGE_INTEGER)&i64FreeBytesToCaller,
				(PULARGE_INTEGER)&i64TotalBytes,
				NULL);
			//都是用  字节 做单位      10MB==10485760.0字节  让可用空间比镜像大小>10M, 保险点
			if (unsigned long long(fileSize.QuadPart + (LONGLONG)10485760.0) >= i64FreeBytesToCaller)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("呵呵, 你的U盘的 可用空间 不足哦~~\n建议, 选择格式化制作U盘, 或者请先清出足够空间再继续^_^"));
				return;
			}

			//开始制作启动盘, 不格式化
			this->m_needUnzip = TRUE;
			this->m_FormatState = FALSE;
			//设置 分区表第一个有效项 为活动分区
			INT res_active = this->SetActivePartitionNum(5, &hDevice, NULL, NULL);
			if (-1 == res_active)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("很遗憾, U盘的分区是 不支持制作启动盘 的 扩展分区\n建议 选择 推荐选项--格式化再做启动盘"));
				this->m_needUnzip = FALSE;
				return;
			}
			else if(FALSE == res_active)
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("很遗憾, 设置活动分区出错了T_T"));
				this->m_needUnzip = FALSE;
				return;
			}
			
			//开始写入mbr,pbr,镜像文件
			//AfxMessageBox("即将安装活动分区的mbr和 pbr扇区");
			if (!this->InstallMBR(&hDevice, NULL, NULL, &diskGeometry))		//安装MBR扇区
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("很遗憾, 写入MBR扇区出错了T_T"));
				this->m_needUnzip = FALSE;
				return;
			}
			if (!this->InstallPBR(NULL, str.GetBuffer(), NULL, TRUE,&diskGeometry))		//安装活动分区的pbr扇区
			{
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("很遗憾, 写入PBR扇区出错了T_T"));
				this->m_needUnzip = FALSE;
				return;
			}
			
			//填写 写入镜像的  参数
			if (!this->m_UnZipArg)
				this->m_UnZipArg = new UnZipArg();
			else
				ZeroMemory(this->m_UnZipArg, sizeof(*(this->m_UnZipArg)));

			//填写 解压 参数
			strcpy_s(this->m_UnZipArg->desPath, MAX_PATH, TEXT(str.GetBuffer()));
			strcpy_s(this->m_UnZipArg->isoPath, MAX_PATH, TEXT(m_isoPath));

			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, FALSE);		//开始文字动态效果

			if (!this->StartUnZip())	//开始写入镜像(多线程的), 所以要先写入mbr,pbr扇区
			{
				//若解压失败, 取消文字动态效果
				this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
				((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("开始制作!"));
				CloseHandle(hDevice);
				CloseHandle(hfile);
				AfxMessageBox(TEXT("很遗憾, 写入镜像出错了T_T"));
				this->m_needUnzip = FALSE;
				return;
			}

			//执行到这里 表示,都成功, 故清理工作
			CloseHandle(hDevice);
			CloseHandle(hfile);
			return;
		}
		else
		{
			AfxMessageBox(TEXT("写入U盘ing咧, 请稍后哈~"));
			return;
		}

	}
	else
	{
		CloseHandle(hfile);
		AfxMessageBox(TEXT("请选择 制作启动盘的方式 哈^_^"));		//即选择单选框
		return;
	}
}

#include <winioctl.h>
INT CCreateStartDlg::Partition(TCHAR* drive, INT num, Partition_Table* list, INT activeNum, HWND msgWnd, LPCSTR eventName)
{
	//num是list数组中有效元素的个数
	//drive格式"G:"
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
		sector = new UCHAR[dwSize]();
		if (sector)
		{
			SetFilePointer(hDrv, 0, 0, FILE_BEGIN);
			//获取U盘第一个扇区, 即mbr扇区
			ReadFile(hDrv, sector, dwSize, &dwBytes, NULL);
			if (dwBytes == dwSize)
			{
				//判断mbr扇区是不是分区的pbr扇区,  下面有用
				INT type = this->CheckMbrPbr(sector, dwSize, NULL, NULL, NULL, NULL);

				this->CreatePartitionTable(sector, dwSize, num, list, activeNum, &diskGeometry);

				dwBytes = 0;
				SetFilePointer(hDrv, 0, NULL, FILE_BEGIN);
				this->LockVolume(NULL, drive);
				//写入新的分区表
				WriteFile(hDrv, sector, dwSize, &dwBytes, NULL);
				this->UnLockVolume();
				if (dwBytes == dwSize)
				{
					//AfxMessageBox(_T(" U盘分区表设置成功!"));
					if (type && type != MBR)	//即当第一个扇区是pbr扇区时
					{
						//如果mbr扇区就是分区pbr扇区的话, 通过改变分区表来分区的办法就会失败, 所以要变mbr扇区为非pbr扇区
						if (!this->InstallMBR(&hDrv, NULL, NULL, &diskGeometry))		//安装MBR扇区
							goto ERROR2;
					}
					//清零分区表对应分区的引导扇区		初步解决了  RemountDrive()时候, 弹出window的格式化U盘窗口
					this->LockVolume(NULL, drive);
					INT deleteRes = this->DeletePartitionBootSector(&hDrv, num, sector, dwSize, &diskGeometry);	
					this->UnLockVolume();
					if (!this->RemountDrive(1, FALSE, drive))
						goto ERROR2;
					if (!deleteRes)
					{
						this->LockVolume(NULL, drive);
						//清零分区表对应分区的引导扇区		初步解决了  RemountDrive()时候, 弹出window的格式化U盘窗口
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

	//这里不要CloseHandle(hDrv);   由子线程 CCreateStartDlg::FormatPartition()代为执行
	//如果收到	WM_MYMSG(FORMAT_OK)	则AfxMessageBox("U盘分区成功");
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
	AfxMessageBox("U盘分区失败(还没创建子线程)");
	return FALSE;

}

INT __stdcall CCreateStartDlg::FormatPartition()
{
	//drive格式"G:"
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
				str.GetBuffer(), SW_HIDE, 40000, &hProcess);		//40秒 == 40000ms 	该函数是阻塞型函数
		
		if (WAIT_TIMEOUT == res || WAIT_FAILED == res)
		{
			//也可以直接关闭这个进程，只要保留sei.hProcess就行 
			TerminateProcess(hProcess, 0);
			goto ERROR2;
		}
		else if (WAIT_OBJECT_0 == res)
		{
			//AfxMessageBox("格式化完,即将更新");
			Sleep(1000);
			//if (!this->BackupPartitionBootSector(&m_hDrv, i + 1, &m_diskGeometry))
			//goto ERROR2;

			if (this->ChangePartitionTable(&m_hDrv, this->m_FormatPartitionArg->drive, NULL, order[i]))		//采取更改分区表项目顺序做法, 
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
			AfxMessageBox("WaitForSingleObject 新返回值");
			goto ERROR2;
		}

/*
		SHELLEXECUTEINFO sei;
		// 启动进程 
		ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
		TCHAR Path[MAX_PATH + 1];//TCHAR取代char  MAX_PATH最长路径
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.hwnd = NULL;
		sei.lpVerb = NULL;
		sei.lpFile = "format.com";	//"cmd.exe ";		"format"
		GetWindowsDirectory(Path, MAX_PATH);	//取得windows目录
		strcat_s(Path, MAX_PATH, "\\System32");
		sei.lpParameters = str;
		sei.lpDirectory = Path;
		sei.nShow = SW_HIDE;//SW_HIDE;SW_NORMAL
		sei.hInstApp = NULL;
		ShellExecuteEx(&sei);
		//   加入下面这句就是等待该进程结束 
		int res = WaitForSingleObject(sei.hProcess, 40000);		//40秒 == 40000ms 
	*/
	}
	//AfxMessageBox("即将恢复引导扇区");
	//if (!this->RestorePartitionBootSector(&m_hDrv, this->m_FormatPartitionArg->num, &m_diskGeometry))
	//	goto ERROR2;
	if (this->m_needUnzip)
	{
		//如果需要解压, 证明本次分区操作是制作启动盘操作的一部分, 故写入mbr,pbr,镜像文件
		//AfxMessageBox("即将安装活动分区的mbr和 pbr扇区");
		if (!this->InstallMBR(&m_hDrv, NULL, NULL, &m_diskGeometry))		//安装MBR扇区
			goto ERROR2;
		if (!this->InstallPBR(&m_hDrv, NULL, NULL, FALSE, &m_diskGeometry))		//安装活动分区的pbr扇区
			goto ERROR2;
		if (!this->UpdateDisk(&m_hDrv, NULL, NULL))
			goto ERROR2;
		//AfxMessageBox("即将开始写入镜像.");
		if(!this->StartUnZip())
			goto ERROR2;
	}


	if (this->m_FormatPartitionArg->msgWnd)
	{
		//成功分区, 将好消息传递出去
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
	//如if (!this->m_FormatPartitionArg->hDrv)	给主线程代为CloseHandle(m_hDrv);
	CloseHandle(m_hDrv);
	
	return TRUE;


ERROR2:
	::FlushFileBuffers(m_hDrv);
	//if (!this->m_FormatPartitionArg->hDrv)	给主线程代为CloseHandle(m_hDrv);
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
	//partitionNum 从1 开始, 表示U盘上第几个分区 , 1 , 2, 3, 4
	//BackupPartitionBootSector本函数只是将sector中分区表的第一项对应的U盘分区的第一个引导扇区备份到this->m_PartitionBootSector[partitionNum - 1]中,
	//并且将该分区的第一个引导扇区清空

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
	LARGE_INTEGER offset;		// 有符号的64位整型表示
	UCHAR* newBootSector = new UCHAR[m_diskGeometry.BytesPerSector]();

	memcpy_s(&sys_start_pos, 4, &sector[sector_size - 66 + 8], 4);		//分区前预留扇区, 即分区开始扇区

	offset.QuadPart = ((ULONG64)sys_start_pos) * ((ULONG64)m_diskGeometry.BytesPerSector);
	// 从打开的文件（磁盘）中移动文件指针。offset.LowPart低32位为移动字节数
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
	//num表示分区表里面的有效项个数, 1,2, 3, 4
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
	LARGE_INTEGER offset;		// 有符号的64位整型表示
	for (int i = 0; i < num; i++)
	{
		memcpy_s(&sys_start_pos[i], 4, &sector[sector_size - 66 + 8 + i * 16], 4);		//分区前预留扇区, 即分区开始扇区

		offset.QuadPart = ((ULONG64)sys_start_pos[i])* ((ULONG64)m_diskGeometry.BytesPerSector);
		// 从打开的文件（磁盘）中移动文件指针。offset.LowPart低32位为移动字节数
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
	//partitionNum表示要获取的引导扇区是 分区表第partitionNum个分区的, 设为 1,2, 3, 4
	if (partitionNum > 4 || partitionNum <= 0 || !hDevice || !diskGeometry || !sector)
		return FALSE;

	INT sector_size = 0;
	if (!(sector_size = this->GetDiskSector(sector, hDevice, NULL, NULL, diskGeometry)))
		return FALSE;
	
	INT sector_type = this->CheckMbrPbr(sector, sector_size, NULL);	//有可能返回false
	if (NO_MBR_PBR == sector_type || !sector_type)
		return FALSE;
	else if (PBR_NTFS == sector_type 
		|| PBR_FAT32 == sector_type
		|| PBR_FAT == sector_type
		|| PBR_EXFAT == sector_type)
	{
		if (1 == partitionNum)	//如果pbr是第一个扇区, 则肯定U盘只有一个分区,也就是第一个分区
			return sector_size;
		else
			return FALSE;
	}
	
	//下面的 即MBR == sector_type
	DWORD sys_start_pos = 0;
	memcpy_s(&sys_start_pos, 4, &sector[sector_size - 66 + 8 + (partitionNum - 1) * 16], 4);		//分区前预留扇区, 即分区开始扇区
	if (sys_start_pos <= 0)		//对分区前预留扇区标志位做有效性检测
		return FALSE;

	LARGE_INTEGER offset;
	DWORD dwBytes = 0;
	offset.QuadPart = (ULONG64)sys_start_pos * (ULONG64)diskGeometry->BytesPerSector;		// 有符号的64位整型表示 
	SetFilePointer(*hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
	ReadFile(*hDevice, sector, diskGeometry->BytesPerSector, &dwBytes, NULL);		//读取活动分区第一个引导扇区
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
	//partitionNum为 分区表第几项 , 即第几个分区(无论是否有效), 如1, 2,3 4
	//若bootSector有效,则只能是文件系统的引导扇区,本函数将分析该扇区
	//且hDevice优先级比bootSector高

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
	memcpy_s(name, sizeof(name), &sector[3], 4);		//sector[3,4,5,6]是NTFS的File system ID ("NTFS")
	
	//判断分区的文件系统
	if (strcmp(name, TEXT("NTFS")) == 0)
	{
		type = NTFS;
	}
	else
	{
		ZeroMemory(name, sizeof(name));
		memcpy_s(name, sizeof(name), &sector[82], 5);		//sector[82,83,84,85,86]是FAT32的File system标志位("FAT32")
		if (strcmp(name, TEXT("FAT32")) == 0)
			type = FAT32;
		else
		{
			ZeroMemory(name, sizeof(name));
			memcpy_s(name, sizeof(name), &sector[54], 5);		//sector是FAT16的File system标志位("FAT16")
			if (strcmp(name, TEXT("FAT16")) == 0)
				type = FAT16;
			else
			{
				ZeroMemory(name, sizeof(name));
				memcpy_s(name, sizeof(name), &sector[3], 5);		//sector是exFAT的File system标志位("EXFAT")
				if (strcmp(name, TEXT("EXFAT")) == 0)
					type = exFAT;
				else
					goto FINAL;		//type = UNKNOW;
			}
				
		}
	}

	//设置分区的  文件系统type 和 大小size 参数
	DWORD partitionSize = 0;
	if (NTFS == type)
	{
		memcpy_s(&partitionSize, sizeof(DWORD), &sector[40], 4);		//ntfs引导扇区中 分区总共扇区 标志位
		if (partitionSize <= 0)
			goto FINAL;
		else
			partitionSize++;		//因为ntfs的 分区总共扇区 是算少了第一个引导扇区的 , 而fat32中是没有算少的

		strcpy_s(list->type, sizeof(list->type), TEXT("NTFS"));
		list->size = ULONG64((ULONG64)partitionSize * (ULONG64)diskGeometry->BytesPerSector / (1024.0 * 1024.0));
	}
	else if (FAT32 == type)
	{
		memcpy_s(&partitionSize, sizeof(DWORD), &sector[32], 4);		//fat32引导扇区中 分区总共扇区 标志位	Sectors (on large volumes)
		if (partitionSize <= 0)
			goto FINAL;

		strcpy_s(list->type, sizeof(list->type), TEXT("FAT32"));
		list->size = ULONG64((ULONG64)partitionSize * (ULONG64)diskGeometry->BytesPerSector / (1024.0 * 1024.0));
	}
	else if (FAT16 == type)
	{
		memcpy_s(&partitionSize, sizeof(DWORD), &sector[32], 4);		//fat16(fat)引导扇区中 分区总共扇区 标志位
		if (partitionSize <= 0)
			goto FINAL;

		strcpy_s(list->type, sizeof(list->type), TEXT("FAT"));
		list->size = ULONG64((ULONG64)partitionSize * (ULONG64)diskGeometry->BytesPerSector / (1024.0 * 1024.0));
	}
	else if (exFAT == type)
	{
		//分区的总簌数目标志位
		memcpy_s(&partitionSize, sizeof(DWORD), &sector[92], 4);
		//每扇区大小的字节数的2的乘方形式, 一般是9, 即2^9=512字节, 最大是12
		UCHAR sectorSize = sector[108];
		//每簌的扇区数也是2的乘方形式, 最大是25, 即32M, 本机上测试是06, 即2^6=64
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
	//返回值 表示 U盘分区表上第几个项 为活动分区,如1,2,3,4,   NO_Active_Partition表示没有活动分区    FALSE表出错
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
	//寻找活动分区, 根据活动分区标志位
	for (m_ActivePartitionNum = 1; 
			m_ActivePartitionNum <= 4 
			&& sector[sector_size - 66 + (m_ActivePartitionNum - 1) * 16] != 0x80; 
			m_ActivePartitionNum++) {};
	if (m_ActivePartitionNum > 4)
	{
		res = NO_Active_Partition;
		goto FINAL;		//找不到活动分区
	}
	else
	{
		res = m_ActivePartitionNum;
		goto FINAL;		//找不到活动分区
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
	//activePartitionNum 表示 设置U盘分区表上第几个项 为活动分区,
	//如1,2,3,4,   0表示不设活动分区,  5表示将分区表的第一个 有效主分区项 (不一定排第一项)设为活动分区
	//返回值FALSE表出错, -1表要设置的activePartitionNum对应的分区是扩展分区, TRUE表成功设置(就算activePartitionNum == 0)
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

	//只有存在MBR扇区的U盘才可以设置活动分区		CheckMbrPbr()有可能返回false
	if (MBR != this->CheckMbrPbr(sector, sector_size, NULL, NULL, NULL, NULL))
		goto FINAL;

	if (5 == activePartitionNum)
	{
		BOOL isFindMain = FALSE;	//是否找到有效主分区
		BOOL isFindExtend = FALSE;	//是否找到 扩展分区
		DWORD sys_start_pos = 0;
		for (INT j = 0; j < 4; j++)
		{
			//检查是否主分区,  --主分区 ,即非扩展分区0F/05   通过文件系统标志位
			if (sector[sector_size - 66 + 4 + j * 16] != 0x0F
				&& sector[sector_size - 66 + 4 + j * 16] != 0x05)
			{
				//检测分区有效性, 通过分区前 预留扇区  标志位
				memcpy_s(&sys_start_pos, sizeof(sys_start_pos),
					&sector[sector_size - 66 + 8 + j * 16], 4);
				if (sys_start_pos > 0)
				{
					//找到分区表 第一个有效项
					m_activePartitionNum = j + 1;
					isFindMain = TRUE;		//找到主分区
					break;
				}
			}
			else
			{	//找到 扩展分区
				isFindExtend = TRUE;
			}
		}

		if (!isFindMain && !isFindExtend)
		{	//没有主分区 和 扩展分区, 即U盘什么分区都没有
			res = FALSE;
			goto FINAL;
		}
		else if (!isFindMain && isFindExtend)
		{	//没有主分区 和 只有扩展分区, 不符合做启动盘要求
			res = -1;
			goto FINAL;
		}
		//剩下的就是找到主分区了
	}

	for (INT i = 1; i <= 4; i++)
	{
		if (m_activePartitionNum == i)
		{
			//检测  文件系统类型  标志位 是否为 扩展分区0F/05
			if (sector[sector_size - 66 + 4 + (i - 1) * 16] != 0x0F
				&& sector[sector_size - 66 + 4 + (i - 1) * 16] != 0x05)
			{
				sector[sector_size - 66 + (i - 1) * 16] = 0x80;
				//让循环继续执行, 使 非活动分区 设为0x00
			}
			else
			{
				res = -1;	//扩展分区不可以做启动盘, 只有主分区才可以
				goto FINAL;
			}
		}
		else
			sector[sector_size - 66 + (i - 1) * 16] = 0x00;
	}

	//开始写入扇区
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
	//drive格式"G:"
	//disk格式"1"
	//order格式"1234"或 "2134" 等, 若为NULL , 则表示默认"2134"

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

	memset(&sector[sector_size - 66], 0x00, 64);		//16 * 4 ==64  将原来的分区表删除,准备新建分区表
	for (int i = 0; i < num; i++)
	{
		unsigned char sys_active = { 0 };				//分区活动标志
		unsigned char sys_pre_start[4] = { 0 };			//分区前 预留扇区 标志
		unsigned char sys_size[4] = { 0 };				//分区大小 标志
		unsigned char sys_start_pos[3] = { 0 };			//分区开始位置标志
		unsigned char sys_filesystem = { 0 };			//文件系统标志
		unsigned char sys_end_pos[3] = { 0 };			//分区结束位置标志


		if (activeNum == 0 || activeNum != i + 1)		//activeNum == =0 表没有活动分区
		{
			sys_active = 0x00;		//活动分区 标志位
		}
		else	//activeNum>0时候,表第activeNum分区为活动分区
		{
			sys_active = 0x80;		//活动分区 标志位
		}

		if (0 == i)
		{
			temp_pre = unsigned int(1024.0 * 1024.0 / diskGeometry->BytesPerSector);		//分区表项中 分区前预留扇区 标志位(现根据diskgenius做法,在第一分区前预留1MB大小)
		}
		else
		{
			temp_pre = unsigned int(temp_pre + (list[i - 1].size) * 1024.0 * 1024.0 / diskGeometry->BytesPerSector);		//分区表项中 分区前预留扇区 标志位
		}

		for (int j = 0; j < 4; j++)
		{
			sys_pre_start[j] = *(((unsigned char*)&temp_pre) + j);		//分区表项中 分区前预留扇区 标志位		(注意大小端)
		}

		unsigned int temp_size = unsigned int((list[i].size) * 1024.0 * 1024.0 / diskGeometry->BytesPerSector);		// list[i].size是指MB为单位的,所以*1024.0 * 1024.0
		for (int j = 0; j < 4; j++)
		{
			sys_size[j] = *(((unsigned char*)&temp_size) + j);		//分区表项中 分区大小 标志位	(注意大小端)
		}

		sys_start_pos[2] = unsigned char(temp_pre / (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack));	//分区开始柱面
		sys_start_pos[1] = unsigned char(temp_pre % (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) % diskGeometry->SectorsPerTrack + 1);	//分区开始扇区
		sys_start_pos[0] = unsigned char(temp_pre % (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) / diskGeometry->SectorsPerTrack);	//分区开始磁头

		if (strcmp(list[i].type, _T("ntfs")) == 0
			|| strcmp(list[i].type, _T("NTFS")) == 0)
		{
			sys_filesystem = 0x07;		//文件系统标志位
		}
		else if (strcmp(list[i].type, _T("fat32")) == 0
			|| strcmp(list[i].type, _T("FAT32")) == 0)
		{
			sys_filesystem = 0x0c;
		}
		else
		{
			sys_filesystem = 0x00;			//文件系统为未使用
		}

		if ((temp_pre + temp_size - 1) / (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) > 255)
		{
			sys_end_pos[2] = 0xff;
			sys_end_pos[1] = 0xff;
			sys_end_pos[0] = 0xfe;
		}
		else
		{
			sys_end_pos[2] = unsigned char((temp_pre + temp_size - 1) / (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack));	//分区结束柱面 (注意由于是一个字节做存储大小,所以肯定不会大于255, 即391会赋值成为135)
			sys_end_pos[1] = unsigned char((temp_pre + temp_size - 1) % (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) % diskGeometry->SectorsPerTrack + 1);	//分区结束扇区
			sys_end_pos[0] = unsigned char((temp_pre + temp_size - 1) % (diskGeometry->TracksPerCylinder * diskGeometry->SectorsPerTrack) / diskGeometry->SectorsPerTrack);	//分区结束磁头
		}

		unsigned char table[16] = 				//分区表中的单条项目
		{
			/*
			[0]				分区活动标志
			[1]				分区开始磁头
			[2]				分区开始扇区
			[3]				分区开始柱面
			[4]				文件系统标志
			[5]				分区结束磁头
			[6]				分区结束扇区
			[7]				分区结束柱面
			[8,9,10,11]		分区前 预留扇区
			[12,13,14,15]	分区大小*/
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
	//sector[]是指disksector, 即磁盘的第一扇区, 因为上面有分区表信息
	//num是指disksector中分区表中有效的条项的数目, 基于分区表中有效条项都是按顺序排列的假设
	if (!*hDevice || INVALID_HANDLE_VALUE == *hDevice || !diskGeometry)
		return FALSE;
	//CloseHandle(*hDevice);
	//this->GetDriveHandle(TEXT("G:"), hDevice);
	DWORD sys_start_pos[4] = {0};
	UCHAR* newBootSector = NULL;

	DWORD dwbytes = 0;
	LARGE_INTEGER offset;		// 有符号的64位整型表示
	for (int i = 0; i < 4; i++)
	{
		memcpy_s(&sys_start_pos[i], 4, &sector[sector_size - 66 + 8 + i * 16], 4);		//分区前预留扇区标志位, 即分区开始扇区
		if (!sys_start_pos[i])		//分区前预留扇区标志位为0, 说明该分区表项无效
			continue;
		//根据分区类型标志位确定分区类型
		if (0x0c == sector[sector_size - 66 + 4 + i * 16] 
			|| 0x1c == sector[sector_size - 66 + 4 + i * 16])
		{
			newBootSector = pbr_fat32_data;		//fat32分区
			//复制 分区前预留扇区标志位, 即分区开始扇区  和 分区大小标志位
			memcpy_s(&newBootSector[28], 8, &sector[sector_size - 66 + 8 + i * 16], 8);
		}
		else if (0x07 == sector[sector_size - 66 + 4 + i * 16]
			|| 0x17 == sector[sector_size - 66 + 4 + i * 16])
		{
			newBootSector = pbr_ntfs_data;		//ntfs分区
			//复制 分区前预留扇区标志位, 即分区开始扇区
			memcpy_s(&newBootSector[28], 4, &sector[sector_size - 66 + 8 + i * 16], 4);
			//复制 分区大小标志位
			DWORD ntfs_size = 0;
			memcpy_s(&ntfs_size, 4, &sector[sector_size - 66 + 12 + i * 16],4);
			ntfs_size--;
			//因为ntfs的引导扇区中, 文件大小标志位(Total sectors)是减1的
			memcpy_s(&newBootSector[40], 4, &ntfs_size, 4);
		}

		offset.QuadPart = ((ULONG64)sys_start_pos[i]) * ((ULONG64)diskGeometry->BytesPerSector);
		// 从打开的文件（磁盘）中移动文件指针。offset.LowPart低32位为移动字节数
		SetFilePointer(*hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
		::WriteFile(*hDevice, newBootSector, diskGeometry->BytesPerSector, &dwbytes, NULL);
		if (diskGeometry->BytesPerSector != dwbytes)
		{
			//DWORD err = GetLastError();
			//CString str;
			//str.Format("DeletePartitionBootSector()中错误,错误代码%d", err);
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
	//drive格式"G:"
	//disk格式"1"

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
	//drive格式"G:"
	//disk格式"1"
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
			//AfxMessageBox("U盘分区成功");
			//先不进行 this->m_FormatState = FALSE; 因为写入镜像的时候需要用到判断
		}
		else if(FORMAT_ERROR == wParam)
		{
			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
			((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("开始制作!"));
			this->m_FormatState = FALSE;
			if (this->m_needUnzip)		//分区失败,  如果需要解压, 证明是制作启动盘, 所以停止写入镜像
				this->m_needUnzip = FALSE;
			AfxMessageBox("U盘分区失败");
		}
	}
	
	if (this->m_needUnzip)
	{
		if (UNZIP_OK == wParam)
		{
			this->m_needUnzip = FALSE;
			this->m_FormatState = FALSE;
			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
			((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("开始制作!"));
			AfxMessageBox("镜像写入完成--恭喜, 大功告成!");
		}
		else if (UNZIP_ERROR == wParam)
		{
			this->m_needUnzip = FALSE;
			this->m_FormatState = FALSE;
			this->SetMyTimer(GetSafeHwnd(), TIMER_Start, TIMER_Start_time, TRUE);
			((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("开始制作!"));
			AfxMessageBox("镜像写入失败");
		}
	}

	this->m_USB_ViewerDlg->GetUSB(this, IDC_COMBO1);			//在组合框上更新所找到的U盘
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
	// 获得磁盘结构信息
	//
	DWORD dwBytes = 0;
	BOOL res = DeviceIoControl(m_hDevice,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,    // 调用了CTL_CODE macro宏
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

	//将sector中的分区表复制到mbr_data的分区表位置上
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
	//use_hDrive 表示 是否使用 分区的句柄 来代替磁盘句柄, 当TRUE时候, 需要传入有效的drive,且hDevice ==NULL
	//只有当安装PBR的分区 是系统当前显示的分区, 且安装的PBR是NTFS_PBR时, 才[必须]用到
	//因为经过多次测试, 如果是上述情况时, 写入的NTFS_PBR会失败, 因系统貌似保护着U盘显示分区的第一个引导扇区之后的几个扇区
	//不过通过hDrive分区的句柄 却可以写入成功.   当然由于FAT32_PBR==512Byte==第一个引导扇区大小, 应该没被保护, 就顺利写入了

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

	//U盘扇区大小有效性检测, 暂只支持512字节大小扇区的U盘
	if (m_diskGeometry.BytesPerSector != (sizeof(pbr_fat32_data) / sizeof(pbr_fat32_data[0])))
		goto ERROR1;

	//获得活动分区是第m_ActivePartitionNum分区
	INT m_ActivePartitionNum = this->GetActivePartitionNum(&m_hDevice, NULL, NULL);
	if (!m_ActivePartitionNum || NO_Active_Partition == m_ActivePartitionNum)
		goto ERROR1;

	//获得活动分区的type类型, size信息
	Partition_Table list;		// = { 0 };
	if (!this->GetOnePartitionInfo(&list, &m_hDevice, m_ActivePartitionNum, &m_diskGeometry))
		goto ERROR1;
	
	//获得活动分区的引导扇区
	sector = new unsigned char[m_diskGeometry.BytesPerSector]();
	if (!this->GetPartitionBootSector(sector, &m_hDevice, m_ActivePartitionNum, &m_diskGeometry))
		goto ERROR1;
	DWORD pbr_data_size = 0;
	unsigned char* pbr_data = NULL;
	if (strcmp(list.type, TEXT("ntfs")) == 0	\
		||strcmp(list.type, TEXT("NTFS")) == 0)
	{
		//将活动分区(ntfs)的引导扇区特有部分(前84个字节)复制到pbr_ntfs_data对应位置上
		memcpy_s(&pbr_ntfs_data[0], 84, &sector[0], 84);
		pbr_data = pbr_ntfs_data;
		pbr_data_size = sizeof(pbr_ntfs_data);
	}
	else if (strcmp(list.type, TEXT("fat32")) == 0	\
		|| strcmp(list.type, TEXT("FAT32")) == 0)
	{
		//将活动分区(fat32)的引导扇区特有部分(前90个字节)复制到pbr_fat32_data对应位置上
		memcpy_s(&pbr_fat32_data[0], 90, &sector[0], 90);
		pbr_data = pbr_fat32_data;
		pbr_data_size = sizeof(pbr_fat32_data);
	}
	else if (strcmp(list.type, TEXT("fat")) == 0	\
		|| strcmp(list.type, TEXT("FAT")) == 0	\
		|| strcmp(list.type, TEXT("exFAT")) == 0	\
		|| strcmp(list.type, TEXT("EXFAT")) == 0)
	{
		AfxMessageBox(TEXT("抱歉丫~~~\n暂时还不支持fat和exfat文件系统\n暂时只支持fat32和ntfs文件系统\n建议选用推荐选项--格式化后制作启动盘"));
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
		//获取活动分区的开始位置
		memcpy_s(&sys_start_pos, sizeof(sys_start_pos), &sector[sector_size - 66 + 8 + (m_ActivePartitionNum - 1) * 16], 4);
		if (sys_start_pos <= 0)
			goto ERROR1;
		offset.QuadPart = (ULONG64)sys_start_pos * (ULONG64)m_diskGeometry.BytesPerSector;
		
		SetFilePointer(m_hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
	}

	this->LockVolume(NULL, drive);
	//写入活动分区的引导扇区
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
	//删除文件,如G:\[BOOT], 则把[BOOT]文件夹及里面的文件都删除
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
		this->m_UnZipArg = new UnZipArg();		//new进行初始化

	//this->m_UnZipArg->desPath =		//由CCreateStartDlg::OnBnClickedOk()填写
	//this->m_UnZipArg->exePath = ;		//已经设置为NULL, CCreateStartDlg::UnZip()会补填
	//this->m_UnZipArg->isoPath = ;		//由CCreateStartDlg::OnBnClickedOk()填写
	this->m_UnZipArg->msgWnd = this->GetSafeHwnd();

	if (0 >= _beginthreadex(NULL, 0, ThreadFun.ThreadAdress, this, 0, NULL))
		return FALSE;
	else
		return TRUE;
}

INT CCreateStartDlg::UnZip()
{
	if (this->m_FormatState)	//需要格式化, 即是用  推荐选项--格式化再制作启动盘
	{
		if (!this->RemountDrive(2, FALSE))	//挂载U盘第2分区
			goto ERROR2;
	}
	else	//需要格式化, 即是用 保留数据再制作启动盘
	{
		if (!this->RemountDrive(1, FALSE))	//相当于重新挂载U盘
			goto ERROR2;
	}

	if (!this->FreeFile(TEXT("7z.exe"), TEXT("UNZIP"), IDR_7z, NULL, FALSE))		//释放7z
		goto ERROR1;

	/*struct UnZipArg
	{
		CHAR desPath[MAX_PATH];		 == "g:" 或  "g:\" 都行
		CHAR exePath[MAX_PATH];		 == "C:\Users\hello\LOCALS~1\TMP\"  或者 NULL
		CHAR isoPath[MAX_PATH];		 == "C:\Data\CA.iso"   或 NULL
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
	//strcat_s(m_exePath, MAX_PATH, _T("7z.exe"));		//7z路径

	if (*(this->m_UnZipArg->isoPath))
	{
		strcpy_s(m_isoPath, MAX_PATH, this->m_UnZipArg->isoPath);
	}
	else
	{
		GetCurrentDirectory(MAX_PATH, m_isoPath);		//如E:\Projects\test\test
		strcat_s(m_isoPath, MAX_PATH, "\\Data\\CA.iso");		//iso路径
	}

	sprintf_s(Parameters, _T("x \"%s\" -o\"%s\" -y"), m_isoPath, this->m_UnZipArg->desPath);		//Parameters参数

	HANDLE hProcess = INVALID_HANDLE_VALUE;
	int res = this->ShellExe(TEXT("Open"), TEXT("7z.exe"), TEXT("%temp%"),
		Parameters, SW_HIDE, 1800000, &hProcess);		//30min == 1800s == 1800000ms 	该函数是阻塞型函数

	if (WAIT_TIMEOUT == res || WAIT_FAILED == res)
	{
		//也可以直接关闭这个进程，只要保留sei.hProcess就行 
		TerminateProcess(hProcess, 0);
		goto ERROR1;
	}
	else if (WAIT_OBJECT_0 == res)
	{
		HANDLE hDrv = INVALID_HANDLE_VALUE;
		//哪怕this->m_UnZipArg->desPath是长路径, this->GetDriveHandle()都可以获取该分区的句柄
		this->GetDriveHandle(this->m_UnZipArg->desPath, &hDrv);

		CString boolFolder(this->m_UnZipArg->desPath);
		if (boolFolder.Right(1).CompareNoCase(TEXT("\\")) == 0)
			boolFolder.Append(TEXT("[BOOT]"));
		else
			boolFolder.Append(TEXT("\\[BOOT]"));
		this->DeleteFolder(boolFolder.GetBuffer());

		::FlushFileBuffers(hDrv);		//刷人缓存中的数据到U盘
		Sleep(2000);
		this->FreeFile(TEXT("7z.exe"), TEXT("UNZIP"), IDR_7z, NULL, TRUE);		//清除7z.exe
		if (this->m_FormatState)	//需要格式化, 即是用  推荐选项--格式化再制作启动盘
			this->RemountDrive(2, TRUE);	//还原U盘分区设置
		::PostMessage(this->m_UnZipArg->msgWnd, WM_MYMSG, UNZIP_OK, 0);
		return TRUE;
	}
	else
	{
		AfxMessageBox("UnZip()等待到未知参数?");
		goto ERROR1;
	}
/*

	SHELLEXECUTEINFO sei;
	// 启动进程 
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
	//   加入下面这句就是等待该进程结束 
	int res = WaitForSingleObject(sei.hProcess, 1800000);	//30min == 1800s == 1800000ms*/
	


ERROR1:
	if (this->m_FormatState)	//需要格式化, 即是用  推荐选项--格式化再制作启动盘
		this->RemountDrive(2, TRUE);	//还原U盘分区设置
	//else	//不需要格式化, 即是用 保留数据再制作启动盘
	//{	不需要做操作
	//}

ERROR2:
	this->FreeFile(TEXT("7z.exe"), TEXT("UNZIP"), IDR_7z, NULL, TRUE);		//清除7z.exe
	::PostMessage(this->m_UnZipArg->msgWnd, WM_MYMSG, UNZIP_ERROR, 0);
	return FALSE;
}

/*
INT CCreateStartDlg::Free7z(LPCSTR path / *= NULL* /, BOOL isDelete / *= FALSE* /)
{
	//本函数有两个作用: 释放7z.exe 和 删除7z.exe
	//path == "C:\Users\hello\LOCALS~1\TMP\" 等(需要\后缀)
	//若path为null, 则自动填写为%TMP%
	TCHAR m_path[MAX_PATH] = { 0 }; 
	if (path)
	{
		strcpy_s(m_path, MAX_PATH, path);
	}
	else
	{
		GetTempPath(MAX_PATH, m_path);		//返回 %TMP% 目录, 如C:\Users\hello\LOCALS~1\TMP
	}
	strcat_s(m_path, MAX_PATH, _T("7z.exe"));

	if (isDelete)
		return ::DeleteFile(m_path);	//删除7z.exe

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
	WriteFile(hfile, h7z, fileSize, &dwBytes, NULL);		//释放7z.exe到 临时文件夹
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
	//该函数是阻塞型函数
	CHAR m_path[MAX_PATH + 1] = {0};
	if (!strcmp(path, TEXT("%temp%"))
		|| !strcmp(path, TEXT("%tmp%")))
	{
		GetTempPath(MAX_PATH, m_path);			//返回 %TMP% 目录, 如C:\Users\hello\LOCALS~1\TMP
	}
	else if (!strcmp(path, TEXT("system32")))
	{
		GetWindowsDirectory(m_path, MAX_PATH + 1);		//取得windows目录,如C:\Windows
		strcat_s(m_path, MAX_PATH + 1, "\\System32");
	}
	else
	{
		strcpy_s(m_path, MAX_PATH + 1, path);
	}

	SHELLEXECUTEINFO sei;
	// 启动进程 
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
	//   加入下面这句就是等待该进程结束 
	*hProcess = sei.hProcess;
	return WaitForSingleObject(sei.hProcess, timeout);
}

INT CCreateStartDlg::FreeFile(CHAR* outName, CHAR* resType, INT resID, LPCSTR path /*= NULL*/, BOOL isDelete /*= FALSE*/)
{
	//本函数有两个作用: 释放resType类的resID到path下的outName文件中 和 删除path下的outName文件
	//path == "C:\Users\hello\LOCALS~1\TMP\" 等(需要\后缀)
	//若path为null, 则自动填写为%TMP%

	TCHAR m_path[MAX_PATH] = { 0 };
	if (path)
	{
		strcpy_s(m_path, MAX_PATH, path);
	}
	else
	{
		GetTempPath(MAX_PATH, m_path);		//返回 %TMP% 目录, 如C:\Users\hello\LOCALS~1\TMP
	}
	strcat_s(m_path, MAX_PATH, TEXT(outName));

	if (isDelete)
		return ::DeleteFile(m_path);	//删除文件

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
	WriteFile(hfile, pfile, fileSize, &dwBytes, NULL);		//释放到 目标文件夹
	CloseHandle(hfile);
	return dwBytes == fileSize;
}
/*

INT CCreateStartDlg::FreeQemu(LPCSTR path = NULL, BOOL isDelete = FALSE)
{
	//本函数有两个作用: 释放MyQemu.exe 和 删除MyQemu.exe
	//path == "C:\Users\hello\LOCALS~1\TMP\" 等(需要\后缀)
	//若path为null, 则自动填写为%TMP%
	TCHAR m_path[MAX_PATH] = { 0 };
	if (path)
	{
		strcpy_s(m_path, MAX_PATH, path);
	}
	else
	{
		GetTempPath(MAX_PATH, m_path);		//返回 %TMP% 目录, 如C:\Users\hello\LOCALS~1\TMP
	}
	strcat_s(m_path, MAX_PATH, _T("MyQemu.exe"));

	if (isDelete)
		return ::DeleteFile(m_path);	//删除MyQemu.exe

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
	WriteFile(hfile, h7z, fileSize, &dwBytes, NULL);		//释放MyQemu.exe到 临时文件夹
	CloseHandle(hfile);
	return dwBytes == fileSize;
}*/

INT CCreateStartDlg::RemountDrive(INT remountNum /*= 2*/, BOOL isRestore /*= FALSE*/, LPCSTR desPath /*= NULL*/)
{
	//本函数使系统挂载U盘中第remountNum个分区(1, 2, 3, 4)
	//isRestore表示是否将U盘的分区挂载情况复原
	//一般第一次调用设置remountNum值 , 第二次调用 增加设置isRestore值
	//desPath 可以被设为"g:"等,达到重新挂载该盘
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
	//返回U盘 总容量  , 字节做单位

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
	//nEventType就是WM_DEVICECHANGE消息的wParam参数，具体的值参考msdn
	switch (nEventType)
	{
	case 0x8000:		// DBT_DEVICEARRIVAL==0x8000		u盘插入
	case 0x8004:		//DBT_DEVICEREMOVECOMPLETE==0x8004	u盘拔出
	{
		this->m_USB_ViewerDlg->GetUSB(this, IDC_COMBO1);	//重新加载U盘
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
	//检查魔数 55AA
	if (m_sector[sector_size - 2] != 0x55
		|| m_sector[sector_size - 1] != 0xAA)
	{
		res = NO_MBR_PBR;
		goto FINAL;
	}

	//检验是否为mbr
	BOOL isMBR = TRUE;
	for (int i = 0; i < 4; i++)
	{
		//检查活动分区标志位
		if (m_sector[sector_size - 66 + i * 16] != 0x80
			&& m_sector[sector_size - 66 + i * 16] != 0x00)
		{
			isMBR = FALSE;
			break;
		}
	}
	if (isMBR)
	{
		res = MBR;		//是MBR扇区
		goto FINAL;
	}
	
	if (0 >= diskGeometry.BytesPerSector)
		diskGeometry.BytesPerSector = sector_size;
	//以下是 只有pbr扇区情况
	if (!this->GetOnePartitionInfo(&temp_list, NULL, NULL, &diskGeometry, m_sector, sector_size))
	{
		//GetOnePartitionInfo()返回false, 即m_sector也不是pbr
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
	/*//检查是否为NTFS_PBR
	CHAR name[6] = { 0 };
	memcpy_s(name, sizeof(name), &m_sector[3], 4);		//m_sector[3,4,5,6]是NTFS的File system ID ("NTFS")
	//判断分区的文件系统
	if (strcmp(name, TEXT("NTFS")) == 0)
	{
		res = PBR_NTFS;
	}
	else
	{
		//检查是否为FAT32_PBR
		ZeroMemory(name, sizeof(name));
		memcpy_s(name, sizeof(name), &m_sector[82], 5);		//m_sector[82,83,84,85,86]是FAT32的File system标志位("FAT32")
		if (strcmp(name, TEXT("FAT32")) == 0)
			res = PBR_FAT32;
		else
		{
			res = NO_MBR_PBR;
			goto FINAL;
		}
	}

	//如果有需要则, 设置分区的  文件系统type 和 大小size 参数
	if (list)
	{
		DWORD partitionSize = 0;
		if (PBR_NTFS == res)
		{
			memcpy_s(&partitionSize, sizeof(DWORD), &m_sector[40], 4);		//ntfs引导扇区中 分区总共扇区 标志位
			if (partitionSize <= 0)
			{
				res = NO_MBR_PBR;
				goto FINAL;
			}
			else
				partitionSize++;		//因为ntfs的 分区总共扇区 是算少了第一个引导扇区的 , 而fat32中是没有算少的

			strcpy_s(list->type, sizeof(list->type), TEXT("NTFS"));
			list->size = ULONG64((ULONG64)partitionSize * (ULONG64)sector_size / (1024.0 * 1024.0));
			//没有diskGeometry->BytesPerSector, 就用sector_size代替
		}
		else if (PBR_FAT32 == res)
		{
			memcpy_s(&partitionSize, sizeof(DWORD), &m_sector[32], 4);		//fat32引导扇区中 分区总共扇区 标志位	Sectors (on large volumes)
			if (partitionSize <= 0)
			{
				res = NO_MBR_PBR;
				goto FINAL;
			}

			strcpy_s(list->type, sizeof(list->type), TEXT("FAT32"));
			list->size = ULONG64((ULONG64)partitionSize * (ULONG64)sector_size / (1024.0 * 1024.0));
			//没有diskGeometry->BytesPerSector, 就用sector_size代替
		}
		else
		{
			res = NO_MBR_PBR;
			goto FINAL;
		}
	}
	goto FINAL;		//可能是PBR_FAT32 或者 PBR_NTFS
	*/

FINAL:
	if (!hDevice)
		CloseHandle(m_hDevice);		//CloseHandle()已经对m_hDevice == 0xfffff 有防错处理
	if (sector_size <= 0 || !sector)
	{
		if (m_sector)
			delete[] m_sector;
	}

	return res;
}

INT CCreateStartDlg::SetMyTimer(HWND hWnd, UINT timerID, UINT time, BOOL isKill /*= FALSE*/)
{
	//time 是微秒做单位
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
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT(" 制作ing."));
		break;
	case 2:
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("  制作ing.."));
		break;
	case 3:
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("   制作ing..."));
		break;
	case 4:
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("制作ing"));
		break;
	}
	num = ++num > 4 ? 1 : num;
	
	CDialogEx::OnTimer(nIDEvent);
}


void CCreateStartDlg::OnBnClickedCancel()
{
	if (this->m_FormatState || this->m_needUnzip)
	{
		AfxMessageBox(TEXT("制作启动盘ing, 请稍后"));
		return;
	}

	CDialogEx::OnCancel();
}


void CCreateStartDlg::OnBnClickedBoot()
{
	if (this->m_FormatState || this->m_needUnzip)
	{
		AfxMessageBox(TEXT("制作启动盘ing, 请稍后"));
		return;
	}
	CString drive;
	((CComboBox*)GetDlgItem(IDC_COMBO1))->GetWindowText(drive);
	//因为U盘的显示格式是"G:\ 16G"
	if (drive.Find(":\\") == -1)
	{
		AfxMessageBox(TEXT("先选择U盘才能进行操作哈"));
		return;		//针对  空白的选项
	}

	union
	{
		INT(__stdcall CCreateStartDlg::*BootPE)();
		unsigned int(__stdcall* ThreadAdress)(void*);
	}ThreadFun;
	ThreadFun.BootPE = &CCreateStartDlg::BootPE;

	if (0 >= _beginthreadex(NULL, 0, ThreadFun.ThreadAdress, this, 0, NULL))
	{
		AfxMessageBox(TEXT("模拟器好像遇到点问题哦 T_T\n\n不好意思啊, 我要回去修理了"));
		return;
	}
}

INT __stdcall CCreateStartDlg::BootPE()
{
	CString drive;
	((CComboBox*)GetDlgItem(IDC_COMBO1))->GetWindowText(drive);
	INT pos = drive.Find(":\\");	//因为U盘的显示格式是"G:\ 16G"
	if (pos == -1)
	{
		AfxMessageBox(TEXT("先选择U盘才能进行操作哈"));
		return FALSE;		//针对  空白的选项
	}
	drive = drive.Mid(pos - 1, 2);		//现在str表示"G:"

	AfxMessageBox(TEXT("提醒: 【Ctrl + Alt】可让鼠标从模拟器中返回到真实系统哈^_^\n\n模拟器只是用来验证能否引导U盘的系统\n\n若运行模拟器里的工具, 可能会导致真实系统的不稳定哟~~~"));

	LPCSTR fileName[] =
	{	//MyQemu解压出来的所有文件
		"MyQemu.exe",
		"qemu.exe",
		"bios.bin",
		"libusb0.dll",
		"libz-1.dll",
		"SDL.dll",
		"vgabios.bin"
	};

	CString str(TEXT("x -y"));		//作为参数, 表示 无条件覆盖原文件的解压
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	INT diskNumber = -1;

	if (!this->GetDiskNumber(drive.GetBuffer(), NULL, &diskNumber))
		goto ERROR1;

	if (!this->FreeFile(TEXT("MyQemu.exe"), TEXT("UNZIP"), IDR_Qemu, NULL, FALSE))		//释放MyQemu.exe
		goto ERROR1;

	int res = this->ShellExe(TEXT("Open"), TEXT("MyQemu.exe"), TEXT("%temp%"),
		str.GetBuffer(), SW_HIDE, 5000, &hProcess);		//5s == 5000ms 现在先用来解压	该函数是阻塞型函数

														//确认qemu参数
	str.Format(TEXT("-L . -boot c -m 300 -hda //./PhysicalDrive%d -localtime -vga std -snapshot"), diskNumber);

	res = this->ShellExe(TEXT("Open"), TEXT("qemu.exe"), TEXT("%temp%"),
		str.GetBuffer(), SW_HIDE, 1800000, &hProcess);		//30min == 1800s == 1800000ms 现在开始运行模拟器	该函数是阻塞型函数
	if (WAIT_OBJECT_0 == res)
	{
		for (INT i = sizeof(fileName) / sizeof(fileName[0]); i; )
		{
			//清理文件
			this->FreeFile((CHAR*)(fileName[--i]), NULL, NULL, NULL, TRUE);
		}
		return TRUE;
	}
	else if (WAIT_TIMEOUT == res || WAIT_FAILED == res)
	{
		//也可以直接关闭这个进程，只要保留sei.hProcess就行 
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
		//清理文件
		this->FreeFile((CHAR*)(fileName[--i]), NULL, NULL, NULL, TRUE);
	}
	AfxMessageBox(TEXT("模拟器好像遇到点问题哦 T_T\n\n不好意思啊, 我要回去修理了"));
	return FALSE;
}


