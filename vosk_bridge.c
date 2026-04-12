#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

typedef void (*FnSetLogLevel)(int);
typedef void* (*FnModelNew)(const char*);
typedef void (*FnModelFree)(void*);
typedef void* (*FnRecognizerNew)(void*, float);
typedef void (*FnRecognizerFree)(void*);
typedef int (*FnAcceptWaveform)(void*, const char*, int);
typedef const char* (*FnResult)(void*);
typedef const char* (*FnPartialResult)(void*);
typedef const char* (*FnFinalResult)(void*);

static void write_json_line(const char *json)
{
    char compact[4096];
    int j = 0;
    for (int i = 0; json[i] != '\0' && j < (int)sizeof(compact) - 2; i++) {
        if (json[i] != '\n' && json[i] != '\r') {
            compact[j++] = json[i];
        }
    }
    compact[j++] = '\n';
    compact[j] = '\0';
    DWORD written;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), compact, (DWORD)j, &written, NULL);
}

int main(int argc, char *argv[])
{
    _setmode(_fileno(stdout), _O_BINARY);
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 2) {
        write_json_line("{\"type\":\"error\",\"message\":\"Usage: vosk_bridge <model_path>\"}");
        return 1;
    }

    const char *model_path = argv[1];

    HMODULE hVosk = LoadLibraryA("libvosk.dll");
    if (!hVosk) {
        DWORD err = GetLastError();
        char msg[512];
        _snprintf(msg, sizeof(msg), "{\"type\":\"error\",\"message\":\"Failed to load libvosk.dll (error %lu)\"}", err);
        write_json_line(msg);
        return 1;
    }

    FnSetLogLevel fn_set_log_level = (FnSetLogLevel)GetProcAddress(hVosk, "vosk_set_log_level");
    FnModelNew fn_model_new = (FnModelNew)GetProcAddress(hVosk, "vosk_model_new");
    FnModelFree fn_model_free = (FnModelFree)GetProcAddress(hVosk, "vosk_model_free");
    FnRecognizerNew fn_recognizer_new = (FnRecognizerNew)GetProcAddress(hVosk, "vosk_recognizer_new");
    FnRecognizerFree fn_recognizer_free = (FnRecognizerFree)GetProcAddress(hVosk, "vosk_recognizer_free");
    FnAcceptWaveform fn_accept_waveform = (FnAcceptWaveform)GetProcAddress(hVosk, "vosk_recognizer_accept_waveform");
    FnResult fn_result = (FnResult)GetProcAddress(hVosk, "vosk_recognizer_result");
    FnPartialResult fn_partial_result = (FnPartialResult)GetProcAddress(hVosk, "vosk_recognizer_partial_result");
    FnFinalResult fn_final_result = (FnFinalResult)GetProcAddress(hVosk, "vosk_recognizer_final_result");

    if (!fn_model_new || !fn_recognizer_new || !fn_accept_waveform || !fn_result || !fn_partial_result) {
        write_json_line("{\"type\":\"error\",\"message\":\"Failed to resolve VOSK functions\"}");
        FreeLibrary(hVosk);
        return 1;
    }

    fn_set_log_level(-1);

    void *model = fn_model_new(model_path);
    if (!model) {
        char msg[512];
        _snprintf(msg, sizeof(msg), "{\"type\":\"error\",\"message\":\"Failed to load model: %s\"}", model_path);
        write_json_line(msg);
        FreeLibrary(hVosk);
        return 1;
    }

    void *recognizer = fn_recognizer_new(model, 16000.0f);
    if (!recognizer) {
        write_json_line("{\"type\":\"error\",\"message\":\"Failed to create recognizer\"}");
        fn_model_free(model);
        FreeLibrary(hVosk);
        return 1;
    }

    write_json_line("{\"type\":\"ready\"}");

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    char buffer[8192];
    DWORD bytesRead;

    while (ReadFile(hStdin, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        int res = fn_accept_waveform(recognizer, buffer, (int)bytesRead);
        if (res == 1) {
            const char *json = fn_result(recognizer);
            write_json_line(json);
        } else {
            const char *json = fn_partial_result(recognizer);
            write_json_line(json);
        }
    }

    if (fn_final_result) {
        const char *json = fn_final_result(recognizer);
        write_json_line(json);
    }

    fn_recognizer_free(recognizer);
    fn_model_free(model);
    FreeLibrary(hVosk);

    return 0;
}
