// PartitionDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "USB_Viewer.h"
#include "PartitionDlg.h"
#include "afxdialogex.h"
#include "Define.h"
#include "CreateStartDlg.h"

#define	TEXT_REST_SIZE	TEXT("ʣ������")
#define	TEXT_ALL_SIZE	TEXT("ȫ��")


// CPartitionDlg �Ի���
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
ON_MESSAGE(WM_MYMSG, &CPartitionDlg::OnMymsg)
ON_WM_DEVICECHANGE()		//���ڽ����豸(U��)�䶯, �Ѹ��½������Ϣ
ON_WM_TIMER()
END_MESSAGE_MAP()


// CPartitionDlg ��Ϣ�������


BOOL CPartitionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
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


	this->m_USB_ViewerDlg->GetUSB(this, IDC_SELECT_USB);	//��������U��
	this->OnCbnSelchangeSelectUsb();	//ʹ����this->m_drive;
	for (int i = 0; i < 4; i++)
	{
		this->p_file_system[i]->AddString(TEXT("NTFS"));
		this->p_file_system[i]->AddString(TEXT("FAT32"));

		this->p_unit[i]->AddString(TEXT("GB"));
		this->p_unit[i]->AddString(TEXT("MB"));
	}
	this->p_file_system[0]->SetCurSel(0);
	this->p_unit[0]->SetCurSel(0);

	this->m_FormatState = FALSE;	//��û�з���

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣:  OCX ����ҳӦ���� FALSE
}

void CPartitionDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (0 == this->m_drive.CompareNoCase(TEXT("")))
	{
		AfxMessageBox(TEXT("��, ��ѡ��U�̹� ^_^"));
		return;
	}

	if (this->m_FormatState)
	{
		AfxMessageBox(TEXT("����ing��, ���Ժ�^_^"));
		return;
	}


