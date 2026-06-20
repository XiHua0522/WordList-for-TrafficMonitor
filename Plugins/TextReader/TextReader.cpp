#include "pch.h"
#include "TextReader.h"
#include "DataManager.h"
#include "OptionsDlg.h"
#include "OptionsContainerDlg.h"
#include "Common.h"
#include <sstream>
#include <iomanip>

CTextReader CTextReader::m_instance;

CTextReader::CTextReader()
{
}

CTextReader& CTextReader::Instance()
{
    return m_instance;
}

IPluginItem* CTextReader::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_item;
    default:
        break;
    }
    return nullptr;
}

const wchar_t* CTextReader::GetTooltipInfo()
{
    static std::wstring tooltip_info;
    tooltip_info.clear();
    if (g_data.m_setting_data.show_in_tooltips)
    {
        std::wstringstream stream;
        stream << g_data.StringRes(IDS_OPEN_FILE).GetString() << L": " << g_data.m_setting_data.file_path << L"\r\n";
        std::wstring cur_chapter = g_data.GetChapter().GetCurrentChapterTitle();
        if (!cur_chapter.empty())
            stream << g_data.StringRes(IDS_CHAPTER).GetString() << L": " << cur_chapter << L"\r\n";
        stream << g_data.StringRes(IDS_READ_POSITION).GetString() << L": " << std::setiosflags(std::ios::fixed) << std::setprecision(2) << static_cast<double>(g_data.m_setting_data.current_position) * 100 / g_data.GetTextContexts().size() << "%";
        tooltip_info = stream.str();
    }
    return tooltip_info.c_str();
}

void CTextReader::DataRequired()
{
    //TODO: 在此添加获取监控数据的代码
}

ITMPlugin::OptionReturn CTextReader::ShowOptionsDialog(void* hParent)
{
    CWnd* pParent = CWnd::FromHandle((HWND)hParent);
    int result = ShowOptionsDlg(pParent);
    return (result == IDOK ? ITMPlugin::OR_OPTION_CHANGED : ITMPlugin::OR_OPTION_UNCHANGED);
}

const wchar_t* CTextReader::GetInfo(PluginInfoIndex index)
{
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        return g_data.StringRes(IDS_PLUGIN_NAME).GetString();
    case TMI_DESCRIPTION:
        return g_data.StringRes(IDS_PLUGIN_DESCRIPTION).GetString();
    case TMI_AUTHOR:
        return L"zhongyang219";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by Zhong Yang 2025";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/zhongyang219/TrafficMonitorPlugins";
        break;
    case TMI_VERSION:
        return L"1.02";
    default:
        break;
    }
    return L"";
}

void CTextReader::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        //从配置文件读取配置
        g_data.LoadConfig(std::wstring(data));
        SetAutoReadTimer();

        //设置阅读进度定时保存
        SetTimer(NULL, 1009, 10000, [](HWND, UINT, UINT_PTR, DWORD) {
            g_data.SaveReadPosition();
        });

        //启动一个定时器用于检查文件变化
        SetTimer(NULL, 1542, 1000, [](HWND, UINT, UINT_PTR, DWORD) {
            g_data.CheckFileChange();
        });

        break;
    default:
        break;
    }
}


