// PartitionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "USB_Viewer.h"
#include "PartitionDlg.h"
#include "afxdialogex.h"
#include "Define.h"
#include "CreateStartDlg.h"

#define	TEXT_REST_SIZE	TEXT("剩余容量")
#define	TEXT_ALL_SIZE	TEXT("全部")


// CPartitionDlg 对话框
IMPLEMENT_DYNAMIC(CPartitionDlg, CDialogEx)

CPartitionDlg::CPartitionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPartitionDlg::IDD, pParent)
{
	this->m_USB_ViewerDlg = new CUSB_ViewerDlg;
	this->m_CreateStartDlg = new CCreateStartDlg;
}

CPartitionDlg::~CPartitionDlg()
{
	delete this->m_USB_ViewerDlg;
	delete this->m_CreateStartDlg;
}

void CPartitionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELECT_USB, m_select_usb);
/*
		DDX_Control(pDX, IDC_FILE_SYSTEM1, m_file_system1);
		DDX_Control(pDX, IDC_FILE_SYSTEM2, m_file_system2);
		DDX_Control(pDX, IDC_FILE_SYSTEM3, m_file_system3);
		DDX_Control(pDX, IDC_FILE_SYSTEM4, m_file_system4);
		DDX_Control(pDX, IDC_SIZE1, m_size1);
		DDX_Control(pDX, IDC_SIZE2, m_size2);
		DDX_Control(pDX, IDC_SIZE3, m_size3);
		DDX_Control(pDX, IDC_SIZE4, m_size4);
		DDX_Control(pDX, IDC_UNIT1, m_unit1);
		DDX_Control(pDX, IDC_UNIT2, m_unit2);
		DDX_Control(pDX, IDC_UNIT3, m_unit3);
		DDX_Control(pDX, IDC_UNIT4, m_unit4);
		DDX_Control(pDX, IDC_ACTIVE1, m_active1);
		DDX_Control(pDX, IDC_ACTIVE2, m_active2);
		DDX_Control(pDX, IDC_ACTIVE3, m_active3);
		DDX_Control(pDX, IDC_ACTIVE4, m_active4);*/
}


BEGIN_MESSAGE_MAP(CPartitionDlg, CDialogEx)
ON_CBN_SELCHANGE(IDC_SELECT_USB, &CPartitionDlg::OnCbnSelchangeSelectUsb)
ON_CBN_DROPDOWN(IDC_SIZE1, &CPartitionDlg::OnCbnDropdownSize1)
ON_CBN_DROPDOWN(IDC_SIZE2, &CPartitionDlg::OnCbnDropdownSize2)
ON_CBN_DROPDOWN(IDC_SIZE3, &CPartitionDlg::OnCbnDropdownSize3)
ON_CBN_DROPDOWN(IDC_SIZE4, &CPartitionDlg::OnCbnDropdownSize4)
ON_BN_CLICKED(IDC_ACTIVE1, &CPartitionDlg::OnBnClickedActive1)
ON_BN_CLICKED(IDC_ACTIVE2, &CPartitionDlg::OnBnClickedActive2)
ON_BN_CLICKED(IDC_ACTIVE3, &CPartitionDlg::OnBnClickedActive3)
ON_BN_CLICKED(IDC_ACTIVE4, &CPartitionDlg::OnBnClickedActive4)
ON_BN_CLICKED(IDCANCEL, &CPartitionDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDOK, &CPartitionDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CPartitionDlg 消息处理程序


BOOL CPartitionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	this->p_file_system[0] = (CComboBox*)(this->GetDlgItem(IDC_FILE_SYSTEM1));
	this->p_file_system[1] = (CComboBox*)(this->GetDlgItem(IDC_FILE_SYSTEM2));
	this->p_file_system[2] = (CComboBox*)this->GetDlgItem(IDC_FILE_SYSTEM3);
	this->p_file_system[3] = (CComboBox*)this->GetDlgItem(IDC_FILE_SYSTEM4);

	this->p_size[0] = (CComboBox*)this->GetDlgItem(IDC_SIZE1);
	this->p_size[1] = (CComboBox*)this->GetDlgItem(IDC_SIZE2);
	this->p_size[2] = (CComboBox*)this->GetDlgItem(IDC_SIZE3);
	this->p_size[3] = (CComboBox*)this->GetDlgItem(IDC_SIZE4);

	this->p_unit[0] = (CComboBox*)this->GetDlgItem(IDC_UNIT1);
	this->p_unit[1] = (CComboBox*)this->GetDlgItem(IDC_UNIT2);
	this->p_unit[2] = (CComboBox*)this->GetDlgItem(IDC_UNIT3);
	this->p_unit[3] = (CComboBox*)this->GetDlgItem(IDC_UNIT4);

	this->p_active[0] = (CButton*)this->GetDlgItem(IDC_ACTIVE1);
	this->p_active[1] = (CButton*)this->GetDlgItem(IDC_ACTIVE2);
	this->p_active[2] = (CButton*)this->GetDlgItem(IDC_ACTIVE3);
	this->p_active[3] = (CButton*)this->GetDlgItem(IDC_ACTIVE4);


	this->m_USB_ViewerDlg->GetUSB(this, IDC_SELECT_USB);	//加载已有U盘
	this->OnCbnSelchangeSelectUsb();	//使设置this->m_drive;
	for (int i = 0; i < 4; i++)
	{
		this->p_file_system[i]->AddString(TEXT("NTFS"));
		this->p_file_system[i]->AddString(TEXT("FAT32"));

		this->p_unit[i]->AddString(TEXT("GB"));
		this->p_unit[i]->AddString(TEXT("MB"));
	}
	this->p_file_system[0]->SetCurSel(0);
	this->p_unit[0]->SetCurSel(0);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常:  OCX 属性页应返回 FALSE
}

void CPartitionDlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	/*if (0 == this->m_drive.CompareNoCase(TEXT("")))
	{
		AfxMessageBox(TEXT("亲, 先选择U盘哈 ^_^"));
		return;
	}*/




	CString size, file_system, unit;
	INT restSizeNum = 0, activeNum = 0;		//表示 第几个 选择剩余容量的分区 和 第几个 设置活动分区的分区 
	for (int i = 0, j = 0; i < 4; i++)
	{
		this->p_size[i]->GetWindowText(size);
		if (this->p_size[i]->GetCurSel() == 1 && size.Find(TEXT_REST_SIZE) > -1)
		{
			restSizeNum = i + 1;
			this->p_unit[i]->SetCurSel(0);	//为了方便下面的检测逻辑
			if (++j > 1)
			{
				restSizeNum = 0;
				AfxMessageBox(TEXT("剩余容量 只能分给一个分区啊"));
				return;
			}
		}
	}


	Partition_Table m_table[4] ={0};
	LARGE_INTEGER usb_size;		//u盘总容量
	ULONG64 allSize = 0;	//所有分区总大小
	INT no_size = 0;	//没有填写  大小 的分区的计数
	//MB做单位
	usb_size.QuadPart = (this->m_CreateStartDlg->GetUSBAllSize(this->m_drive.GetBuffer()).QuadPart / (1024.0 * 1024.0));	
	for (int i = 0; i < 4; i++)
	{
		this->p_size[i]->GetWindowText(size);

		if (!*(size.GetBuffer()))
		{
			no_size++;
			continue;
		}
		if (this->IsNumber(size) 
			|| (this->p_size[i]->GetCurSel() == 0 && size.Find(TEXT_ALL_SIZE) > -1)		//选择了  全部容量 也合法
			|| (this->p_size[i]->GetCurSel() == 1 && size.Find(TEXT_REST_SIZE) > -1))		//选择了  剩余容量 也合法
		{
			this->p_file_system[i]->GetWindowText(file_system);
			if (!*(file_system.GetBuffer()))
			{
				file_system.Format(TEXT("第 %d 分区还没选择 文件系统 呢"), i + 1);
				AfxMessageBox(file_system.GetBuffer());
				return;
			}
			this->p_unit[i]->GetWindowText(unit);
			if (!*(unit.GetBuffer()))
			{
				unit.Format(TEXT("第 %d 分区还没选择 单位 呢"), i + 1);
				AfxMessageBox(unit.GetBuffer());
				return;
			}
		}
		else
		{
			size.Format(TEXT("第 %d 分区要输入有效大小哈"), i+1);
			AfxMessageBox(size.GetBuffer());
			return;
		}

		if (this->p_active[i]->GetCheck())
			activeNum = i + 1;		//记录活动分区的号

		
		//经过上面检测, 起码说明 这一项分区信息 填写正确
		//开始填写m_table  该分区的信息
		if (this->p_size[i]->GetCurSel() == 0 && size.Find(TEXT_ALL_SIZE) > -1)
		{
			m_table[i].size = (ULONG64)usb_size.QuadPart;			//选择了  全部容量 也合法
		}
		else
		{
			if (this->p_size[i]->GetCurSel() == 1 && size.Find(TEXT_REST_SIZE) > -1)
			{
				//剩余容量 的赋值操作  最后才做
			}
			else
			{
				if (unit.CompareNoCase(TEXT("GB")) == 0)
				{
					m_table[i].size = (ULONG64)(atof(size.GetBuffer()) * 1024.0);
				}
				else if (unit.CompareNoCase(TEXT("MB")) == 0)
				{
					m_table[i].size = (ULONG64)(atoll(size.GetBuffer()));
				}
			}
		}

		if (file_system.CompareNoCase(TEXT("NTFS")) == 0)
			strcpy_s(m_table[i].type, sizeof(m_table[i].type), TEXT("ntfs"));
		else
			strcpy_s(m_table[i].type, sizeof(m_table[i].type), TEXT("fat32"));
		//到这里, 填写m_table  该分区的信息完毕

		//开始计算截止为止 分区总大小
		if ((allSize += m_table[i].size) > usb_size.QuadPart)
		{
			//同时 也处理了  多次选择  全部容量  的问题
			AfxMessageBox(TEXT("所有分区的总大小 超过 U盘容量啦"));
			return;
		}
	}
	if (no_size >= 4)
	{
		AfxMessageBox(TEXT("请填写好分区设置哈"));
		return;
	}


	//开始处理  剩余容量 的赋值操作
	if (restSizeNum > 0)	//说明有  选过  剩余容量
	{
		m_table[restSizeNum - 1].size = usb_size.QuadPart - allSize;
	}
	
	//处理 m_table 中的 size 值 可能出现 0 , 则除之
	//顺便 编写 确认框 文字
	CString temp;
	temp.Format(TEXT("选择U盘:  %s  总容量: %.1fG\n"), this->m_drive.GetBuffer(), (usb_size.QuadPart / 1024.0));
	if (activeNum > 0)
	{
		temp.AppendFormat(TEXT("活动分区:  第%d分区 \n"), activeNum);
	}
	else
	{
		temp.AppendFormat(TEXT("活动分区:  无 \n"));
	}
	for (int i = 0, j = 0; i < 4; i++)
	{
		if (m_table[i].size != 0 && m_table[i].size <= usb_size.QuadPart)
		{
			sprintf_s(m_table[j].type, sizeof(m_table[j].type), m_table[i].type);
			m_table[j].size = m_table[i].size;
			m_table[j].type[5] = '\0';
			m_table[j].type[6] = '\0';
			m_table[j].type[7] = '\0';
			temp.AppendFormat(TEXT("第%d分区:  %ldM  %.1fG  %s \n"), (j + 1), (long)m_table[j].size, (m_table[j].size / 1024.0), m_table[j].type);
			j++;
		}
	}

	temp.Append(TEXT("\n分区将清除U盘所有数据, 请做好备份后再确认!\n"));
	//弹出确认框 供 用户最后确认
	if (IDOK == AfxMessageBox(temp, MB_YESNO))
	{
		//经过上面检测 和 确认, 说明用户所有设置有效 , 可以分区
		//m_CreateStartDlg->Partition(this->m_drive.GetBuffer(), sizeof(m_table) / sizeof(Partition_Table), m_table, activeNum,
			//this->GetSafeHwnd(), NULL);
	}
	//CDialogEx::OnOK();
}


