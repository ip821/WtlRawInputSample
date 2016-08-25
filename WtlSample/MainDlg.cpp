// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
    return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
    UIUpdateChildWindows();
    return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_listBox.Attach(GetDlgItem(IDC_LIST_KEYBOARDS));

    // center the dialog on the screen
    CenterWindow();

    // set icons
    HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
    SetIcon(hIcon, TRUE);
    HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
    SetIcon(hIconSmall, FALSE);

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    UIAddChildWindowContainer(m_hWnd);

    UINT cDeviceNum = 0;
    auto res = GetRawInputDeviceList(NULL, &cDeviceNum, sizeof(RAWINPUTDEVICELIST));
    vector<RAWINPUTDEVICELIST> vDeviceList(cDeviceNum);
    res = GetRawInputDeviceList(&vDeviceList[0], &cDeviceNum, sizeof(RAWINPUTDEVICELIST));

    vector<RAWINPUTDEVICELIST> vKeyboards;
    for(auto& it : vDeviceList)
    {
        //if (it.dwType & RIM_TYPEKEYBOARD)
            vKeyboards.push_back(it);
    }

    for (auto& it : vKeyboards)
    {
        UINT uiSize = 0;
        res = GetRawInputDeviceInfo(it.hDevice, RIDI_DEVICENAME, nullptr, &uiSize);
        if (res != S_OK)
        {
            auto dwError = GetLastError();
            dwError;
        }
        CString strName;
        {
            auto lpBuffer = strName.GetBuffer(uiSize);
            res = GetRawInputDeviceInfo(it.hDevice, RIDI_DEVICENAME, lpBuffer, &uiSize);
            strName.ReleaseBuffer();
        }

        if (strName.IsEmpty())
            continue;

        strName = strName.Mid(4);

        vector<CString> vDeviceNameTokens;
        StrSplit(strName, L"#", vDeviceNameTokens);
        CString strQueryPath;
        strQueryPath.Format(L"System\\CurrentControlSet\\Enum\\%s\\%s\\%s", vDeviceNameTokens[0], vDeviceNameTokens[1], vDeviceNameTokens[2]);

        CRegKey reg;
        reg.Open(HKEY_LOCAL_MACHINE, strQueryPath, GENERIC_READ);
        ULONG ulCount = 0;
        reg.QueryStringValue(L"DeviceDesc", nullptr, &ulCount);
        CString strDesc;
        {
            auto lpBuffer = strDesc.GetBuffer(ulCount);
            reg.QueryStringValue(L"DeviceDesc", lpBuffer, &ulCount);
            strDesc.ReleaseBuffer();
        }

        vector<CString> vDescTokens;
        StrSplit(strDesc, L";", vDescTokens);
        m_listBox.AddString(vDescTokens[1]);
    }

    return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // unregister message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

    return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // TODO: Add validation code 
    CloseDialog(wID);
    return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CloseDialog(wID);
    return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
    DestroyWindow();
    ::PostQuitMessage(nVal);
}