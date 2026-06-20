#include "pch.h"
#include "Common.h"
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

std::wstring CCommon::StrToUnicode(const char* str, bool utf8)
{
    if (str == nullptr)
        return std::wstring();
    std::wstring result;
    int size;
    size = MultiByteToWideChar((utf8 ? CP_UTF8 : CP_ACP), 0, str, -1, NULL, 0);
    if (size <= 0) return std::wstring();
    wchar_t* str_unicode = new wchar_t[size + 1];
    MultiByteToWideChar((utf8 ? CP_UTF8 : CP_ACP), 0, str, -1, str_unicode, size);
    result.assign(str_unicode);
    delete[] str_unicode;
    return result;
}

std::string CCommon::UnicodeToStr(const wchar_t* wstr, bool utf8)
{
    if (wstr == nullptr)
        return std::string();
    std::string result;
    int size{ 0 };
    size = WideCharToMultiByte((utf8 ? CP_UTF8 : CP_ACP), 0, wstr, -1, NULL, 0, NULL, NULL);
    if (size <= 0) return std::string();
    char* str = new char[size + 1];
    WideCharToMultiByte((utf8 ? CP_UTF8 : CP_ACP), 0, wstr, -1, str, size, NULL, NULL);
    result.assign(str);
    delete[] str;
    return result;
}

bool CCommon::IsUTF8Bytes(const char* data)
{
    int charByteCounter = 1;  //计算当前正分析的字符应还有的字节数
    unsigned char curByte; //当前分析的字节.
    bool ascii = true;
    int length = static_cast<int>(strlen(data));
    for (int i = 0; i < length; i++)
    {
        curByte = static_cast<unsigned char>(data[i]);
        if (charByteCounter == 1)
        {
            if (curByte >= 0x80)
            {
                ascii = false;
                //判断当前
                while (((curByte <<= 1) & 0x80) != 0)
                {
                    charByteCounter++;
                }
                //标记位首位若为非0 则至少以2个1开始 如:110XXXXX...........1111110X
                if (charByteCounter == 1 || charByteCounter > 6)
                {
                    return false;
                }
            }
        }
        else
        {
            //若是UTF-8 此时第一位必须为1
            if ((curByte & 0xC0) != 0x80)
            {
                return false;
            }
            charByteCounter--;
        }
    }
    if (ascii) return false;        //如果全是ASCII字符，返回false
    else return true;
}

void CCommon::StringSplit(const std::wstring& str, const std::wstring& div_str, std::vector<std::wstring>& results, bool skip_empty)
{
    results.clear();
    size_t split_index = 0 - div_str.size();
    size_t last_split_index = 0 - div_str.size();
    while (true)
    {
        split_index = str.find(div_str, split_index + div_str.size());
        std::wstring split_str = str.substr(last_split_index + div_str.size(), split_index - last_split_index - div_str.size());
        if (!split_str.empty() || !skip_empty)
            results.push_back(split_str);
        if (split_index == std::wstring::npos)
            break;
        last_split_index = split_index;
    }
}

bool CCommon::GetFileLastModified(const std::wstring& file_path, unsigned __int64& modified_time)
{
    WIN32_FILE_ATTRIBUTE_DATA file_attributes{};
    if (GetFileAttributesEx(file_path.c_str(), GetFileExInfoStandard, &file_attributes))
    {
        ULARGE_INTEGER last_modified_time{};
        last_modified_time.HighPart = file_attributes.ftLastWriteTime.dwHighDateTime;
        last_modified_time.LowPart = file_attributes.ftLastWriteTime.dwLowDateTime;
        modified_time = last_modified_time.QuadPart;
        return true;
    }
    return false;
}

static std::string Base64Encode(const unsigned char* data, size_t len)
{
    static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    size_t i;
    unsigned int val;
    for (i = 0; i + 2 < len; i += 3)
    {
        val = (data[i] << 16) | (data[i + 1] << 8) | (data[i + 2]);
        out.push_back(table[(val >> 18) & 0x3F]);
        out.push_back(table[(val >> 12) & 0x3F]);
        out.push_back(table[(val >> 6) & 0x3F]);
        out.push_back(table[val & 0x3F]);
    }
    if (i < len)
    {
        int left = len - i;
        val = (data[i] << 16) | (left == 2 ? (data[i + 1] << 8) : 0);
        out.push_back(table[(val >> 18) & 0x3F]);
        out.push_back(table[(val >> 12) & 0x3F]);
        if (left == 2)
            out.push_back(table[(val >> 6) & 0x3F]);
        else
            out.push_back('=');
        out.push_back('=');
    }
    return out;
}