void CPartitionDlg::OnCbnSelchangeSelectUsb()
{
	// TODO:  在此添加控件通知处理程序代码
	CString str;
	this->m_select_usb.GetWindowText(str);
	INT pos = str.Find(TEXT(":\\"));	//因为U盘的显示格式是"G:\ 16G"
	if (pos == -1)
		this->m_drive = TEXT("");		//无效选项就为空
	else
		this->m_drive = str.Mid(pos - 1, 2);		//有效选项则变成"G:"
}


void CPartitionDlg::OnCbnDropdownSize1()
{
	// TODO:  在此添加控件通知处理程序代码
	this->SetSize(0);
}

void CPartitionDlg::OnCbnDropdownSize2()
{
	// TODO:  在此添加控件通知处理程序代码
	this->SetSize(1);
}

void CPartitionDlg::OnCbnDropdownSize3()
{
	// TODO:  在此添加控件通知处理程序代码
	this->SetSize(2);
}

void CPartitionDlg::OnCbnDropdownSize4()
{
	// TODO:  在此添加控件通知处理程序代码
	this->SetSize(3);
}


INT CPartitionDlg::SetSize(INT sizeNum)
{
	//sizeNum是m_size控件在p_size[]数组中的下标
	if (0==this->m_drive.CompareNoCase(TEXT("")))
	{
		this->p_size[sizeNum]->ResetContent();
		return FALSE;
	}

	LARGE_INTEGER usb_size = this->m_CreateStartDlg->GetUSBAllSize(this->m_drive.GetBuffer());
	usb_size.QuadPart = usb_size.QuadPart / (1024.0 * 1024.0);	//转化为MB

	CString str;
	this->p_unit[sizeNum]->GetWindowText(str);
	if (str.Find(TEXT("GB")) > -1)
	{
		this->p_size[sizeNum]->ResetContent();
		str.Format(TEXT("%.1f(全部)"), usb_size.QuadPart / 1024.0);
		this->p_size[sizeNum]->AddString(str.GetBuffer());
		this->p_size[sizeNum]->AddString(TEXT("剩余容量"));
		for (int i = 1; i <= (usb_size.QuadPart / 1024); i++)		//转化为GB
		{
			str.Format(TEXT("%d"), i);
			this->p_size[sizeNum]->AddString(str.GetBuffer());
		}
	}
	else if (str.Find(TEXT("MB")) > -1)
	{
		this->p_size[sizeNum]->ResetContent();	//清除原来内容
		str.Format(TEXT("%d(全部)"), usb_size.QuadPart);
		this->p_size[sizeNum]->AddString(str.GetBuffer());
		this->p_size[sizeNum]->AddString(TEXT_REST_SIZE);// TEXT("剩余容量"));
		for (int i = 500; i <= (usb_size.QuadPart); i += 500)
		{
			str.Format(TEXT("%d"), i);
			this->p_size[sizeNum]->AddString(str.GetBuffer());
		}
	}
	else
		return FALSE;

	return TRUE;
}


