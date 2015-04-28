#pragma once
#include "afxdialogex.h"
#include "Resource.h"

#include <winioctl.h>
#include "Define.h"
#include "USB_ViewerDlg.h"



class CCreateStartDlg:
	public CDialogEx
{
public:

	CCreateStartDlg();
	~CCreateStartDlg();

	INT SetFormatPartitionArg(FormatPartitionArg* arg);
	INT Partition(TCHAR* drive, INT num, Partition_Table* list, INT activeNum, HWND msgWnd, LPCSTR eventName);
	INT CreatePartitionTable(UCHAR sector[], INT sector_size, INT num, Partition_Table* list, INT activeNum, DISK_GEOMETRY* diskGeometry);
	INT DeletePartitionBootSector(HANDLE *hDevice, INT num, UCHAR sector[], INT sector_size, DISK_GEOMETRY* diskGeometry);
	INT BackupPartitionBootSector(HANDLE *hDevice, INT partitionNum, DISK_GEOMETRY* diskGeometry);
	INT RestorePartitionBootSector(HANDLE *hDevice, INT num, DISK_GEOMETRY* diskGeometry);
	INT GetPartitionBootSector(UCHAR sector[], HANDLE *hDevice, INT partitionNum, DISK_GEOMETRY* diskGeometry);
	INT GetOnePartitionInfo(Partition_Table *list, HANDLE *hDevice, INT partitionNum, DISK_GEOMETRY* diskGeometry, UCHAR bootSector[] = NULL, INT sector_size = 0);
	INT GetActivePartitionNum(HANDLE *hDevice, LPCSTR drive, LPCSTR disk);
	INT SetActiveAndHide(INT PartitionNum, INT active, INT hide,HANDLE *hDevice, LPCSTR drive, LPCSTR disk);
	LARGE_INTEGER GetUSBAllSize(LPCSTR drive);
	INT CheckMbrPbr(UCHAR sector[], INT sector_size, Partition_Table* list = NULL, HANDLE *hDevice = NULL, LPCSTR drive = NULL, LPCSTR disk = NULL);
	INT __stdcall FormatPartition();
	INT __stdcall UnZip();
	INT __stdcall BootPE();
	INT StartUnZip();
	//INT Free7z(LPCSTR path = NULL, BOOL isDelete = FALSE);
	//INT FreeQemu(LPCSTR path = NULL, BOOL isDelete = FALSE);
	INT ShellExe(CHAR* operate, CHAR* file, CHAR* path, CHAR* parameters, INT show, INT timeout, _Out_ HANDLE* hProcess);		//�ú����������ͺ���
	INT FreeFile(CHAR* outName, CHAR* resType, INT resID, LPCSTR path = NULL, BOOL isDelete = FALSE);
	INT LockVolume(HANDLE *hDisk, LPCSTR drive = NULL);
	INT UnLockVolume();
	INT DeleteFolder(LPCSTR path);
	INT DismountVolume(LPCSTR drive = NULL);
	INT RemountDrive(INT remountNum = 2, BOOL isRestore = FALSE, LPCSTR desPath = NULL);
	INT UpdateDisk(HANDLE *hDevice, LPCSTR drive, LPCSTR disk);
	INT GetDriveHandle(LPCSTR drive, HANDLE *hDrive);
	INT GetDiskHandle(LPCSTR drive, LPCSTR disk, HANDLE *hDevice);
	INT GetDiskNumber(LPCSTR drive, HANDLE *hDrive, INT* diskNumber);
	INT GetDiskSector(UCHAR sector[], HANDLE *hDevice, LPCSTR drive, LPCSTR disk, DISK_GEOMETRY* diskGeometry);
	INT GetDiskGeometry(HANDLE *hDevice, LPCSTR drive, LPCSTR disk, DISK_GEOMETRY* diskGeometry);
	INT ChangePartitionTable(HANDLE *hDisk, LPCSTR drive, LPCSTR disk, LPCSTR order = NULL);
	INT InstallMBR(HANDLE *hDevice, LPCSTR drive, LPCSTR disk, DISK_GEOMETRY* diskGeometry);
	INT InstallPBR(HANDLE *hDevice, LPCSTR drive = NULL, LPCSTR disk = NULL, BOOL use_hDrive = FALSE, DISK_GEOMETRY* diskGeometry = NULL);
	INT SetMyTimer(HWND hWnd, UINT timerID, UINT time, BOOL isKill = FALSE);

private:
	CUSB_ViewerDlg *m_USB_ViewerDlg;
	FormatPartitionArg *m_FormatPartitionArg;
	UnZipArg* m_UnZipArg;
	UCHAR* m_PartitionBootSector[4];
	INT m_FormatState;
	INT m_needUnzip;
	HANDLE m_hVolume;
	enum 
	{ 
		TIMER_Start = 1000,
		TIMER_Start_time = 750
	};

public:
	// �Ի�������
	enum { IDD = IDD_CreateStartDlg };
	
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnGetResult(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);		//���U�̲���  �γ�
	afx_msg void OnTimer(UINT_PTR nIDEvent);		//ʵ������ing�Ķ�̬Ч��
	afx_msg void OnBnClickedBoot();

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCancel();

};