/*
	CString size, file_system, unit;
	INT restSizeNum = 0, activeNum = 0;		//��ʾ �ڼ��� ѡ��ʣ�������ķ��� �� �ڼ��� ���û�����ķ��� 
	for (int i = 0, j = 0; i < 4; i++)
	{
		this->p_size[i]->GetWindowText(size);
		if (this->p_size[i]->GetCurSel() == 1 && size.Find(TEXT_REST_SIZE) > -1)
		{
			restSizeNum = i + 1;
			this->p_unit[i]->SetCurSel(0);	//Ϊ�˷�������ļ���߼�
			if (++j > 1)
			{
				restSizeNum = 0;
				AfxMessageBox(TEXT("ʣ������ ֻ�ָܷ�һ��������"));
				return;
			}
		}
	}*/

	CString size, file_system, unit, temp;
	Partition_Table m_table[4] ={0};
	bool isActive[4] = {0};		//�ֱ��Ӧ�ĸ������Ƿ�Ϊ�����
	LARGE_INTEGER usb_size;		//u��������
	ULONG64 allSize = 0;	//���з����ܴ�С
	//restSizeNum��ʾ �ڼ��� ѡ��ʣ�������ķ��� ,activeNum��ڼ��� ���û�����ķ��� ;	
	//noSizeCount �� û����д  ��С �ķ����ļ���, restSizeCount �� ѡ�� ʣ������ �ķ�������, allSizeCount �� ѡ�� ȫ������ �ķ�������
	INT noSizeCount = 0, restSizeCount = 0, allSizeCount = 0, restSizeNum = 0, activeNum = 0;
	//MB����λ
	usb_size.QuadPart = (LONGLONG)(this->m_CreateStartDlg->GetUSBAllSize(this->m_drive.GetBuffer()).QuadPart / (1024.0 * 1024.0));
	for (int i = 0; i < 4; i++)
	{
		this->p_size[i]->GetWindowText(size);

		if (!*(size.GetBuffer()))
		{
			noSizeCount++;		//����  û����д ��С �ķ�������
			continue;
		}

		if (this->p_size[i]->GetCurSel() == 0 && size.Find(TEXT_ALL_SIZE) > -1)		//ѡ����  ȫ������ Ҳ�Ϸ�
		{
			if (++allSizeCount > 1)
			{
				AfxMessageBox(TEXT("����һ����������С��ѡ���� U�̵�ȫ������Ŷ^_^"));
				return;
			}
			this->p_unit[i]->SetCurSel(0);		//���������� û�� ��д ��λ
		}
		else if (this->p_size[i]->GetCurSel() == 1 && size.Find(TEXT_REST_SIZE) > -1)		//ѡ����  ʣ������ Ҳ�Ϸ�
		{
			if (++restSizeCount > 1)
			{
				restSizeNum = 0;
				AfxMessageBox(TEXT("����һ����������С��ѡ���� U�̵�ʣ������Ŷ^_^"));
				return;
			}
			restSizeNum = i + 1;		//restSizeNum��ʾ �ڼ��� ѡ��ʣ�������ķ���
			this->p_unit[i]->SetCurSel(0);		//���������� û�� ��д ��λ
		}

		if (this->IsNumber(size) 
			|| (this->p_size[i]->GetCurSel() == 0 && size.Find(TEXT_ALL_SIZE) > -1)		//ѡ����  ȫ������ Ҳ�Ϸ�
			|| (this->p_size[i]->GetCurSel() == 1 && size.Find(TEXT_REST_SIZE) > -1))		//ѡ����  ʣ������ Ҳ�Ϸ�
		{
			this->p_file_system[i]->GetWindowText(file_system);
			if (!*(file_system.GetBuffer()))
			{
				file_system.Format(TEXT("�� %d ������ûѡ���ļ�ϵͳ����"), i + 1);
				AfxMessageBox(file_system.GetBuffer());
				return;
			}
			this->p_unit[i]->GetWindowText(unit);
			if (!*(unit.GetBuffer()))
			{
				unit.Format(TEXT("�� %d ������ûѡ�񡾵�λ����"), i + 1);
				AfxMessageBox(unit.GetBuffer());
				return;
			}
		}
		else
		{
			size.Format(TEXT("�� %d ����Ҫ������Ч����С����"), i+1);
			AfxMessageBox(size.GetBuffer());
			return;
		}

		if (this->p_active[i]->GetCheck())
		{
			isActive[i] = TRUE;		//��Ӧ������ ���־ λ��Ϊtrue
			activeNum = i + 1;		//��¼������ĺ�
		}

		
		//����������, ����˵�� ��һ�������Ϣ ��д��ȷ
		//��ʼ��д m_table  �÷�������Ϣ
		if (this->p_size[i]->GetCurSel() == 0 && size.Find(TEXT_ALL_SIZE) > -1)
		{
			m_table[i].size = (ULONG64)usb_size.QuadPart;			//ѡ����  ȫ������ Ҳ�Ϸ�
		}
		else
		{
			if (this->p_size[i]->GetCurSel() == 1 && size.Find(TEXT_REST_SIZE) > -1)
			{
				//ʣ������ �ĸ�ֵ����  ������
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
			strcpy_s(m_table[i].type, sizeof(m_table[i].type), TEXT("NTFS"));
		else
			strcpy_s(m_table[i].type, sizeof(m_table[i].type), TEXT("FAT32"));

		//������, ��дm_table  �÷�������Ϣ���


		//��ʼ�����ֹΪֹ �����ܴ�С
		if ((allSize += m_table[i].size) > (ULONG64)usb_size.QuadPart)
		{
			//ͬʱ Ҳ������  ���ѡ��  ȫ������  ������
			AfxMessageBox(TEXT("���з����ġ��ܴ�С������ U��������"));
			return;
		}
	}
	if (noSizeCount >= 4)
	{
		//˵�� �ĸ����� ��û����д ��С
		AfxMessageBox(TEXT("����д�÷������ù�"));
		return;
	}


	//��ʼ����  ʣ������ �ĸ�ֵ����
	if (restSizeNum > 0)	//˵����  ѡ��  ʣ������
	{
		m_table[restSizeNum - 1].size = usb_size.QuadPart - allSize;
	}
	
	//���� m_table �е� size ֵ ���ܳ��� 0 , ���֮
	//˳�� ��д ȷ�Ͽ� ����
	//��format֧�ֵ��ļ�ϵͳ  �������  ���
	temp.Format(TEXT("ѡ��U��:  %s  \nU��������:  %ldM  %.1fG \n\n"), this->m_drive.GetBuffer(), (long)usb_size.QuadPart, (usb_size.QuadPart / 1024.0));
	INT itemCount = 0;
	for (int i = 0; i < 4; i++)
	{
		if (m_table[i].size > 0 && m_table[i].size <= (unsigned long long)usb_size.QuadPart)
		{
			//��ʼ ��format֧�ֵ��ļ�ϵͳ  �������  ���
			if (strcmp(m_table[i].type, TEXT("NTFS")) == 0
				|| strcmp(m_table[i].type, TEXT("ntfs")) == 0)
			{
				if (m_table[i].size <= NTFS_SIZE_LIMIT)
				{
					temp.Format(TEXT("�� %d ����, NTFS��ʹ�С�� %dM Ŷ"), i + 1, NTFS_SIZE_LIMIT + 1);
					AfxMessageBox(temp);
					return;
				}
			}
			else
			{
				if (m_table[i].size <= FAT32_SIZE_LIMIT)
				{
					temp.Format(TEXT("�� %d ����, FAT32��ʹ�С�� %dM Ŷ"), i + 1, FAT32_SIZE_LIMIT + 1);
					AfxMessageBox(temp);
					return;
				}
			}
			if (isActive[i])
				activeNum = itemCount + 1;
			isActive[itemCount] = isActive[i];
			sprintf_s(m_table[itemCount].type, sizeof(m_table[itemCount].type), m_table[i].type);
			m_table[itemCount].size = m_table[i].size;
			temp.AppendFormat(TEXT("��%d����:  %ldM  %.1fG  %s \n"), (itemCount + 1), (long)m_table[itemCount].size, (m_table[itemCount].size / 1024.0), m_table[itemCount].type);
			itemCount++;
		}
		else if(m_table[i].size == 0)
		{	//�� ��С Ϊ0 , ���ǻ����ʱ, ��Ч
			if (isActive[i])
				activeNum = 0;
		}
	}

	if (activeNum > 0)
	{
		temp.AppendFormat(TEXT("�����:  ��%d���� \n\n"), activeNum);
	}
	else
	{
		temp.AppendFormat(TEXT("�����:  �� \n\n"));
	}

	temp.Append(TEXT("\n���������U����������, ������ ���� ����ȷ��!\n"));
	//����ȷ�Ͽ� �� �û����ȷ��
	if (IDYES == AfxMessageBox(temp, MB_YESNO))
	{
		//���������� �� ȷ��, ˵���û�����������Ч , ���Է���
		this->m_FormatState = TRUE;
		m_CreateStartDlg->SetMyTimer(GetSafeHwnd(), TIMER_Partition, TIMER_Partition_time, FALSE);
		if (!m_CreateStartDlg->Partition(this->m_drive.GetBuffer(), itemCount, m_table, activeNum,
			this->GetSafeHwnd(), NULL))
		{
			((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("��ʼ����"));
			m_CreateStartDlg->SetMyTimer(GetSafeHwnd(), TIMER_Partition, TIMER_Partition_time, TRUE);
			this->m_FormatState = FALSE;
		}
	}
	//CDialogEx::OnOK();
}


void CPartitionDlg::OnCbnSelchangeSelectUsb()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString str;
	this->m_select_usb.GetWindowText(str);
	INT pos = str.Find(TEXT(":\\"));	//��ΪU�̵���ʾ��ʽ��"G:\ 16G"
	if (pos == -1)
		this->m_drive = TEXT("");		//��Чѡ���Ϊ��
	else
		this->m_drive = str.Mid(pos - 1, 2);		//��Чѡ������"G:"
}


void CPartitionDlg::OnCbnDropdownSize1()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->SetSize(0);
}