void CTextReader::ShowContextMenu(CWnd* pWnd)
{
    LoadContextMenu();
    CMenu* context_menu = m_menu.GetSubMenu(0);

    //设置菜单状态
    context_menu->CheckMenuItem(ID_START_AUTO_READ, MF_BYCOMMAND | (g_data.m_setting_data.auto_read ? MF_CHECKED : MF_UNCHECKED));
    context_menu->CheckMenuItem(ID_AUTO_SPEAK, MF_BYCOMMAND | (g_data.m_setting_data.auto_speak ? MF_CHECKED : MF_UNCHECKED));
    context_menu->CheckMenuItem(ID_HIDE, MF_BYCOMMAND | (g_data.m_boss_key_pressed ? MF_CHECKED : MF_UNCHECKED));

    if (context_menu != nullptr)
    {
        CPoint point1;
        GetCursorPos(&point1);
        DWORD id = context_menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point1.x, point1.y, pWnd);
        //点击了“选项”
        if (id == ID_OPTIONS)
        {
            ShowOptionsDlg(pWnd);
        }
        //点击了“章节”
        else if (id == ID_CHAPTERS)
        {
            ShowOptionsDlg(pWnd, 1);
        }
        //点击了“书签”
        else if (id == ID_BOOKMARKS)
        {
            ShowOptionsDlg(pWnd, 2);
        }
        // ID_DELETE_BOOKMARK_CMD removed; deletion handled in bookmark dialog via Delete key
        //点击了“上一页”
        else if (id == ID_PREVIOUS)
        {
            g_data.PageUp();
        }
        //点击了“下一页”
        else if (id == ID_NEXT)
        {
            g_data.PageDown();
        }
        //点击了“添加书签”
        else if (id == ID_ADD_BOOKMARK)
        {
            g_data.AddBookmark();
        }
        //点击了“自动翻页”
        else if (id == ID_START_AUTO_READ)
        {
            g_data.m_setting_data.auto_read = !g_data.m_setting_data.auto_read;
            SetAutoReadTimer();
        }
        //点击了“自动朗读”
        else if (id == ID_AUTO_SPEAK)
        {
            g_data.m_setting_data.auto_speak = !g_data.m_setting_data.auto_speak;
        }
        //点击了“隐藏”
        else if (id == ID_HIDE)
        {
            g_data.m_boss_key_pressed = !g_data.m_boss_key_pressed;
        }
    }

}

void CTextReader::SetAutoReadTimer()
{
    if (m_timer_id != 0)
        KillTimer(NULL, m_timer_id);
    if (g_data.m_setting_data.auto_read)
    {
        m_timer_id = SetTimer(NULL, 1008, static_cast<UINT>(g_data.m_setting_data.auto_read_timer_interval), [](HWND, UINT, UINT_PTR, DWORD)
            {
                g_data.PageDown();
                if (g_data.m_setting_data.auto_speak)
                {
                    CTextReader::Instance().SpeakCurrent();
                }
            });
    }
}

void* CTextReader::GetPluginIcon()
{
    return g_data.GetIcon(IDI_ICON1);
}


int CTextReader::GetCommandCount()
{
    return CMD_MAX;
}

const wchar_t* CTextReader::GetCommandName(int command_index)
{
    CommandIndex index = static_cast<CommandIndex>(command_index);
    switch (index)
    {
    case CTextReader::CMD_PRE:
        return g_data.StringRes(IDS_MENU_PREVIOUS);
    case CTextReader::CMD_NEXT:
        return g_data.StringRes(IDS_MENU_NEXT);
    case CTextReader::CMD_ADD_BOOKMARK:
        return g_data.StringRes(IDS_MENU_ADD_BOOKMARK);
    case CTextReader::CMD_AUTO_PAGE:
        return g_data.StringRes(IDS_MENU_AUTO_PAGE);
    case CTextReader::CMD_HIDE:
        return g_data.StringRes(IDS_MENU_HIDE);
    case CTextReader::CMD_CHAPTER:
        return g_data.StringRes(IDS_MENU_CHAPTER);
    case CTextReader::CMD_BOOKMARK:
        return g_data.StringRes(IDS_MENU_BOOKMARK);
    case CTextReader::CMD_AUTO_SPEAK:
        return g_data.StringRes(IDS_MENU_AUTO_SPEAK);
    }
    return nullptr;
}

