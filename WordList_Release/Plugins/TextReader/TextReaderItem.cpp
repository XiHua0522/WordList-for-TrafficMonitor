#include "pch.h"
#include "TextReaderItem.h"
#include "DataManager.h"
#include "Common.h"
#include "TextReader.h"

const wchar_t* CTextReaderItem::GetItemName() const
{
    return g_data.StringRes(IDS_PLUGIN_ITEM_NAME);
}

const wchar_t* CTextReaderItem::GetItemId() const
{
    //TODO: 在此返回插件的唯一ID，建议只包含字母和数字
    return L"W5XuBvH0";
}

const wchar_t* CTextReaderItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CTextReaderItem::GetItemValueText() const
{
    return L"";
}

const wchar_t* CTextReaderItem::GetItemValueSampleText() const
{
    return L"Text Reader Plugin";
}

bool CTextReaderItem::IsCustomDraw() const
{
    //TODO: 根据是否由插件自绘返回对应的值
    return true;
}


int CTextReaderItem::GetItemWidth() const
{
    // 如果处于单词表模式，尝试根据当前词测量宽度以实现自适应窗口
    if (g_data.IsWordList())
    {
        auto p = g_data.GetCurrentWord();
        std::wstring text;
        if (!p.second.empty())
            text = p.first + L"\n" + p.second;
        else
            text = p.first;

        if (text.empty())
            return g_data.m_setting_data.window_width;

        // 使用屏幕 DC 测量文本宽度（保守估算，不考虑复杂排版）
        CWindowDC dc(NULL);
        CFont* oldFont = dc.SelectObject(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
        CSize sz = dc.GetTextExtent(text.c_str());
        dc.SelectObject(oldFont);
        int measured = sz.cx + g_data.DPI(8); // 加一点内边距
        return max(measured, g_data.m_setting_data.window_width);
    }
    return g_data.m_setting_data.window_width;
}

void CTextReaderItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    g_data.m_draw_width = w;
    static std::wstring text;
    bool hide{ g_data.m_setting_data.hide_when_lose_focus && !g_data.HasFocus() };   //窗口没有没有焦点时，显示文本消失
    if (g_data.m_boss_key_pressed || hide)                              //按下老板键，显示文本消失
    {
        text.clear();
    }
    else if (g_data.IsWordList())
    {
        auto p = g_data.GetCurrentWord();
        if (!p.second.empty())
            text = p.first + L"\n" + p.second;
        else
            text = p.first;
    }
    else if (g_data.m_setting_data.current_position < g_data.GetTextContexts().size())
    {
        text = g_data.GetTextContexts().substr(g_data.m_setting_data.current_position, 100);
    }

    //绘图句柄
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    g_data.m_multi_line = (h > g_data.DPI(30));
    g_data.m_page_step = g_data.GetPageStep(pDC);

    //矩形区域
    CRect rect(CPoint(x, y), CSize(w, h));
    UINT flag{};
        if (g_data.IsWordList())
        {
            // 在单词表模式下始终两行显示并居中换行（原文+翻译）
            flag = DT_CENTER | DT_NOPREFIX | DT_WORDBREAK;
        }
    else if (g_data.IsMultiLine())
    {
        flag = DT_NOPREFIX | DT_WORDBREAK;
    }
    else
    {
        flag = DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE;
    }
    // 如果是单词表且多行，需要垂直居中：先计算文本所需矩形，再居中绘制
    if (g_data.IsWordList() && g_data.IsMultiLine())
    {
        CRect calcRect = rect;
        pDC->DrawText(text.c_str(), &calcRect, flag | DT_CALCRECT);
        int textH = calcRect.Height();
        int top = rect.top + (rect.Height() - textH) / 2;
        CRect drawRect(rect.left, top, rect.right, top + textH);
        pDC->DrawText(text.c_str(), drawRect, flag);
    }
    else
    {
        pDC->DrawText(text.c_str(), rect, flag);
    }

}

int CTextReaderItem::OnMouseEvent(MouseEventType type, int x, int y, void * hWnd, int flag)
{
    g_data.m_wnd = (HWND)hWnd;
    CWnd* pWnd = CWnd::FromHandle((HWND)hWnd);
    if (type == IPluginItem::MT_LCLICKED)
    {
        // 取消单击切换单词：当处于单词表模式时，不响应左键单击
        if (!g_data.IsWordList())
        {
            g_data.PageDown(g_data.m_page_step);
            return 1;
        }
        return 0;
    }
        else if (type == IPluginItem::MT_DBCLICKED)
        {
            if (g_data.IsWordList())
            {
                auto p = g_data.GetCurrentWord();
                if (!p.first.empty())
                {
                    double speed = static_cast<double>(g_data.m_setting_data.speech_speed_percent) / 100.0;
                    int repeat = g_data.m_setting_data.speech_repeat_count;
                    if (repeat < 1) repeat = 1;
                    if (repeat > 10) repeat = 10;

                    // 清洗原文：去掉括号及其内部内容
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

                    // 清洗翻译：去掉开头的词性标记
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
                        double word_speed = speed * 0.85;
                        if (word_speed < 0.1) word_speed = 0.1;
                        CCommon::SpeakPair(word, 0x0411, meaning, 0x0804, word_speed, speed, repeat, 1, 1);
                    }
                    else
                        CCommon::SpeakText(word, 0x0411, speed, repeat);
                }
                return 1;
            }
            return 1;
        }
    else if (type == IPluginItem::MT_RCLICKED)
    {
        CTextReader::Instance().ShowContextMenu(pWnd);
        return 1;
    }
    // 鼠标滚轮：当为单词表模式时，用滚轮切词；否则保留原有滚动阅读行为（若开启）
    if (g_data.IsWordList())
    {
        if (type == IPluginItem::MT_WHEEL_UP)
        {
            g_data.PrevWord();
            return 1;
        }
        else if (type == IPluginItem::MT_WHEEL_DOWN)
        {
            g_data.NextWord();
            return 1;
        }
    }
    else if (g_data.m_setting_data.mouse_wheel_read)
    {
        int mouse_wheel_step;
        if (g_data.m_setting_data.mouse_wheel_read_page)
            mouse_wheel_step = g_data.m_page_step;
        else
            mouse_wheel_step = g_data.m_setting_data.mouse_wheel_charactors;

        if (type == IPluginItem::MT_WHEEL_UP)
        {
            g_data.PageUp(mouse_wheel_step);
            return 1;
        }
        else if (type == IPluginItem::MT_WHEEL_DOWN)
        {
            g_data.PageDown(mouse_wheel_step);
            return 1;
        }
    }

    return 0;
}

int CTextReaderItem::OnKeboardEvent(int key, bool ctrl, bool shift, bool alt, void* hWnd, int flag)
{
    g_data.m_wnd = (HWND)hWnd;
    if (key == VK_LEFT)
    {
        g_data.PageUp(1);
        return 1;
    }
    else if (key == VK_RIGHT)
    {
        g_data.PageDown(1);
        return 1;
    }
    else if (key == VK_UP)
    {
        g_data.PageUp();
        return 1;
    }
    else if (key == VK_DOWN)
    {
        g_data.PageDown();
    }
    else if (key == VK_SPACE)
    {
        g_data.m_boss_key_pressed = !g_data.m_boss_key_pressed;
    }
    return 0;
}
