
#include <speechapi_cxx.h>
#include "appx.hpp"
#include "window.h"
#include "controls.h"
#include "LoopbackCapture.h"
using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;
constexpr inline const char MS_SR_KEY[] = "Key:XUw7C0rcZAIQvG837YP4F1KHz2RqYuQgtyXrcbFhsWFNGjG08HJElmPGesxNMbib0s8y39NEti3q3RwPNRbuDv75ejZbTa9yLcTAUixC";
const WCHAR syspath1[] = LR"(C:\Windows\SystemApps\MicrosoftWindows.Client.Core_cw5n1h2txyewy\LiveCaptions)";
const WCHAR syspath2[] = LR"(C:\Windows\SystemApps\MicrosoftWindows.Client.Core_cw5n1h2txyewy)";
const WCHAR packageName[] = L"MicrosoftWindows.Speech.";
static auto punctuations = std::set(
    {L'【', L'】', L'。', L'，', L'！', L'？', L'　', L'‘', L'’', L'“', L'”', L'、', L'《', L'》', L'；', L'：', L'…', L'（', L'）', L'」', L'「',
     L' ', L',', L'·', L'.', L'\'', L'\"', L'?', L'/', L';', L':', L'|', L'[', L']', L'{', L'}', L'-', L'_', L'=', L'+', L'`', L'~', L'!', L'#', L'$', L'%', L'^', L'&', L'*', L'(', L')', L'\\'});
inline std::wstring StringToWideString(const std::string &text, UINT encoding = CP_UTF8)
{
    std::vector<wchar_t> buffer(text.size() + 1);
    int length = MultiByteToWideChar(encoding, 0, text.c_str(), text.size() + 1, buffer.data(), buffer.size());
    return std::wstring(buffer.data(), length - 1);
}
inline std::string WideStringToString(const std::wstring &text, UINT cp = CP_UTF8)
{
    std::vector<char> buffer((text.size() + 1) * 4);
    WideCharToMultiByte(cp, 0, text.c_str(), -1, buffer.data(), buffer.size(), nullptr, nullptr);
    return buffer.data();
}

