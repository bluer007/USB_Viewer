#pragma once


enum
{
	//以下常量定义程序完成的状态
	WM_MYMSG = WM_USER + 1,
	FORMAT_OK,
	FORMAT_ERROR,
	UNZIP_OK,
	UNZIP_ERROR,

	NO_MBR_PBR,
	MBR,
	PBR_NTFS,
	PBR_FAT32,

	NO_Active_Partition,		//用于CCreateStartDlg::GetActivePartitionNum()函数, 表示找不到活动分区
};

const DWORD FAT32_SIZE_LIMIT = 35;		//format命令要求fat32 > 35MB
const DWORD NTFS_SIZE_LIMIT = 2;		//format命令要求ntfs > 2MB

struct Partition_Table
{
	CHAR type[8];						//只有"ntfs"和"fat32"有效
	unsigned long long size;			//以MB为单位		(ULONG64)
};

struct FormatPartitionArg
{
	HANDLE hDrv;
	CHAR drive[3];
	INT num;
	Partition_Table* list;
	HWND msgWnd;
	CHAR eventName[10];
};

struct UnZipArg
{
	CHAR desPath[MAX_PATH];
	CHAR exePath[MAX_PATH];
	CHAR isoPath[MAX_PATH];
	HWND msgWnd;
};

