//----------------------------------------------------------------------------------------------------
// 2026 - Luis Palacios
//----------------------------------------------------------------------------------------------------

#include "MainWindow.h"
#include "gittoolkit/log.h"
#include "gittoolkit/sync_status.h"

#include <format>

#pragma comment(lib, "comctl32.lib")

namespace gittoolkit::gui {

static std::wstring ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), size);
    return result;
}

GtkMainWindow::GtkMainWindow() = default;

GtkMainWindow::~GtkMainWindow() {
    if (_font != nullptr) {
        DeleteObject(_font);
    }
}

bool GtkMainWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    _hInstance = hInstance;

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = L"GitToolkitMainWindow";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        LogError("No se pudo registrar la clase de ventana");
        return false;
    }

    _hwnd = CreateWindowExW(
        0,
        L"GitToolkitMainWindow",
        L"git-toolkit",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 700,
        nullptr, nullptr, hInstance, this
    );

    if (_hwnd == nullptr) {
        LogError("No se pudo crear la ventana principal");
        return false;
    }

    ShowWindow(_hwnd, nCmdShow);
    UpdateWindow(_hwnd);
    return true;
}

int GtkMainWindow::Run() {
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK GtkMainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GtkMainWindow* pThis = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<GtkMainWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->_hwnd = hwnd;
    } else {
        pThis = reinterpret_cast<GtkMainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis != nullptr) {
        return pThis->HandleMessage(msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT GtkMainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            OnCreate();
            return 0;

        case WM_SIZE:
            OnSize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_COMMAND:
            OnCommand(wParam);
            return 0;

        case WM_NOTIFY:
            OnNotify(reinterpret_cast<LPNMHDR>(lParam));
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(_hwnd, msg, wParam, lParam);
}

void GtkMainWindow::OnCreate() {
    // Fuente Segoe UI 10pt
    _font = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    CreateControls();

    // Cargar configuración
    GtkEngineResult result = _engine.LoadConfig();
    if (result == GtkEngineResult::Ok) {
        _configLoaded = true;
        PopulateTree();
        UpdateStatusBar(std::format(L"Configuración cargada: {} fuentes",
            _engine.GetConfig().sources.size()));
    } else {
        UpdateStatusBar(L"Error cargando configuración");
    }
}

void GtkMainWindow::CreateControls() {
    // Botones de la barra superior
    int btnY = 5;
    int btnH = 28;
    int btnX = 5;

    _btnValidate = CreateWindowW(L"BUTTON", L"Validar", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        btnX, btnY, 80, btnH, _hwnd, reinterpret_cast<HMENU>(IDC_BTN_VALIDATE), _hInstance, nullptr);
    btnX += 85;

    _btnSync = CreateWindowW(L"BUTTON", L"Sincronizar", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        btnX, btnY, 100, btnH, _hwnd, reinterpret_cast<HMENU>(IDC_BTN_SYNC), _hInstance, nullptr);
    btnX += 105;

    _btnStatus = CreateWindowW(L"BUTTON", L"Estado", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        btnX, btnY, 80, btnH, _hwnd, reinterpret_cast<HMENU>(IDC_BTN_STATUS), _hInstance, nullptr);
    btnX += 85;

    _btnPull = CreateWindowW(L"BUTTON", L"Pull", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        btnX, btnY, 60, btnH, _hwnd, reinterpret_cast<HMENU>(IDC_BTN_PULL), _hInstance, nullptr);

    // TreeView (izquierda)
    _treeView = CreateWindowExW(0, WC_TREEVIEWW,
        L"", WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
        5, 40, 300, 500, _hwnd, reinterpret_cast<HMENU>(IDC_TREEVIEW), _hInstance, nullptr);

    // Panel de detalle (derecha, texto multilínea de solo lectura)
    _detailEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
        L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        310, 40, 700, 500, _hwnd, reinterpret_cast<HMENU>(IDC_DETAIL_EDIT), _hInstance, nullptr);

    // Barra de estado inferior
    _statusLabel = CreateWindowW(L"STATIC",
        L"Listo", WS_CHILD | WS_VISIBLE | SS_LEFT,
        5, 545, 1000, 20, _hwnd, reinterpret_cast<HMENU>(IDC_STATUS_LABEL), _hInstance, nullptr);

    // Aplicar fuente
    SendMessage(_btnValidate, WM_SETFONT, reinterpret_cast<WPARAM>(_font), TRUE);
    SendMessage(_btnSync, WM_SETFONT, reinterpret_cast<WPARAM>(_font), TRUE);
    SendMessage(_btnStatus, WM_SETFONT, reinterpret_cast<WPARAM>(_font), TRUE);
    SendMessage(_btnPull, WM_SETFONT, reinterpret_cast<WPARAM>(_font), TRUE);
    SendMessage(_treeView, WM_SETFONT, reinterpret_cast<WPARAM>(_font), TRUE);
    SendMessage(_detailEdit, WM_SETFONT, reinterpret_cast<WPARAM>(_font), TRUE);
    SendMessage(_statusLabel, WM_SETFONT, reinterpret_cast<WPARAM>(_font), TRUE);
}

