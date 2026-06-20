#include "pch.h"
#include "DataManager.h"
#include "Common.h"
#include <vector>
#include <sstream>
#include <fstream>
#include "../utilities/bass64/base64.h"

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
    //初始化DPI
    HDC hDC = ::GetDC(HWND_DESKTOP);
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(HWND_DESKTOP, hDC);
}

CDataManager::~CDataManager()
{
    SaveConfig();
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

static void WritePrivateProfileInt(const wchar_t* app_name, const wchar_t* key_name, int value, const wchar_t* file_path)
{
    wchar_t buff[16];
    swprintf_s(buff, L"%d", value);
    WritePrivateProfileString(app_name, key_name, buff, file_path);
}

void CDataManager::LoadConfig(const std::wstring& config_dir)
{
    //获取模块的路径
    HMODULE hModule = reinterpret_cast<HMODULE>(&__ImageBase);
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hModule, path, MAX_PATH);
    std::wstring module_path = path;
    m_config_path = module_path;
    if (!config_dir.empty())
    {
        size_t index = module_path.find_last_of(L"\\/");
        //模块的文件名
        std::wstring module_file_name = module_path.substr(index + 1);
        m_config_path = config_dir + module_file_name;
    }
    m_config_path += L".ini";
    //TODO: 在此添加载入配置的代码
    TCHAR buff[MAX_PATH];
    GetPrivateProfileString(_T("config"), _T("file_path"), _T(""), buff, MAX_PATH, m_config_path.c_str());
    m_setting_data.file_path = buff;
    LoadTextContents(m_setting_data.file_path.c_str());

    m_setting_data.current_position = GetPrivateProfileInt(_T("config"), _T("current_position"), 0, m_config_path.c_str());
    m_setting_data.window_width = GetPrivateProfileInt(_T("config"), _T("window_width"), 180, m_config_path.c_str());
    m_setting_data.show_in_tooltips = GetPrivateProfileInt(_T("config"), _T("show_in_tooltips"), 0, m_config_path.c_str());
    m_setting_data.enable_mulit_line = GetPrivateProfileInt(_T("config"), _T("enable_mulit_line"), 0, m_config_path.c_str());
    m_setting_data.hide_when_lose_focus = GetPrivateProfileInt(_T("config"), _T("hide_when_lose_focus"), 0, m_config_path.c_str());
    m_setting_data.auto_read = GetPrivateProfileInt(_T("config"), _T("auto_read"), 0, m_config_path.c_str());
    m_setting_data.auto_read_timer_interval = GetPrivateProfileInt(_T("config"), _T("auto_read_timer_interval"), 2000, m_config_path.c_str());
    m_setting_data.speech_speed_percent = GetPrivateProfileInt(_T("config"), _T("speech_speed_percent"), 100, m_config_path.c_str());
    m_setting_data.auto_decode_base64 = GetPrivateProfileInt(_T("config"), _T("auto_decode_base64"), 1, m_config_path.c_str());
    m_setting_data.use_word_list = GetPrivateProfileInt(_T("config"), _T("use_word_list"), 0, m_config_path.c_str());
    m_setting_data.use_own_context_menu = GetPrivateProfileInt(_T("config"), _T("use_own_context_menu"), 1, m_config_path.c_str());
    m_setting_data.restart_at_end = GetPrivateProfileInt(_T("config"), _T("restart_at_end"), 0, m_config_path.c_str());
    m_setting_data.auto_reload_when_file_changed = GetPrivateProfileInt(_T("config"), _T("auto_reload_when_file_changed"), 0, m_config_path.c_str());
    m_setting_data.mouse_wheel_read = GetPrivateProfileInt(_T("config"), _T("mouse_wheel_read"), 0, m_config_path.c_str());
    m_setting_data.mouse_wheel_charactors = GetPrivateProfileInt(_T("config"), _T("mouse_wheel_charactors"), 3, m_config_path.c_str());
    m_setting_data.mouse_wheel_read_page = GetPrivateProfileInt(_T("config"), _T("mouse_wheel_read_page"), 0, m_config_path.c_str());
    m_setting_data.speech_repeat_count = GetPrivateProfileInt(_T("config"), _T("speech_repeat_count"), 1, m_config_path.c_str());
    m_setting_data.auto_speak = GetPrivateProfileInt(_T("config"), _T("auto_speak"), 0, m_config_path.c_str());
    m_setting_data.auto_speak_repeat = GetPrivateProfileInt(_T("config"), _T("auto_speak_repeat"), 1, m_config_path.c_str());

    //载入书签
    m_bookmark_mgr.LoadFromConfig(m_config_path);

    // 词表模式下，根据保存的 current_position 恢复词索引
    if (IsWordList())
    {
        if (m_setting_data.current_position >= 0 && m_setting_data.current_position < static_cast<int>(m_word_list.size()))
            m_word_index = m_setting_data.current_position;
        else
            m_word_index = 0;
    }
}

