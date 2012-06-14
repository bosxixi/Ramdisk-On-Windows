
// driverSetupDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "driverSetup.h"
#include "driverSetupDlg.h"
#include "afxdialogex.h"
#include "installDlg.h"
#include "configDlg.h"
#include "removeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CdriverSetupDlg �Ի���




CdriverSetupDlg::CdriverSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CdriverSetupDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CdriverSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CdriverSetupDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CdriverSetupDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CdriverSetupDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CdriverSetupDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CdriverSetupDlg ��Ϣ�������

BOOL CdriverSetupDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CdriverSetupDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CdriverSetupDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CdriverSetupDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	installDlg dlg1;
	dlg1.DoModal();
	Invalidate(true);
}


void CdriverSetupDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	configDlg dlg2;
	dlg2.DoModal();
	Invalidate(true);
	
}


void CdriverSetupDlg::OnBnClickedButton3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	removeDlg dlg3;
	dlg3.DoModal();
	Invalidate(true);
}
