#pragma once
#include "afxwin.h"
#include "PartitionDlg.h"
#include "USB_ViewerDlg.h"
#include "CreateStartDlg.h"

// CPartitionDlg 对话框

class CPartitionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPartitionDlg)

public:
	CPartitionDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPartitionDlg();


	INT SetSize(INT sizeNum);
	INT SetActive(INT activeNum);
	BOOL IsNumber(CString& str);

private:
	CString m_drive;

	CCreateStartDlg* m_CreateStartDlg;
	CUSB_ViewerDlg* m_USB_ViewerDlg;

	CComboBox* p_file_system[4];
	CComboBox* p_size[4];
	CComboBox* p_unit[4];
	CButton* p_active[4];
	CComboBox m_select_usb;

/*
	CComboBox m_file_system1;
	CComboBox m_file_system2;
	CComboBox m_file_system3;
	CComboBox m_file_system4;
	CComboBox m_size1;
	CComboBox m_size2;
	CComboBox m_size3;
	CComboBox m_size4;
	CComboBox m_unit1;
	CComboBox m_unit2;
	CComboBox m_unit3;
	CComboBox m_unit4;
	CButton m_active1;
	CButton m_active2;
	CButton m_active3;
	CButton m_active4;*/



// 对话框数据
	enum { IDD = IDD_PartitionDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

afx_msg void OnCbnSelchangeSelectUsb();
afx_msg void OnCbnDropdownSize1();
afx_msg void OnCbnDropdownSize2();
afx_msg void OnCbnDropdownSize3();
afx_msg void OnCbnDropdownSize4();
afx_msg void OnBnClickedActive1();
afx_msg void OnBnClickedActive2();
afx_msg void OnBnClickedActive3();
afx_msg void OnBnClickedActive4();
afx_msg void OnBnClickedCancel();
afx_msg void OnBnClickedOk();
};