void GtkMainWindow::OnSize(int width, int height) {
    int treeWidth = 300;
    int topBar = 40;
    int bottomBar = 25;
    int contentH = height - topBar - bottomBar - 5;

    if (_treeView != nullptr) {
        MoveWindow(_treeView, 5, topBar, treeWidth, contentH, TRUE);
    }
    if (_detailEdit != nullptr) {
        MoveWindow(_detailEdit, treeWidth + 10, topBar, width - treeWidth - 15, contentH, TRUE);
    }
    if (_statusLabel != nullptr) {
        MoveWindow(_statusLabel, 5, height - bottomBar, width - 10, 20, TRUE);
    }
}

void GtkMainWindow::OnCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case IDC_BTN_VALIDATE: DoValidate(); break;
        case IDC_BTN_SYNC:     DoSync(); break;
        case IDC_BTN_STATUS:   DoStatus(); break;
        case IDC_BTN_PULL:     DoPull(); break;
    }
}

void GtkMainWindow::OnNotify(LPNMHDR nmhdr) {
    if (nmhdr->idFrom == IDC_TREEVIEW && nmhdr->code == TVN_SELCHANGEDW) {
        NMTREEVIEWW* nmtv = reinterpret_cast<NMTREEVIEWW*>(nmhdr);
        TVITEMW item;
        wchar_t buffer[256] = {};
        item.hItem = nmtv->itemNew.hItem;
        item.mask = TVIF_TEXT | TVIF_PARAM;
        item.pszText = buffer;
        item.cchTextMax = 256;
        TreeView_GetItem(_treeView, &item);

        SetDetailText(std::format(L"Seleccionado: {}", buffer));
    }
}

void GtkMainWindow::PopulateTree() {
    TreeView_DeleteAllItems(_treeView);

    const GtkConfig& config = _engine.GetConfig();
    for (const GtkSourceConfig& source : config.sources) {
        TVINSERTSTRUCTW tvis = {};
        tvis.hParent = TVI_ROOT;
        tvis.hInsertAfter = TVI_LAST;
        tvis.item.mask = TVIF_TEXT;

        std::wstring sourceText = ToWide(source.key);
        tvis.item.pszText = const_cast<LPWSTR>(sourceText.c_str());
        HTREEITEM hSource = TreeView_InsertItem(_treeView, &tvis);

        for (const GtkRepoConfig& repo : source.repos) {
            TVINSERTSTRUCTW rvis = {};
            rvis.hParent = hSource;
            rvis.hInsertAfter = TVI_LAST;
            rvis.item.mask = TVIF_TEXT;

            std::wstring repoText = ToWide(repo.name);
            rvis.item.pszText = const_cast<LPWSTR>(repoText.c_str());
            TreeView_InsertItem(_treeView, &rvis);
        }
    }
}

void GtkMainWindow::UpdateStatusBar(const std::wstring& text) {
    SetWindowTextW(_statusLabel, text.c_str());
}

void GtkMainWindow::SetDetailText(const std::wstring& text) {
    SetWindowTextW(_detailEdit, text.c_str());
}

