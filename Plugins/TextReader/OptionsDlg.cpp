// OptionsDlg.cpp: 实现文件
//

#include "pch.h"
#include "TextReader.h"
#include "OptionsDlg.h"
#include "DataManager.h"

// COptionsDlg 对话框

IMPLEMENT_DYNAMIC(COptionsDlg, CDialogEx)

COptionsDlg::COptionsDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_OPTIONS_DIALOG, pParent)
{
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::EnableDlgControl(UINT id, bool enable)
{
    CWnd* pCtrl = GetDlgItem(id);
    if (pCtrl != nullptr)
        pCtrl->EnableWindow(enable);
}

void COptionsDlg::EnableControls()
{
    bool mouse_wheel_enable = (IsDlgButtonChecked(IDC_MOUSE_WHEEL_ENABLE_CHECK) != 0);
    bool read_whole_page = (IsDlgButtonChecked(IDC_WHEEL_READ_PAGE_CHECK) != 0);
    EnableDlgControl(IDC_WHEEL_CHARACTOR_NUM_EDIT, mouse_wheel_enable && !read_whole_page);
    EnableDlgControl(IDC_WHEEL_READ_PAGE_CHECK, mouse_wheel_enable);

    bool use_word_list = (IsDlgButtonChecked(IDC_USE_WORDLIST_CHECK) != 0);
    EnableDlgControl(IDC_AUTO_SPEAK_REPEAT_EDIT, use_word_list && m_data.auto_speak);
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BROWSE_BUTTON, &COptionsDlg::OnBnClickedBrowseButton)
    ON_WM_GETMINMAXINFO()
    ON_BN_CLICKED(IDC_MOUSE_WHEEL_ENABLE_CHECK, &COptionsDlg::OnBnClickedMouseWheelEnableCheck)
    ON_BN_CLICKED(IDC_WHEEL_READ_PAGE_CHECK, &COptionsDlg::OnBnClickedWheelReadPageCheck)
    ON_BN_CLICKED(IDC_USE_WORDLIST_CHECK, &COptionsDlg::OnBnClickedUseWordListCheck)
END_MESSAGE_MAP()


// COptionsDlg 消息处理程序


BOOL COptionsDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    SetBackgroundColor(RGB(255, 255, 255));

    m_file_path_ori = m_data.file_path;

    SetDlgItemText(IDC_FILE_PATH_EDIT, m_data.file_path.c_str());
    CString str;
    str.Format(_T("%d"), m_data.current_position);
    SetDlgItemText(IDC_READ_POSITION_EDIT, str);
    str.Format(_T("%d"), m_data.window_width);
    SetDlgItemText(IDC_WINDOW_WIDTH_EDIT, str);
    str.Format(_T("%d"), static_cast<int>(g_data.GetTextContexts().size()));
    SetDlgItemText(IDC_TOTAL_CHAR_STATIC, str);
    double percent{};
    if (g_data.GetTextContexts().size() != 0)
        percent = static_cast<double>(m_data.current_position) * 100 / g_data.GetTextContexts().size();
    str.Format(_T("(%.2f%%)"), percent);
    SetDlgItemText(IDC_PERCENT_STATIC, str);
    CheckDlgButton(IDC_SHOT_TOOLTIP_CHECK, m_data.show_in_tooltips);
    CheckDlgButton(IDC_ENABLE_MULTI_LINE_CHECK, m_data.enable_mulit_line);
    CheckDlgButton(IDC_HIDE_WHEN_LOSE_FOCUS_CHECK, m_data.hide_when_lose_focus);

    CheckDlgButton(IDC_AUTO_READ_CHECK, m_data.auto_read);
    str.Format(_T("%d"), m_data.auto_read_timer_interval);
    SetDlgItemText(IDC_AUTO_READ_INTERVAL_EDIT, str);

    // 语速（百分比），显示为浮点，如 1.0 表示 100%
    CString speedStr;
    double speed = static_cast<double>(m_data.speech_speed_percent) / 100.0;
    speedStr.Format(_T("%.2f"), speed);
    SetDlgItemText(IDC_SPEECH_SPEED_EDIT, speedStr);

    // 发音重复次数
    CString repeatStr;
    repeatStr.Format(_T("%d"), m_data.speech_repeat_count);
    SetDlgItemText(IDC_SPEECH_REPEAT_EDIT, repeatStr);

    CheckDlgButton(IDC_AUTO_DECODE_CHECK, m_data.auto_decode_base64);
    CheckDlgButton(IDC_USE_WORDLIST_CHECK, m_data.use_word_list);
    CheckDlgButton(IDC_USE_OWN_CONTEXT_MENU_CHECK, m_data.use_own_context_menu);
    CheckDlgButton(IDC_RESTART_AT_END_CHECK, m_data.restart_at_end);
    CheckDlgButton(IDC_AUTO_RELOAD_CHECK, m_data.auto_reload_when_file_changed);
    CheckDlgButton(IDC_MOUSE_WHEEL_ENABLE_CHECK, m_data.mouse_wheel_read);
    SetDlgItemText(IDC_WHEEL_CHARACTOR_NUM_EDIT, std::to_wstring(m_data.mouse_wheel_charactors).c_str());
    CheckDlgButton(IDC_WHEEL_READ_PAGE_CHECK, m_data.mouse_wheel_read_page);

    CString autoSpeakRepeatStr;
    autoSpeakRepeatStr.Format(_T("%d"), m_data.auto_speak_repeat);
    SetDlgItemText(IDC_AUTO_SPEAK_REPEAT_EDIT, autoSpeakRepeatStr);

    EnableControls();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}


