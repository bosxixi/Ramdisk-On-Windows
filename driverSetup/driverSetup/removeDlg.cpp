// removeDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "driverSetup.h"
#include "removeDlg.h"
#include "afxdialogex.h"
#include "Windows.h"

// removeDlg �Ի���

IMPLEMENT_DYNAMIC(removeDlg, CDialogEx)

removeDlg::removeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(removeDlg::IDD, pParent)
{

}

removeDlg::~removeDlg()
{
}

void removeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(removeDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &removeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &removeDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// removeDlg ��Ϣ�������


void removeDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	WinExec("ramdisk\\32\\devcon.exe remove ramdisk",SW_SHOW);
	MessageBox(_T("ж�سɹ���"));

	CDialogEx::OnOK();
}


void removeDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	WinExec("ramdisk\\64\\devcon.exe remove ramdisk",SW_SHOW);
	MessageBox(_T("ж�سɹ���"));

	CDialogEx::OnOK();

}
