
// USB_ViewerDlg.h : 头文件
//
#pragma once
#include "Define.h"

// CUSB_ViewerDlg 对话框
class CUSB_ViewerDlg : public CDialogEx
{
// 构造
public:
	CUSB_ViewerDlg(CWnd* pParent = NULL);	// 标准构造函数

	//////////////////////////////////////////////////////////////////////////
	//INT FindDeviceInstanceID(LPCSTR path, PSTR* DeviceInstanceID);
	//INT FindDeviceInstanceID_by_USB(PSTR InstanceID_by_disk, INT sizeSrc, PSTR InstanceID_by_usb);



	
	// 获取当前所有U盘设备, 并显示到ID控件上
	INT GetUSB(CDialogEx* dialog, INT ID);
	//获得给定U盘中的基本分区信息
	INT GetUSBInfo(Partition_Table(*list)[4], /*UCHAR sector[] = NULL, INT sectorSize = 0,*/ HANDLE *hDevice = NULL, LPCSTR drive = NULL, LPCSTR disk = NULL);

	
// 对话框数据
	enum { IDD = IDD_USB_VIEWER_DIALOG };

// 实现
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	HICON m_hIcon;

	
	// 生成的消息映射函数
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCeatestart();
	afx_msg void OnExit();

	afx_msg void OnClickList2(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnAbout();
afx_msg void OnBnClickedCancel();
};