void COptionsDlg::OnBnClickedBrowseButton()
{
    // TODO: 在此添加控件通知处理程序代码
    CFileDialog fileDlg(TRUE, _T("txt"), NULL, 0, g_data.StringRes(IDS_FILTER), this);
    if (fileDlg.DoModal() == IDOK)
    {
        m_data.file_path = fileDlg.GetPathName().GetString();
        SetDlgItemText(IDC_FILE_PATH_EDIT, m_data.file_path.c_str());
        if (m_data.file_path != m_file_path_ori)
        {
            SetDlgItemText(IDC_READ_POSITION_EDIT, _T("0"));    //打开新文件时，将阅读进度置为0
        }
    }

}


void COptionsDlg::OnOK()
{
    // TODO: 在此添加专用代码和/或调用基类

    //载入文本
    if (m_data.file_path != m_file_path_ori)
        g_data.LoadTextContents(m_data.file_path.c_str());
    CString current_position_str;
    GetDlgItemText(IDC_READ_POSITION_EDIT, current_position_str);
    m_data.current_position = _ttoi(current_position_str.GetString());
    CString window_width_str;
    GetDlgItemText(IDC_WINDOW_WIDTH_EDIT, window_width_str);
    m_data.window_width = _ttoi(window_width_str);
    m_data.show_in_tooltips = (IsDlgButtonChecked(IDC_SHOT_TOOLTIP_CHECK) != 0);
    m_data.enable_mulit_line = (IsDlgButtonChecked(IDC_ENABLE_MULTI_LINE_CHECK) != 0);
    m_data.hide_when_lose_focus = (IsDlgButtonChecked(IDC_HIDE_WHEN_LOSE_FOCUS_CHECK) != 0);

    m_data.auto_read = (IsDlgButtonChecked(IDC_AUTO_READ_CHECK) != 0);
    CString auto_read_interval;
    GetDlgItemText(IDC_AUTO_READ_INTERVAL_EDIT, auto_read_interval);
    m_data.auto_read_timer_interval = _ttoi(auto_read_interval);

    m_data.auto_decode_base64 = (IsDlgButtonChecked(IDC_AUTO_DECODE_CHECK) != 0);
    m_data.use_word_list = (IsDlgButtonChecked(IDC_USE_WORDLIST_CHECK) != 0);
    m_data.use_own_context_menu = (IsDlgButtonChecked(IDC_USE_OWN_CONTEXT_MENU_CHECK) != 0);
    m_data.restart_at_end = (IsDlgButtonChecked(IDC_RESTART_AT_END_CHECK) != 0);
    m_data.auto_reload_when_file_changed = (IsDlgButtonChecked(IDC_AUTO_RELOAD_CHECK) != 0);

    m_data.mouse_wheel_read = (IsDlgButtonChecked(IDC_MOUSE_WHEEL_ENABLE_CHECK) != 0);
    CString mouse_wheel_charactors;
    GetDlgItemText(IDC_WHEEL_CHARACTOR_NUM_EDIT, mouse_wheel_charactors);
    m_data.mouse_wheel_charactors = _ttoi(mouse_wheel_charactors);
    m_data.mouse_wheel_read_page = (IsDlgButtonChecked(IDC_WHEEL_READ_PAGE_CHECK) != 0);

    // 读取语速输入，允许小数，转换为百分比整数
    CString speedStr2;
    GetDlgItemText(IDC_SPEECH_SPEED_EDIT, speedStr2);
    double speedVal = _tstof(speedStr2);
    if (speedVal < 0.1) speedVal = 0.1;
    if (speedVal > 3.0) speedVal = 3.0;
    m_data.speech_speed_percent = static_cast<int>(speedVal * 100.0);

    // 读取发音重复次数
    CString repeatStr2;
    GetDlgItemText(IDC_SPEECH_REPEAT_EDIT, repeatStr2);
    int repeatVal = _ttoi(repeatStr2);
    if (repeatVal < 1) repeatVal = 1;
    if (repeatVal > 10) repeatVal = 10;
    m_data.speech_repeat_count = repeatVal;

    // 读取自动朗读重复次数
    CString autoSpeakRepeatStr2;
    GetDlgItemText(IDC_AUTO_SPEAK_REPEAT_EDIT, autoSpeakRepeatStr2);
    int autoSpeakRepeatVal = _ttoi(autoSpeakRepeatStr2);
    if (autoSpeakRepeatVal < 1) autoSpeakRepeatVal = 1;
    if (autoSpeakRepeatVal > 10) autoSpeakRepeatVal = 10;
    m_data.auto_speak_repeat = autoSpeakRepeatVal;

    CDialogEx::OnOK();
}


void COptionsDlg::OnBnClickedMouseWheelEnableCheck()
{
    EnableControls();
}


void COptionsDlg::OnBnClickedWheelReadPageCheck()
{
    EnableControls();
}


void COptionsDlg::OnBnClickedUseWordListCheck()
{
    EnableControls();
}