void* CTextReader::GetCommandIcon(int command_index)
{
    CommandIndex index = static_cast<CommandIndex>(command_index);
    switch (index)
    {
    case CTextReader::CMD_PRE:
        return g_data.GetIcon(IDI_PREVIOUS);
    case CTextReader::CMD_NEXT:
        return g_data.GetIcon(IDI_NEXT);
    case CTextReader::CMD_ADD_BOOKMARK:
        return g_data.GetIcon(IDI_ADD_BOOKMARK);
    //case CTextReader::CMD_AUTO_PAGE:
    //    return g_data.GetIcon(IDI_AUTO_PAGE);
    //case CTextReader::CMD_HIDE:
    //    return g_data.GetIcon(IDI_HIDE);
    case CTextReader::CMD_CHAPTER:
        return g_data.GetIcon(IDI_CHAPTER);
    case CTextReader::CMD_BOOKMARK:
        return g_data.GetIcon(IDI_BOOKMARK);
    default:
        break;
    }
    return nullptr;
}

void CTextReader::OnPluginCommand(int command_index, void* hWnd, void* para)
{
    CWnd* pWnd = CWnd::FromHandle((HWND)hWnd);
    CommandIndex index = static_cast<CommandIndex>(command_index);
    switch (index)
    {
    case CTextReader::CMD_PRE:
        g_data.PageUp();
        break;
    case CTextReader::CMD_NEXT:
        g_data.PageDown();
        break;
    case CTextReader::CMD_ADD_BOOKMARK:
        g_data.AddBookmark();
        break;
    case CTextReader::CMD_AUTO_PAGE:
        g_data.m_setting_data.auto_read = !g_data.m_setting_data.auto_read;
        SetAutoReadTimer();
        break;
    case CTextReader::CMD_HIDE:
        g_data.m_boss_key_pressed = !g_data.m_boss_key_pressed;
        break;
    case CTextReader::CMD_CHAPTER:
        ShowOptionsDlg(pWnd, 1);
        break;
    case CTextReader::CMD_BOOKMARK:
        ShowOptionsDlg(pWnd, 2);
        break;
    case CTextReader::CMD_AUTO_SPEAK:
        g_data.m_setting_data.auto_speak = !g_data.m_setting_data.auto_speak;
        break;
    default:
        break;
    }
}

int CTextReader::IsCommandChecked(int command_index)
{
    CommandIndex index = static_cast<CommandIndex>(command_index);
    switch (index)
    {
    case CMD_AUTO_PAGE:
        return g_data.m_setting_data.auto_read;
    case CMD_AUTO_SPEAK:
        return g_data.m_setting_data.auto_speak;
    case CMD_HIDE:
        return g_data.m_boss_key_pressed;
    }
    return 0;
}

int CTextReader::ShowOptionsDlg(CWnd* pParent, int cur_tab /*= 0*/)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    COptionsContainerDlg dlg(cur_tab, pParent);
    dlg.m_options_dlg.m_data = g_data.m_setting_data;
    auto rtn = dlg.DoModal();
    if (rtn == IDOK)
    {
        bool auto_read_changed{ g_data.m_setting_data.auto_read != dlg.m_options_dlg.m_data.auto_read || g_data.m_setting_data.auto_read_timer_interval != dlg.m_options_dlg.m_data.auto_read_timer_interval };
        bool wordlist_changed{ g_data.m_setting_data.use_word_list != dlg.m_options_dlg.m_data.use_word_list };

        g_data.m_setting_data = dlg.m_options_dlg.m_data;
        if (wordlist_changed && g_data.m_setting_data.use_word_list && !g_data.m_setting_data.file_path.empty())
        {
            // 重新加载以解析词表
            g_data.LoadTextContents(g_data.m_setting_data.file_path.c_str());
        }
        int chapter_position = dlg.m_chapter_dlg.GetSelectedPosition();
        if (chapter_position >= 0)
        {
            if (g_data.IsWordList())
            {
                // 在单词表模式下，按章节文字偏移设置词索引
                g_data.SetWordIndexByTextPos(chapter_position);
            }
            else
            {
                g_data.m_setting_data.current_position = chapter_position;
            }
        }
        int boolmark_position = dlg.m_bookmark_dlg.GetSelectedPosition();
        if (boolmark_position >= 0)
        {
            if (g_data.IsWordList())
            {
                // 在单词表模式下，将书签文字偏移映射到词索引
                g_data.SetWordIndexByTextPos(boolmark_position);
            }
            else
            {
                g_data.m_setting_data.current_position = boolmark_position;
            }
        }
        
        if (auto_read_changed)
        {
            SetAutoReadTimer();
        }

        g_data.SaveConfig();
    }
    return static_cast<int>(rtn);
}

