#pragma once


// installDlg �Ի���

class installDlg : public CDialogEx
{
	DECLARE_DYNAMIC(installDlg)

public:
	installDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~installDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_Install };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	BOOL systemIsXp;
	BOOL systemIs32;
	afx_msg void OnBnClickedOk();
};
