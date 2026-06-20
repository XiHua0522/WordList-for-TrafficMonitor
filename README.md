# WordList for TrafficMonitor 任务栏单词

一个基于 [TextReader](https://github.com/zhongyang219/TrafficMonitorPlugins/releases/tag/TextReader_V1.02) 修改而来的 TrafficMonitor 插件，增加**查看单词/发音**功能。

> **⚠️ 声明**： <br>
> 本人没有任何编程基础，本项目代码均由 AI 修改完成。~~后续大概率不会有任何更新或 Bug 修复，请见谅。如果发现问题，建议自行 fork 后修改，或寻找其他替代方案。~~
我是🐷B,不会用git，结果把代码全部整没了 <br>
> 仅测试过日语词汇，其他语言未测试



---

## 添加功能

| 功能 | 说明 |
|------|------|
| **词表模式** | 在设置中开启后，会自动识别 `单词&翻译` 格式的词表，并拆分为原文/翻译两部分 |
| **双击发音** | 双击插件区域即可朗读当前单词，先用原文语言朗读，再用翻译语言朗读（发音会跳过括号内的原文/词性） |
| **自动朗读** | 开启自动翻页时同步朗读每个单词，无需手动双击 |
| **阅读记忆** | 词表模式下会记录当前阅读位置，关闭插件后再次打开会从上次位置继续 |
| **语速调节** | 支持调节朗读语速；单词朗读时会自动稍慢一点（1~3倍速），方便听清 |

---

## 安装使用

1. **先下载解压解压 [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor)**
2. 下载插件 `WordList_dll.zip`并解压
3. 将 `WordList.dll` 放入 TrafficMonitor 安装目录下的 `plugins` 文件夹中（没有的话自己新建）
4. 重启 TrafficMonitor，在"选项 → 常规设置"下滑至最后点击`插件管理`查看插件是否加载成功
5.（可选）. **在 Windows 系统设置中添加对应的语音语言包**（例如要朗读日语/中文，需在"设置 → 时间和语言 → 语言和区域"中安装对应的语音包）
    
    觉得下载的语言包发音太机械的话可以试试NaturalVoice添加在线语音，参考这两篇文章：

   * [win10添加微软离线自然语音合成](https://blog.csdn.net/FL1623863129/article/details/149852228)（只用看到NaturalVoice设置完成后就行）
   * [更换微软TTS语音引擎切换](https://blog.csdn.net/FL1623863129/article/details/149852228?fromshare=blogdetail&sharetype=blogdetail&sharerId=149852228&sharerefer=PC&sharesource=XiHua_0522&sharefrom=from_link)

---

## 词表文件格式

词表文件支持如下格式，每行一个单词：

```text
单词〔词性〕（翻译）
单词〔词性〕（翻译）
```

示例：

```text
わたし（我）
あなた（你）
ちゅうごくじん（中国人）
たんじょうび（誕生日） 〔名〕 生日
```

在插件设置中勾选**"使用词表模式"**即可自动识别该格式。
词表模式下窗口宽度会变为自适应宽度，以方便展示过长单词
---

实际效果： <br>
![](https://img.alicdn.com/imgextra/i2/2220538134662/O1CN01Tz9wuD1kJFbg5JP7f_!!2220538134662.gif)

## 自动朗读

1. 设置中勾选 "词表模式"，设置下单词朗读的重复次数
2. 右键菜单将 "自动翻页" 和 "自动朗读" 都打开
3. 在设置中调节 "自动翻页时间间隔"，建议设置得长一些（例如 5~10 秒以上），让每个单词有足够时间发音

> **注意**：自动朗读必须配合自动翻页同时开启才会生效。

---

~~## 编译环境~~

~~- Visual Studio 2022~~
~~- MFC / C++~~
~~- Windows SDK 10.0~~

~~用 VS2022 打开 `TrafficMonitorPlugins.sln`，编译 `TextReader`（输出文件名为 `WordList.dll`）即可。~~

---

## 鸣谢

- [zhongyang219](https://github.com/zhongyang219) / [TrafficMonitorPlugins](https://github.com/zhongyang219/TrafficMonitorPlugins)