void CTextReader::SpeakCurrent()
{
    if (!g_data.m_setting_data.auto_speak)
        return;
    double speed = static_cast<double>(g_data.m_setting_data.speech_speed_percent) / 100.0;
    if (g_data.IsWordList())
    {
        auto p = g_data.GetCurrentWord();
        if (!p.first.empty())
        {
            int repeat = g_data.m_setting_data.auto_speak_repeat;
            if (repeat < 1) repeat = 1;
            if (repeat > 10) repeat = 10;

            // 清洗原文：去掉括号及其内部内容（如 ちゅうごくじん（中国人））
            std::wstring word = p.first;
            size_t b1 = word.find(L'（');
            size_t b2 = word.find(L'(');
            size_t cut = std::wstring::npos;
            if (b1 != std::wstring::npos && b2 != std::wstring::npos)
                cut = (b1 < b2) ? b1 : b2;
            else if (b1 != std::wstring::npos)
                cut = b1;
            else if (b2 != std::wstring::npos)
                cut = b2;
            if (cut != std::wstring::npos)
                word = word.substr(0, cut);
            while (!word.empty() && (word.back() == L' ' || word.back() == L'\t'))
                word.pop_back();

            // 清洗翻译：去掉开头的词性标记（如 〔名〕 【名】 [名] （名））
            std::wstring meaning = p.second;
            auto strip_markers = [](std::wstring s) -> std::wstring {
                bool changed = true;
                while (changed && !s.empty()) {
                    changed = false;
                    while (!s.empty() && (s[0] == L' ' || s[0] == L'\t')) s = s.substr(1);
                    struct Pair { wchar_t open; wchar_t close; };
                    Pair pairs[] = { {L'〔', L'〕'}, {L'【', L'】'}, {L'[', L']'}, {L'（', L'）'}, {L'(', L')'} };
                    for (const auto& pr : pairs) {
                        if (!s.empty() && s[0] == pr.open) {
                            size_t end = s.find(pr.close);
                            if (end != std::wstring::npos) {
                                s = s.substr(end + 1);
                                changed = true;
                                break;
                            }
                        }
                    }
                }
                while (!s.empty() && (s[0] == L' ' || s[0] == L'\t')) s = s.substr(1);
                return s;
            };
            meaning = strip_markers(meaning);

            if (!meaning.empty())
            {
                double word_speed = speed * 0.85; // 单词发音稍慢一点
                if (word_speed < 0.1) word_speed = 0.1;
                CCommon::SpeakPair(word, 0x0411, meaning, 0x0804, word_speed, speed, repeat, 1, 1);
            }
            else
                CCommon::SpeakText(word, 0x0411, speed, repeat);
        }
    }
    else
    {
        const std::wstring& text = g_data.GetTextContexts();
        int pos = g_data.m_setting_data.current_position;
        int step = g_data.m_page_step;
        if (step <= 0) step = 1;
        if (pos >= 0 && pos < static_cast<int>(text.size()))
        {
            std::wstring to_speak = text.substr(pos, step);
            while (!to_speak.empty() && (to_speak.back() == L'\n' || to_speak.back() == L'\r' || to_speak.back() == L' ' || to_speak.back() == L'\t'))
                to_speak.pop_back();
            if (!to_speak.empty())
            {
                CCommon::SpeakText(to_speak, 0, speed, 1);
            }
        }
    }
}

void CTextReader::LoadContextMenu()
{
    if (m_menu.m_hMenu == NULL)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_menu.LoadMenu(IDR_MENU1);
    }

}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CTextReader::Instance();
}
