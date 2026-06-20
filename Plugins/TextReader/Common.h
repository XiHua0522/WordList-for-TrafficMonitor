#pragma once
#include <string>
#include <vector>
class CCommon
{
public:
    //将const char*字符串转换成宽字符字符串
    static std::wstring StrToUnicode(const char* str, bool utf8 = false);

    static std::string UnicodeToStr(const wchar_t* wstr, bool utf8 = false);

    //判断一个字符串是否UTF8编码
    static bool IsUTF8Bytes(const char* data);

    static void StringSplit(const std::wstring& str, const std::wstring& div_str, std::vector<std::wstring>& results, bool skip_empty = true);

    //获取一个文件的最后修改时间
    static bool GetFileLastModified(const std::wstring& file_path, unsigned __int64& modified_time);
    // 通过 Windows TTS 发音，lcid 为 0 表示自动检测
    static void SpeakText(const std::wstring& text, unsigned int lcid = 0, double speed = 1.0, int repeat = 1);
    // 顺序朗读两段文本，分别使用指定的 lcid（0 表示自动检测），并分别指定语速
    // repeat1/repeat2 分别控制两段文本各自在每次循环中的重复次数；传入 -1 表示与 repeat 相同（向后兼容）
    static void SpeakPair(const std::wstring& text1, unsigned int lcid1, const std::wstring& text2, unsigned int lcid2, double speed1 = 1.0, double speed2 = 1.0, int repeat = 1, int repeat1 = -1, int repeat2 = -1);
    // 根据文本简单检测语言，返回 LCID（例如 0x0411 为日语，0x0804 为中文，0x0409 为英语）
    static unsigned int DetectLanguageFromText(const std::wstring& text);

};
