#pragma once


// removeDlg �Ի���

class removeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(removeDlg)

public:
	removeDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~removeDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_Remove };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
};