void CDataManager::SaveConfig() const
{
    if (!m_config_path.empty())
    {
        //TODO: 在此添加保存配置的代码
        WritePrivateProfileString(_T("config"), _T("file_path"), m_setting_data.file_path.c_str(), m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("current_position"), m_setting_data.current_position, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("window_width"), m_setting_data.window_width, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("show_in_tooltips"), m_setting_data.show_in_tooltips, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("enable_mulit_line"), m_setting_data.enable_mulit_line, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("hide_when_lose_focus"), m_setting_data.hide_when_lose_focus, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("auto_read"), m_setting_data.auto_read, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("auto_read_timer_interval"), m_setting_data.auto_read_timer_interval, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("speech_speed_percent"), m_setting_data.speech_speed_percent, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("auto_decode_base64"), m_setting_data.auto_decode_base64, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("use_word_list"), m_setting_data.use_word_list, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("use_own_context_menu"), m_setting_data.use_own_context_menu, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("restart_at_end"), m_setting_data.restart_at_end, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("auto_reload_when_file_changed"), m_setting_data.auto_reload_when_file_changed, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("mouse_wheel_read"), m_setting_data.mouse_wheel_read, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("mouse_wheel_charactors"), m_setting_data.mouse_wheel_charactors, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("mouse_wheel_read_page"), m_setting_data.mouse_wheel_read_page, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("speech_repeat_count"), m_setting_data.speech_repeat_count, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("auto_speak"), m_setting_data.auto_speak, m_config_path.c_str());
        WritePrivateProfileInt(_T("config"), _T("auto_speak_repeat"), m_setting_data.auto_speak_repeat, m_config_path.c_str());

        //保存书签
        m_bookmark_mgr.SaveToConfig(m_config_path);
    }
}

const CString& CDataManager::StringRes(UINT id)
{
    auto iter = m_string_table.find(id);
    if (iter != m_string_table.end())
    {
        return iter->second;
    }
    else
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_string_table[id].LoadString(id);
        return m_string_table[id];
    }
}

void CDataManager::DPIFromWindow(CWnd* pWnd)
{
    CWindowDC dc(pWnd);
    HDC hDC = dc.GetSafeHdc();
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
}

int CDataManager::DPI(int pixel)
{
    return m_dpi * pixel / 96;
}

float CDataManager::DPIF(float pixel)
{
    return m_dpi * pixel / 96;
}

int CDataManager::RDPI(int pixel)
{
    return pixel * 96 / m_dpi;
}

HICON CDataManager::GetIcon(UINT id)
{
    auto iter = m_icons.find(id);
    if (iter != m_icons.end())
    {
        return iter->second;
    }
    else
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(id), IMAGE_ICON, DPI(16), DPI(16), 0);
        m_icons[id] = hIcon;
        return hIcon;
    }
}


