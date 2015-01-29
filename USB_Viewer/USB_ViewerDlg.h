
// USB_ViewerDlg.h : ͷ�ļ�
//
#pragma once
#include "Define.h"

// CUSB_ViewerDlg �Ի���
class CUSB_ViewerDlg : public CDialogEx
{
// ����
public:
	CUSB_ViewerDlg(CWnd* pParent = NULL);	// ��׼���캯��

	//////////////////////////////////////////////////////////////////////////
	//INT FindDeviceInstanceID(LPCSTR path, PSTR* DeviceInstanceID);
	//INT FindDeviceInstanceID_by_USB(PSTR InstanceID_by_disk, INT sizeSrc, PSTR InstanceID_by_usb);



	
	// ��ȡ��ǰ����U���豸, ����ʾ��ID�ؼ���
	INT GetUSB(CDialogEx* dialog, INT ID);
	//��ø���U���еĻ���������Ϣ
	INT GetUSBInfo(Partition_Table(*list)[4], /*UCHAR sector[] = NULL, INT sectorSize = 0,*/ HANDLE *hDevice = NULL, LPCSTR drive = NULL, LPCSTR disk = NULL);

	
// �Ի�������
	enum { IDD = IDD_USB_VIEWER_DIALOG };

// ʵ��
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	HICON m_hIcon;

	
	// ���ɵ���Ϣӳ�亯��
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