void initEnv()
{
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        system("chcp 65001");
    }
    SetProcessDPIAware();
    RoInitialize(RO_INIT_MULTITHREADED);

    WCHAR env[65535];
    GetEnvironmentVariableW(L"PATH", env, 65535);
    auto newenv = std::wstring(env) + L";" + syspath1 + L";" + syspath2;
    SetEnvironmentVariableW(L"PATH", newenv.c_str());
}
std::vector<std::string> all_models()
{
    std::vector<std::string> paths;
    for (auto &&[name, path] : FindPackages(packageName))
    {
        std::wcout << name << L"\t" << path << std::endl;
        paths.push_back(WideStringToString(path));
    }
    for (auto &&p : std::filesystem::directory_iterator(L"."))
    {
        if (p.is_directory() && p.path().filename().native()._Starts_with(packageName))
        {
            std::wcout << p.path().native() << std::endl;
            paths.push_back(WideStringToString(p.path().native()));
        }
    }
    return paths;
}
class MainWindow : public mainwindow
{
    gridlayout *mainlayout;
    button *button1;
    combobox *combolangs;
    std::vector<std::string> modelnames;
    std::shared_ptr<EmbeddedSpeechConfig> speechconfig;
    std::shared_ptr<SpeechRecognizer> recognizer;
    std::shared_ptr<PushAudioInputStream> pushStream;
    std::shared_ptr<AudioConfig> audioConfig;
    CComPtr<CLoopbackCapture> capture;
    void languagechanged();
    CEvent recognitionEnd{FALSE, FALSE};
    void create_recognizer();
    void Recognizing(const std::shared_ptr<SpeechRecognitionResult>);
    void Recognized(const std::shared_ptr<SpeechRecognitionResult>);
    bool isrunning = false;
    int curridx = 0;
    int curridx2 = 0;
    multilineedit *recognizing;
    multilineedit *recognized;
    checkbox *copytoclip;
    checkbox *copytoclipwhenpunch;
    std::wstring laststr;

public:
    void on_show() override;
    void on_close() override;
    MainWindow();
};
void MainWindow::on_close()
{
    if (isrunning)
    {
        capture->StopCaptureAsync();
        recognizer->StopContinuousRecognitionAsync().wait();
    }
}
void MainWindow::create_recognizer()
{
    recognizer = SpeechRecognizer::FromConfig(speechconfig, audioConfig);
    recognizer->Recognizing.Connect([&](const SpeechRecognitionEventArgs &e)
                                    { Recognizing(e.Result); });
    auto Recognized_ = [&](const SpeechRecognitionEventArgs &e)
    {
        switch (e.Result->Reason)
        {
        case ResultReason::RecognizedSpeech:
            Recognized(e.Result);
            break;
        }
    };
    recognizer->Recognized.Connect(Recognized_);
    auto Canceled = [&](const SpeechRecognitionCanceledEventArgs &e)
    {
        switch (e.Reason)
        {
        case CancellationReason::EndOfStream:
            break;

        case CancellationReason::Error:
            MessageBoxA(winId, e.ErrorDetails.c_str(), "Error", 0);
            recognitionEnd.Set();
            break;
        }
    };
    recognizer->Canceled.Connect(Canceled);

    recognizer->SessionStarted.Connect(
        [&](const SessionEventArgs &e)
        {
            button1->settext(L"暂停");
            isrunning = true;
        });

    recognizer->SessionStopped.Connect(
        [&](const SessionEventArgs &e)
        {
            recognitionEnd.Set();
            button1->settext(L"开始");
            isrunning = false;
        });
};
bool sendclipboarddata_i(const std::wstring &text, HWND hwnd)
{
    if (!OpenClipboard((HWND)hwnd))
        return false;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
    memcpy(GlobalLock(hMem), text.c_str(), (text.size() + 1) * sizeof(wchar_t));
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, hMem);
    GlobalUnlock(hMem);
    CloseClipboard();
    return true;
}
bool sendclipboarddata(const std::wstring &text, HWND hwnd)
{
    for (int loop = 0; loop < 10; loop++)
    {
        auto succ = sendclipboarddata_i(text, hwnd);
        if (succ)
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return false;
}
void MainWindow::Recognizing(const std::shared_ptr<SpeechRecognitionResult> r)
{
    auto ws = StringToWideString(r->Text);
    std::cout << r->Text << std::endl;
    SetWindowText(recognizing->winId, ws.c_str());
    if (copytoclip->ischecked())
    {
        auto increase = ws._Starts_with(laststr) ? ws.substr(laststr.size()) : L"";
        laststr = ws;
        if ((!copytoclipwhenpunch->ischecked()) || std::any_of(increase.begin(), increase.end(), [](auto &c) -> bool
                                                               { return punctuations.count(c); }))
        {
            sendclipboarddata(ws, winId);
        }
    }
}
void MainWindow::Recognized(const std::shared_ptr<SpeechRecognitionResult> r)
{
    auto ws = StringToWideString(r->Text);
    std::cout << r->Text << std::endl;
    SetWindowText(recognized->winId, ws.c_str());
    if (copytoclip->ischecked())
    {
        sendclipboarddata(ws, winId);
    }
}
void MainWindow::languagechanged()
{
    try
    {
        auto last = isrunning;
        if (last)
        {
            capture->StopCaptureAsync();
            recognizer->StopContinuousRecognitionAsync().wait();
        }
        std::cout << curridx << "\t" << modelnames[curridx] << std::endl;
        speechconfig->SetSpeechRecognitionModel(modelnames[curridx], MS_SR_KEY);

        capture->OnDataCallback = [&](std::string &&data)
        {
            pushStream->Write((uint8_t *)data.data(), data.size());
        };
        audioConfig = AudioConfig::FromStreamInput(pushStream);
        create_recognizer();
        if (last)
        {
            recognizer->StartContinuousRecognitionAsync().wait();
            capture->StartCaptureAsync(GetCurrentProcessId(), false);
        }
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}
void MainWindow::on_show()
{
    capture = new CLoopbackCapture{16000, 16, 1};
    pushStream = AudioInputStream::CreatePushStream();

    auto paths = all_models();

    speechconfig = EmbeddedSpeechConfig::FromPaths(paths);
    speechconfig->SetProfanity(ProfanityOption::Raw);
    for (auto &m : speechconfig->GetSpeechRecognitionModels())
    {
        std::cout << m->Name << "\t" << m->Locales[0] << std::endl;
        combolangs->additem(StringToWideString(m->Locales[0] + "        " + m->Path));
        modelnames.push_back(m->Name);
    }
    combolangs->setcurrent(0);
    combolangs->oncurrentchange = [&](int curr)
    {
        if (curridx == curr)
            return;
        curridx = curr;
        languagechanged();
    };
    languagechanged();
}
MainWindow::MainWindow()
{
    MyFont uifont;
    uifont.fontsize = 14;
    uifont.fontfamily = L"Aria";
    setfont(uifont);
    setcentral(1200, 800);
    mainlayout = new gridlayout;
    setlayout(mainlayout);
    settext(L"DirectLiveCaptions");
    button1 = new button{this, L"开始"};
    mainlayout->addcontrol(button1, 0, 0, 1, 2);
    combolangs = new combobox(this);
    mainlayout->addcontrol(combolangs, 1, 0, 1, 2);
    button1->onclick = [&]()
    {
        if (isrunning)
        {
            capture->StopCaptureAsync();
            recognizer->StopContinuousRecognitionAsync().wait();
        }
        else
        {
            recognizer->StartContinuousRecognitionAsync().wait();
            capture->StartCaptureAsync(GetCurrentProcessId(), false);
        }
    };

    recognizing = new multilineedit{this};
    recognizing->setreadonly(true);
    recognized = new multilineedit{this};
    recognized->setreadonly(true);
    copytoclip = new checkbox(this, L"复制到剪贴板");
    copytoclip->setcheck(true);
    copytoclipwhenpunch = new checkbox(this, L"识别到标点时才复制");
    copytoclipwhenpunch->setcheck(true);
    mainlayout->addcontrol(copytoclip, 2, 0);
    mainlayout->addcontrol(copytoclipwhenpunch, 2, 1);
    mainlayout->addcontrol(recognizing, 3, 0, 1, 2);
    mainlayout->addcontrol(recognized, 4, 0, 1, 2);
    mainlayout->setfixedheigth(0, 40);
    mainlayout->setfixedheigth(1, 40);
    mainlayout->setfixedheigth(2, 40);
}

int main()
{
    initEnv();

    MainWindow w;
    w.show();
    mainwindow::run();
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    main();
}