bool CDataManager::LoadTextContents(LPCTSTR file_path)
{
    std::ifstream file{ file_path, std::ios::binary };
    if (file.fail())
    {
        return false;
    }

    //保存文件的上次修改时间
    CCommon::GetFileLastModified(file_path, m_file_last_modified);

    //获取文件长度
    file.seekg(0, file.end);
    size_t length = file.tellg();
    file.seekg(0, file.beg);
    if (length > MAX_FILE_SIZE)	//限制文件大小不超过MAX_FILE_SIZE
    {
        length = MAX_FILE_SIZE;
    }
    if (length <= 0)
        return false;

    //读取数据
    char* buff = new char[length + 1];
    file.read(buff, length);
    file.close();
    buff[length] = '\0';
    std::string str_contents(buff, length);
    delete[] buff;

    //判断是否是base64编码
    const int BASE64_MAX_LENGTH = 1048576;
    if (g_data.m_setting_data.auto_decode_base64 && utilities::IsBase64Code(str_contents, BASE64_MAX_LENGTH))
    {
        str_contents = utilities::Base64Decode(str_contents);
    }

    bool is_utf8 = CCommon::IsUTF8Bytes(str_contents.c_str());                              //判断编码类型
    m_text_contents = CCommon::StrToUnicode(str_contents.c_str(), is_utf8);	                //转换成Unicode

    //解析章节
    m_chapter_parser.Parse();

    // 尝试将文本解析为单词表：支持两种格式
    // 1) 单行含分隔符：<原文>[  |\t| ]<翻译>
    // 2) 两行一组：第一行为原文，第二行以“〔”等标注词性/翻译（如示例）
    m_word_list.clear();
    std::wstring content = m_text_contents;
    auto trim = [](const std::wstring &s)->std::wstring {
        size_t b = 0;
        while (b < s.size() && (s[b] == L' ' || s[b] == L'\t' || s[b] == L'\r' || s[b] == L'\n')) b++;
        size_t e = s.size();
        while (e > b && (s[e-1] == L' ' || s[e-1] == L'\t' || s[e-1] == L'\r' || s[e-1] == L'\n')) e--;
        return s.substr(b, e - b);
    };

    // 先将所有非空行收集到向量中
    std::vector<std::pair<std::wstring,int>> lines; // pair of line text and start offset
    size_t pos = 0;
    while (pos < content.size()) {
        size_t nl = content.find(L'\n', pos);
        std::wstring line;
        size_t line_start = pos;
        if (nl == std::wstring::npos) {
            line = content.substr(pos);
            pos = content.size();
        } else {
            line = content.substr(pos, nl - pos);
            pos = nl + 1;
        }
        line = trim(line);
        if (line.empty()) continue;
        if (line.size() >= 2 && (line[0] == L'-' && line[1] == L'-')) continue;
        // 保留章节标题以供章节解析，但这里如果是第N课样式也跳过加入词表
        if (line.size() >= 1 && (line[0] == L'第')) continue; // 跳过章节标题
        lines.emplace_back(line, static_cast<int>(line_start));
    }

    // 清空记录位置
    m_word_positions.clear();

    for (size_t i = 0; i < lines.size(); ++i) {
        const std::wstring &line = lines[i].first;
        int line_start = lines[i].second;
        // 查找分隔符：优先两个空格 / 制表符 / 第一个空格
        size_t sep = line.find(L"  ");
        if (sep == std::wstring::npos) sep = line.find(L'\t');
        if (sep == std::wstring::npos) sep = line.find(L' ');
        if (sep != std::wstring::npos) {
            std::wstring left = trim(line.substr(0, sep));
            std::wstring right = trim(line.substr(sep + 1));
            m_word_list.emplace_back(left, right);
            m_word_positions.push_back(line_start);
            continue;
        }

        // 如果没有分隔符，尝试将当前行与下一行配对（下一行以“〔”开头通常为词性/翻译）
        if (i + 1 < lines.size()) {
            const std::wstring &next = lines[i + 1].first;
            if (!next.empty() && (next[0] == L'〔' || next[0] == L'【' || next[0] == L'[' || next[0] == L'（')) {
                m_word_list.emplace_back(line, next);
                m_word_positions.push_back(line_start);
                ++i; // 跳过下一行
                continue;
            }
        }

        // 默认将整行作为原文，尝试识别内联的翻译/词性标记并拆分
        // 例如："ちゅうごくじん（中国人） 〔名〕 中国人" -> split at '〔' 或 '［' 或 '【'
        size_t marker_pos = std::wstring::npos;
        const std::wstring markers = L"〔［【[";
        for (wchar_t mk : markers)
        {
            size_t p = line.find(mk);
            if (p != std::wstring::npos)
            {
                if (marker_pos == std::wstring::npos || p < marker_pos)
                    marker_pos = p;
            }
        }
        if (marker_pos != std::wstring::npos)
        {
            std::wstring left = trim(line.substr(0, marker_pos));
            std::wstring right = trim(line.substr(marker_pos));
            if (!left.empty())
                m_word_list.emplace_back(left, right);
            else
                m_word_list.emplace_back(line, std::wstring());
            m_word_positions.push_back(line_start);
        }
        else
        {
            m_word_list.emplace_back(line, std::wstring());
            m_word_positions.push_back(line_start);
        }
    }

    // 恢复词索引（如果 current_position 在有效范围内）
    if (!m_word_list.empty() && m_setting_data.current_position >= 0
        && m_setting_data.current_position < static_cast<int>(m_word_list.size()))
        m_word_index = m_setting_data.current_position;
    else
        m_word_index = 0;

    return true;
}


const std::wstring& CDataManager::GetTextContexts() const
{
    return m_text_contents;
}

int CDataManager::GetPageStep(CDC* dc)
{
    int step = 1;
    if (dc == nullptr)
        return 1;
    while (true)
    {
        if (m_setting_data.current_position + step >= m_text_contents.size())
            break;
        int text_width = dc->GetTextExtent(m_text_contents.c_str() + m_setting_data.current_position, step).cx;
        if (text_width > m_draw_width)
        {
            return (step - 1);
        }
        step++;
    }
    return step;
}

std::pair<std::wstring, std::wstring> CDataManager::GetCurrentWord() const
{
    if (m_word_list.empty()) return { L"", L"" };
    int idx = m_word_index;
    if (idx < 0) idx = 0;
    if (idx >= (int)m_word_list.size()) idx = (int)m_word_list.size() - 1;
    return m_word_list[idx];
}

