#pragma once


// configDlg �Ի���

class configDlg : public CDialogEx
{
	DECLARE_DYNAMIC(configDlg)

public:
	configDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~configDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_Config };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString diskSymbol;
	int diskSize;
	afx_msg void OnBnClickedOk();
};