void GtkMainWindow::DoValidate() {
    UpdateStatusBar(L"Validando...");
    GtkConfigResult result = _engine.Validate();
    if (result == GtkConfigResult::Ok) {
        const GtkConfig& config = _engine.GetConfig();
        int repoCount = 0;
        for (const GtkSourceConfig& s : config.sources) {
            repoCount += static_cast<int>(s.repos.size());
        }
        SetDetailText(std::format(L"✓ Configuración válida\r\n\r\nFuentes: {}\r\nRepos: {}",
            config.sources.size(), repoCount));
        UpdateStatusBar(L"Validación correcta");
    } else {
        SetDetailText(L"✗ Configuración inválida. Revisa el log.");
        UpdateStatusBar(L"Error de validación");
    }
}

void GtkMainWindow::DoSync() {
    if (!_configLoaded) return;
    UpdateStatusBar(L"Sincronizando...");
    SetDetailText(L"Sincronizando repositorios...\r\nEsto puede tardar unos segundos.");

    GtkSyncOptions options;
    GtkEngineResult result = _engine.Sync(options);

    if (result == GtkEngineResult::Ok) {
        SetDetailText(L"✓ Sincronización completada");
        UpdateStatusBar(L"Sincronización completada");
    } else {
        SetDetailText(L"⚠ Sincronización parcial (revisa el log)");
        UpdateStatusBar(L"Sincronización con errores");
    }
    PopulateTree();
}

void GtkMainWindow::DoStatus() {
    if (!_configLoaded) return;
    UpdateStatusBar(L"Comprobando estado...");

    GtkStatusOptions options;
    options.verbose = true;
    std::vector<sync::GtkRepoStatusInfo> statuses = _engine.GetStatus(options);

    std::wstring detail;
    int clean = 0, needsPull = 0, review = 0, errors = 0;

    for (const sync::GtkRepoStatusInfo& info : statuses) {
        std::wstring statusStr = ToWide(sync::StatusToString(info.status));
        detail += std::format(L"{} [{}]\r\n", ToWide(info.path), statusStr);

        if (info.status != sync::GtkRepoStatus::Clean) {
            if (!info.branch.empty())
                detail += std::format(L"  Rama: {}\r\n", ToWide(info.branch));
            if (info.ahead > 0)
                detail += std::format(L"  Adelantados: {}\r\n", info.ahead);
            if (info.behind > 0)
                detail += std::format(L"  Atrasados: {}\r\n", info.behind);
            if (info.modified > 0)
                detail += std::format(L"  Modificados: {}\r\n", info.modified);
            if (info.untracked > 0)
                detail += std::format(L"  Sin rastrear: {}\r\n", info.untracked);
            if (!info.errorMessage.empty())
                detail += std::format(L"  Error: {}\r\n", ToWide(info.errorMessage));
        }
        detail += L"\r\n";

        switch (info.status) {
            case sync::GtkRepoStatus::Clean:
            case sync::GtkRepoStatus::BehindMain:
                clean++; break;
            case sync::GtkRepoStatus::NeedsPull:  needsPull++; break;
            case sync::GtkRepoStatus::NeedsReview: review++; break;
            case sync::GtkRepoStatus::Error:       errors++; break;
        }
    }

    detail += std::format(L"---\r\nTotal: {} repos — {} limpios, {} pull, {} revisar, {} errores\r\n",
        statuses.size(), clean, needsPull, review, errors);

    SetDetailText(detail);
    UpdateStatusBar(std::format(L"Estado: {} repos ({} limpios, {} pull, {} revisar)",
        statuses.size(), clean, needsPull, review));
}

void GtkMainWindow::DoPull() {
    if (!_configLoaded) return;
    UpdateStatusBar(L"Haciendo pull...");

    GtkStatusOptions options;
    GtkEngineResult result = _engine.PullSafe(options);

    if (result == GtkEngineResult::Ok) {
        SetDetailText(L"✓ Pull completado en todos los repos seguros");
        UpdateStatusBar(L"Pull completado");
    } else {
        SetDetailText(L"⚠ Pull parcial (algunos repos con errores)");
        UpdateStatusBar(L"Pull con errores");
    }
}

} // namespace gittoolkit::gui