void CDataManager::NextWord()
{
    if (m_word_list.empty()) return;
    m_word_index++;
    if (m_word_index >= (int)m_word_list.size())
        m_word_index = 0;
    m_setting_data.current_position = m_word_index;
    m_position_modified = true;
}

void CDataManager::PrevWord()
{
    if (m_word_list.empty()) return;
    m_word_index--;
    if (m_word_index < 0)
        m_word_index = (int)m_word_list.size() - 1;
    m_setting_data.current_position = m_word_index;
    m_position_modified = true;
}

void CDataManager::PageUp(int step)
{
    if (step <= 0)
        step = m_page_step;
    if (m_boss_key_pressed)
        return;
    if (IsWordList())
    {
        PrevWord();
        return;
    }
    if (m_setting_data.current_position > 0)
        m_setting_data.current_position -= step;
    if (m_setting_data.current_position < 0)
        m_setting_data.current_position = 0;
    m_position_modified = true;
}

void CDataManager::PageDown(int step)
{
    if (step <= 0)
        step = m_page_step;
    if (m_boss_key_pressed)
        return;
    // 如果处于单词表模式，滚动/自动翻页应当切换到下一词
    if (IsWordList())
    {
        NextWord();
        m_position_modified = true;
        return;
    }
    const int MAX_POS = static_cast<int>(GetTextContexts().size() - 2);
    if (m_setting_data.current_position < MAX_POS || m_setting_data.restart_at_end)
        m_setting_data.current_position += step;
    if (m_setting_data.current_position > MAX_POS)
    {
        if (m_setting_data.restart_at_end)
            m_setting_data.current_position = 0;
        else
            m_setting_data.current_position = MAX_POS;
    }
    if (m_setting_data.current_position < 0)
        m_setting_data.current_position = 0;
    m_position_modified = true;
}

int CDataManager::FindWordIndexByTextPos(int text_pos) const
{
    if (m_word_positions.empty()) return -1;
    // find first index whose position >= text_pos
    for (size_t i = 0; i < m_word_positions.size(); ++i)
    {
        if (m_word_positions[i] >= text_pos)
            return static_cast<int>(i);
    }
    return static_cast<int>(m_word_positions.size() - 1);
}

void CDataManager::SetWordIndexByTextPos(int text_pos)
{
    int idx = FindWordIndexByTextPos(text_pos);
    if (idx >= 0)
        m_word_index = idx;
}

void CDataManager::AddBookmark()
{
    int pos = m_setting_data.current_position;
    if (IsWordList())
    {
        if (m_word_index >= 0 && m_word_index < static_cast<int>(m_word_positions.size()))
            pos = m_word_positions[m_word_index];
    }
    m_bookmark_mgr.AddBookmark(m_setting_data.file_path, pos);
    SaveConfig();
}

bool CDataManager::IsMultiLine() const
{
    return m_multi_line && m_setting_data.enable_mulit_line;
}

bool CDataManager::HasFocus() const
{
    return GetForegroundWindow() == m_wnd;
}

CChapterParser & CDataManager::GetChapter()
{
    return m_chapter_parser;
}

const std::set<int>& CDataManager::GetBookmark()
{
    return m_bookmark_mgr.GetBookmark(m_setting_data.file_path);
}

void CDataManager::RemoveBookmark(int pos)
{
    m_bookmark_mgr.RemoveBookmark(m_setting_data.file_path, pos);
    SaveConfig();
}

int CDataManager::GetCurrentTextPosForBookmark() const
{
    if (IsWordList())
    {
        if (m_word_index >= 0 && m_word_index < static_cast<int>(m_word_positions.size()))
            return m_word_positions[m_word_index];
        if (!m_word_positions.empty())
            return m_word_positions[0];
        return m_setting_data.current_position;
    }
    return m_setting_data.current_position;
}

void CDataManager::SaveReadPosition()
{
    if (m_position_modified)
    {
        SaveConfig();
        m_position_modified = false;
    }
}

void CDataManager::CheckFileChange()
{
    if (m_setting_data.auto_reload_when_file_changed && !m_setting_data.file_path.empty())
    {
        //检查文件的最后修改时间
        unsigned __int64 last_modified{};
        if (CCommon::GetFileLastModified(m_setting_data.file_path, last_modified))
        {
            //如果文件的最后修改时间比打开的时候新，则重新打开文件
            if (last_modified > m_file_last_modified)
            {
                m_file_last_modified = last_modified;
                LoadTextContents(m_setting_data.file_path.c_str());
            }
        }
    }
}