unsigned int CCommon::DetectLanguageFromText(const std::wstring& text)
{
    for (wchar_t ch : text)
    {
        if (ch >= 0x3040 && ch <= 0x30FF) // Hiragana or Katakana
            return 0x0411; // ja-JP
        if (ch >= 0x4E00 && ch <= 0x9FFF) // CJK Unified Ideographs
            return 0x0804; // zh-CN (fallback to Chinese)
    }
    return 0x0409; // en-US
}

static int SpeedToRate(double speed)
{
    if (speed < 0.1) speed = 0.1;
    if (speed > 3.0) speed = 3.0;
    double v = (speed - 0.1) / (3.0 - 0.1) * 20.0 - 10.0; // map 0.1->-10, 3.0->10
    int rate = static_cast<int>(std::round(v));
    if (rate < -10) rate = -10;
    if (rate > 10) rate = 10;
    return rate;
}

void CCommon::SpeakText(const std::wstring& text, unsigned int lcid, double speed, int repeat)
{
    if (text.empty())
        return;
    if (lcid == 0)
        lcid = DetectLanguageFromText(text);

    // Prepare PowerShell script. Use single-quoted string and escape single quotes by doubling them.
    std::wstring t = text;
    size_t pos = 0;
    while ((pos = t.find(L'\'', pos)) != std::wstring::npos)
    {
        t.insert(pos, 1, L'\'');
        pos += 2;
    }

    wchar_t hexbuf[16];
    swprintf_s(hexbuf, L"%04X", lcid & 0xFFFF);
    // decimal form
    int dec = lcid & 0xFFFF;

    int rate = SpeedToRate(speed);
    wchar_t ratebuf[8]; swprintf_s(ratebuf, L"%d", rate);

    std::wstring script = L"$voice=New-Object -ComObject SAPI.SpVoice; $tokens=$voice.GetVoices(); $targetHex='";
    script += hexbuf;
    script += L"'; $targetShort = $targetHex.TrimStart('0'); $targetDec = '";
    {
        wchar_t decbuf[16];
        swprintf_s(decbuf, L"%d", dec);
        script += decbuf;
    }
    script += L"'; foreach($t in $tokens){ $lang=$t.GetAttribute('Language'); $name=$t.GetAttribute('Name'); if($lang){ if($lang -match $targetHex -or $lang -match $targetShort -or $lang -match $targetDec){ $voice.Voice=$t; break } } if($name){ if($name -match 'Haruka' -or $name -match 'haruka' -or $name -match 'ja-' -or $name -match 'Japanese' -or $name -match '日本語'){ $voice.Voice=$t; break } } }; $voice.Rate = ";
    script += ratebuf;

    if (repeat <= 1)
    {
        script += L"; $voice.Speak('";
        script += t;
        script += L"')";
    }
    else
    {
        if (repeat > 50) repeat = 50;
        wchar_t rb[16]; swprintf_s(rb, L"%d", repeat);
        script += L"; for($i=0;$i -lt ";
        script += rb;
        script += L";$i++){$voice.Speak('";
        script += t;
        script += L"')}";
    }

    // Encode script as UTF-16LE bytes and base64 encode
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(script.c_str());
    size_t byte_len = (script.size()) * sizeof(wchar_t);
    std::string b64 = Base64Encode(bytes, byte_len);

    // Build command: powershell -NoProfile -NonInteractive -EncodedCommand <b64>
    std::wstring cmd = L"powershell -NoProfile -NonInteractive -EncodedCommand ";
    // Convert base64 to wide
    std::wstring wb64(b64.begin(), b64.end());
    cmd += wb64;

    // Start the process without waiting
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    // Use CreateProcessW
    if (!CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        // fallback: ShellExecute
        ShellExecuteW(nullptr, L"open", L"powershell", wb64.c_str(), nullptr, SW_HIDE);
    }
    else
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

void CCommon::SpeakPair(const std::wstring& text1, unsigned int lcid1, const std::wstring& text2, unsigned int lcid2, double speed1, double speed2, int repeat, int repeat1, int repeat2)
{
    if (text1.empty() && text2.empty()) return;

    if (repeat1 < 0) repeat1 = 1;
    if (repeat2 < 0) repeat2 = 1;
    if (repeat <= 1) repeat = 1;
    if (repeat > 50) repeat = 50;
    if (repeat1 > 50) repeat1 = 50;
    if (repeat2 > 50) repeat2 = 50;

    // prepare texts and escape single quotes
    auto escape = [](std::wstring s){ size_t p = 0; while ((p = s.find(L'\'', p)) != std::wstring::npos) { s.insert(p, 1, L'\''); p += 2; } return s; };
    std::wstring t1 = escape(text1);
    std::wstring t2 = escape(text2);

    // Build PowerShell script: select voice and speak sequence. Repeat if needed.
    std::wstring script = L"$voice=New-Object -ComObject SAPI.SpVoice; $tokens=$voice.GetVoices();";

    // outer repeat loop
    script += L" for($i=0;$i -lt ";
    {
        wchar_t rb[16]; swprintf_s(rb, L"%d", repeat);
        script += rb;
    }
    script += L";$i++){";

    // text1 inner loop
    if (!t1.empty() && repeat1 > 0) {
        script += L" for($j=0;$j -lt ";
        {
            wchar_t r1[16]; swprintf_s(r1, L"%d", repeat1);
            script += r1;
        }
        script += L";$j++){";

        unsigned int use1 = lcid1 == 0 ? DetectLanguageFromText(text1) : lcid1;
        wchar_t hex1[16]; swprintf_s(hex1, L"%04X", use1 & 0xFFFF);
        int dec1 = use1 & 0xFFFF;
        script += L" foreach($v in $tokens){ $lang=$v.GetAttribute('Language'); $name=$v.GetAttribute('Name'); if($lang){ if($lang -match '";
        script += hex1;
        script += L"' -or $lang -match '";
        std::wstring short1 = std::wstring(hex1).substr( (std::wstring(hex1).find_first_not_of(L'0')==std::wstring::npos)? std::wstring::npos : std::wstring(hex1).find_first_not_of(L'0'));
        if (!short1.empty()) { script += short1; script += L"' -or $lang -match '"; }
        {
            wchar_t decbuf1[16]; swprintf_s(decbuf1, L"%d", dec1);
            script += decbuf1;
        }
        script += L"'){ $voice.Voice=$v; break } } if($name){ if($name -match 'Haruka' -or $name -match 'haruka' -or $name -match 'ja-' -or $name -match 'Japanese' -or $name -match '日本語'){ $voice.Voice=$v; break } } }; $voice.Rate = ";
        {
            wchar_t rb1[8]; swprintf_s(rb1, L"%d", SpeedToRate(speed1)); script += rb1;
        }
        script += L"; $voice.Speak('";
        script += t1;
        script += L"'); }";
    }

    // text2 inner loop
    if (!t2.empty() && repeat2 > 0) {
        script += L" for($k=0;$k -lt ";
        {
            wchar_t r2[16]; swprintf_s(r2, L"%d", repeat2);
            script += r2;
        }
        script += L";$k++){";

        unsigned int use2 = lcid2 == 0 ? DetectLanguageFromText(text2) : lcid2;
        wchar_t hex2[16]; swprintf_s(hex2, L"%04X", use2 & 0xFFFF);
        int dec2 = use2 & 0xFFFF;
        script += L" foreach($v in $tokens){ $lang=$v.GetAttribute('Language'); $name=$v.GetAttribute('Name'); if($lang){ if($lang -match '";
        script += hex2;
        script += L"' -or $lang -match '";
        std::wstring short2 = std::wstring(hex2).substr( (std::wstring(hex2).find_first_not_of(L'0')==std::wstring::npos)? std::wstring::npos : std::wstring(hex2).find_first_not_of(L'0'));
        if (!short2.empty()) { script += short2; script += L"' -or $lang -match '"; }
        {
            wchar_t decbuf2[16]; swprintf_s(decbuf2, L"%d", dec2);
            script += decbuf2;
        }
        script += L"'){ $voice.Voice=$v; break } } if($name){ if($name -match 'Huihui' -or $name -match 'huihui' -or $name -match 'zh-' -or $name -match 'Chinese' -or $name -match '中文'){ $voice.Voice=$v; break } } }; $voice.Rate = ";
        {
            wchar_t rb2[8]; swprintf_s(rb2, L"%d", SpeedToRate(speed2)); script += rb2;
        }
        script += L"; $voice.Speak('";
        script += t2;
        script += L"'); }";
    }

    // close outer for-loop
    script += L" }";

    // Encode and launch as before
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(script.c_str());
    size_t byte_len = (script.size()) * sizeof(wchar_t);
    std::string b64 = Base64Encode(bytes, byte_len);
    std::wstring cmd = L"powershell -NoProfile -NonInteractive -EncodedCommand ";
    std::wstring wb64(b64.begin(), b64.end());
    cmd += wb64;

    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    if (!CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        ShellExecuteW(nullptr, L"open", L"powershell", wb64.c_str(), nullptr, SW_HIDE);
    }
    else
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}