void CPartitionDlg::OnBnClickedActive1()
{
	// TODO:  在此添加控件通知处理程序代码
	this->SetActive(0);
}

void CPartitionDlg::OnBnClickedActive2()
{
	// TODO:  在此添加控件通知处理程序代码
	this->SetActive(1);
}

void CPartitionDlg::OnBnClickedActive3()
{
	// TODO:  在此添加控件通知处理程序代码
	this->SetActive(2);
}

void CPartitionDlg::OnBnClickedActive4()
{
	// TODO:  在此添加控件通知处理程序代码
	this->SetActive(3);
}

INT CPartitionDlg::SetActive(INT activeNum)
{
	for (int i = 0; i < 4; i++)
	{
		if (i == activeNum)
			this->p_active[activeNum]->SetCheck(!(this->p_active[activeNum]->GetCheck()));
		else
			this->p_active[i]->SetCheck(FALSE);
	}
	return TRUE;
}

BOOL CPartitionDlg::IsNumber(CString& str)
{
	CString m_str = str;
	m_str = m_str.Trim();
	m_str = m_str.SpanIncluding(TEXT(".0123456789"));
	if (m_str.GetBuffer() != str.Trim())
		return FALSE;
	if (m_str.CompareNoCase(TEXT("")) == 0
		|| m_str.Left(1).CompareNoCase(TEXT(".")) == 0
		|| m_str.Right(1).CompareNoCase(TEXT(".")) == 0)
		return FALSE;

	return TRUE;
}



void CPartitionDlg::OnBnClickedCancel()
{
	// TODO:  在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}
