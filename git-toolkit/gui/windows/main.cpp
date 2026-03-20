//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "MainWindow.h"
#include "gittoolkit/log.h"

#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    gittoolkit::GtkLog::Instance().SetLevel(gittoolkit::GtkLogLevel::Debug);

    gittoolkit::gui::GtkMainWindow mainWindow;
    if (!mainWindow.Create(hInstance, nCmdShow)) {
        MessageBoxW(nullptr, L"No se pudo crear la ventana principal",
                    L"git-toolkit", MB_ICONERROR);
        return 1;
    }

    return mainWindow.Run();
}
