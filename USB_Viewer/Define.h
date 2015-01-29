#pragma once


enum
{
	WM_MYMSG = WM_USER + 1,
	FORMAT_OK,
	FORMAT_ERROR,
	UNZIP_OK,
	UNZIP_ERROR,
};

struct Partition_Table
{
	CHAR type[8];		//ֻ��"ntfs"��"fat32"��Ч
	LONG size;			//��MBΪ��λ
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
