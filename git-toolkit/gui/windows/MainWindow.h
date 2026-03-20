//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "gittoolkit/engine.h"

#include <string>
#include <vector>

namespace gittoolkit::gui {

/// Identificadores de controles
enum ControlIds {
    IDC_TREEVIEW = 1001,
    IDC_STATUS_LABEL = 1002,
    IDC_DETAIL_EDIT = 1003,
    IDC_BTN_SYNC = 2001,
    IDC_BTN_STATUS = 2002,
    IDC_BTN_PULL = 2003,
    IDC_BTN_VALIDATE = 2004,
};

/// Ventana principal de git-toolkit.
class GtkMainWindow {
public:
    GtkMainWindow();
    ~GtkMainWindow();

    /// Crear y mostrar la ventana.
    bool Create(HINSTANCE hInstance, int nCmdShow);

    /// Bucle de mensajes.
    int Run();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void OnCreate();
    void OnSize(int width, int height);
    void OnCommand(WPARAM wParam);
    void OnNotify(LPNMHDR nmhdr);

    void CreateControls();
    void PopulateTree();
    void UpdateStatusBar(const std::wstring& text);
    void SetDetailText(const std::wstring& text);

    // Operaciones (ejecutadas en hilo principal por ahora)
    void DoValidate();
    void DoSync();
    void DoStatus();
    void DoPull();

    HWND _hwnd = nullptr;
    HWND _treeView = nullptr;
    HWND _detailEdit = nullptr;
    HWND _statusLabel = nullptr;
    HWND _btnSync = nullptr;
    HWND _btnStatus = nullptr;
    HWND _btnPull = nullptr;
    HWND _btnValidate = nullptr;

    HINSTANCE _hInstance = nullptr;
    HFONT _font = nullptr;

    GtkEngine _engine;
    bool _configLoaded = false;
};

} // namespace gittoolkit::gui