void CPartitionDlg::OnCbnDropdownSize2()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->SetSize(1);
}

void CPartitionDlg::OnCbnDropdownSize3()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->SetSize(2);
}

void CPartitionDlg::OnCbnDropdownSize4()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->SetSize(3);
}


INT CPartitionDlg::SetSize(INT sizeNum)
{
	//sizeNum��m_size�ؼ���p_size[]�����е��±�
	if (0==this->m_drive.CompareNoCase(TEXT("")))
	{
		this->p_size[sizeNum]->ResetContent();
		return FALSE;
	}

	LARGE_INTEGER usb_size = this->m_CreateStartDlg->GetUSBAllSize(this->m_drive.GetBuffer());
	usb_size.QuadPart = (LONGLONG)(usb_size.QuadPart / (1024.0 * 1024.0));	//ת��ΪMB

	CString str;
	this->p_unit[sizeNum]->GetWindowText(str);
	if (str.Find(TEXT("GB")) > -1)
	{
		this->p_size[sizeNum]->ResetContent();
		str.Format(TEXT("%.1fG(ȫ��)"), usb_size.QuadPart / 1024.0);
		this->p_size[sizeNum]->AddString(str.GetBuffer());
		this->p_size[sizeNum]->AddString(TEXT_REST_SIZE);
		for (int i = 1; i <= (usb_size.QuadPart / 1024); i++)		//ת��ΪGB
		{
			str.Format(TEXT("%d"), i);
			this->p_size[sizeNum]->AddString(str.GetBuffer());
		}
	}
	else if (str.Find(TEXT("MB")) > -1)
	{
		this->p_size[sizeNum]->ResetContent();	//���ԭ������
		str.Format(TEXT("%dG(ȫ��)"), usb_size.QuadPart);
		this->p_size[sizeNum]->AddString(str.GetBuffer());
		this->p_size[sizeNum]->AddString(TEXT_REST_SIZE);// TEXT("ʣ������"));
		for (int i = 500; i <= (usb_size.QuadPart); i += 500)
		{
			str.Format(TEXT("%d"), i);
			this->p_size[sizeNum]->AddString(str.GetBuffer());
		}
	}
	else
	{
		this->p_size[sizeNum]->ResetContent();	//���ԭ������
		str.Format(TEXT("%.1fG(ȫ��)"), usb_size.QuadPart / 1024.0);
		this->p_size[sizeNum]->AddString(str.GetBuffer());
		this->p_size[sizeNum]->AddString(TEXT_REST_SIZE);
	}

	return TRUE;
}


void CPartitionDlg::OnBnClickedActive1()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->SetActive(0);
}

void CPartitionDlg::OnBnClickedActive2()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->SetActive(1);
}

void CPartitionDlg::OnBnClickedActive3()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->SetActive(2);
}

void CPartitionDlg::OnBnClickedActive4()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	this->SetActive(3);
}

INT CPartitionDlg::SetActive(INT activeNum)
{
	for (int i = 0; i < 4; i++)
	{
		if (i == activeNum)
		{
			this->p_active[activeNum]->SetCheck(!(this->p_active[activeNum]->GetCheck()));
			if (this->p_active[activeNum]->GetCheck())
				this->p_active[activeNum]->SetWindowText(TEXT("�����"));
			else
				this->p_active[activeNum]->SetWindowText(TEXT("����"));
		}
		else
		{
			this->p_active[i]->SetCheck(FALSE);
			this->p_active[i]->SetWindowText(TEXT("����"));
		}
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
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (this->m_FormatState)
	{
		AfxMessageBox(TEXT("����ing��, ���Ժ�^_^"));
		return;
	}

	CDialogEx::OnCancel();
}


afx_msg LRESULT CPartitionDlg::OnMymsg(WPARAM wParam, LPARAM lParam)
{
	if (this->m_FormatState)
	{
		((CButton*)(this->GetDlgItem(IDOK)))->SetWindowText(TEXT("��ʼ����"));
		m_CreateStartDlg->SetMyTimer(GetSafeHwnd(), TIMER_Partition, TIMER_Partition_time, TRUE);

		this->m_USB_ViewerDlg->GetUSB(this, IDC_SELECT_USB);	//���¼���U��
		if (FORMAT_OK == wParam)
		{
			AfxMessageBox("U�̷����ɹ�");
		}
		else if (FORMAT_ERROR == wParam)
		{
			AfxMessageBox("U�̷���ʧ��");
		}
		this->m_FormatState = FALSE;
	}
	return 0;
}

BOOL CPartitionDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	//nEventType����WM_DEVICECHANGE��Ϣ��wParam�����������ֵ�ο�msdn
	switch (nEventType)
	{
	case 0x8000:		// DBT_DEVICEARRIVAL==0x8000		u�̲���
	case 0x8004:		//DBT_DEVICEREMOVECOMPLETE==0x8004	u�̰γ�
		{
			this->m_USB_ViewerDlg->GetUSB(this, IDC_SELECT_USB);	//���¼���U��
			this->OnCbnSelchangeSelectUsb();	//ʹ����this->m_drive;
			return TRUE;
		}
	}
	return 0;
}


void CPartitionDlg::OnTimer(UINT_PTR nIDEvent)
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
