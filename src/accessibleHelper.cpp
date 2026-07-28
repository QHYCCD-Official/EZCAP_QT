#include "accessibleHelper.h"
#include "accessibleAction.h"
#include "ezCap.h"
#include "mainMenu.h"
#include "managementMenu.h"
#include "ui_ezCap.h"
#include "ui_managementMenu.h"

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QAction>
#include <QMenu>
#include <QMenuBar>

#ifdef Q_OS_WIN
#include <windows.h>
#include <oleacc.h>
#endif

// Forward declarations of internal helpers
static void setupManagementMenuAccessibility(ManagementMenu *menu);
static void setupMainMenuAccessibility(MainMenu *menu);
static void setupWin32Accessibility(QWidget *mainWindow);

void setupAccessibility(EZCAP *mainWindow)
{
    setupMenuAccessibility();

    Ui::EZCAP *ui = mainWindow->ui;

    mainWindow->setAccessibleName(QObject::tr("EZCAP Main Window"));
    mainWindow->setAccessibleDescription(QObject::tr("QHYCCD Astronomy Camera Control"));

    ui->label_ImgShow->setAccessibleName(QObject::tr("Image Display"));
    ui->label_ImgShow->setAccessibleDescription(QObject::tr("Camera image preview and capture display area"));

    ui->plainTextEdit_debug->setAccessibleName(QObject::tr("Debug Output"));
    ui->plainTextEdit_debug->setAccessibleDescription(QObject::tr("Debug information and log output"));

    ui->verticalScrollBar_ImgShow->setAccessibleName(QObject::tr("Image Vertical Scroll"));
    ui->horizontalScrollBar_ImgShow->setAccessibleName(QObject::tr("Image Horizontal Scroll"));

    ui->statusBar->setAccessibleName(QObject::tr("Status Bar"));

    ui->pBtn_linear->setAccessibleName(QObject::tr("Linear Mode"));
    ui->pBtn_thermal->setAccessibleName(QObject::tr("Thermal Mode"));
    ui->pBtn_false->setAccessibleName(QObject::tr("False Color"));
    ui->pBtn_invert->setAccessibleName(QObject::tr("Invert"));

    if (mainWindow->scrollArea_ImgShow) {
        mainWindow->scrollArea_ImgShow->setAccessibleName(QObject::tr("Image Scroll Area"));
    }

    if (mainMenuBar) {
        setupMainMenuAccessibility(mainMenuBar);
    }

    if (managerMenu) {
        setupManagementMenuAccessibility(managerMenu);
    }

#ifdef Q_OS_WIN
    setupWin32Accessibility(mainWindow);
#endif
}

static void setupManagementMenuAccessibility(ManagementMenu *menu)
{
    Ui::ManagementMenu *ui = menu->ui;

    ui->head_preview->setAccessibleName(QObject::tr("Preview"));
    ui->head_focus->setAccessibleName(QObject::tr("Focus"));
    ui->head_capture->setAccessibleName(QObject::tr("Capture"));
    ui->head_save->setAccessibleName(QObject::tr("Save"));
    ui->head_liveimageformat->setAccessibleName(QObject::tr("Live Image Format"));
    ui->head_livecamerasetup->setAccessibleName(QObject::tr("Camera Setup"));
    ui->head_liveimagesetup->setAccessibleName(QObject::tr("Image Setup"));
    ui->head_Roi->setAccessibleName(QObject::tr("ROI"));
    ui->head_screenView->setAccessibleName(QObject::tr("Screen View"));
    ui->head_hist->setAccessibleName(QObject::tr("Histogram"));

    ui->label_Gain_preview->setAccessibleDescription(QObject::tr("Preview Gain"));
    ui->label_Offset_preview->setAccessibleDescription(QObject::tr("Preview Offset"));
    ui->label_expunit_preview->setAccessibleDescription(QObject::tr("Preview Exposure Unit"));
    ui->lineEdit_exposure_preview->setAccessibleDescription(QObject::tr("Preview Exposure Value"));
    ui->hSlider_exposure_preview->setAccessibleDescription(QObject::tr("Preview Exposure Slider"));
    ui->hSlider_Gain_preview->setAccessibleDescription(QObject::tr("Preview Gain Slider"));
    ui->hSlider_Offset_preview->setAccessibleDescription(QObject::tr("Preview Offset Slider"));

    ui->pBtn_preview->setAccessibleName(QObject::tr("Start Preview"));
    ui->pBtn_live_preview->setAccessibleName(QObject::tr("Live Preview"));
    ui->pBtn_cross->setAccessibleName(QObject::tr("Crosshair"));
    ui->pBtn_grid->setAccessibleName(QObject::tr("Grid"));
    ui->pBtn_circle->setAccessibleName(QObject::tr("Circle"));

    ui->pBtn_coarse->setAccessibleName(QObject::tr("Coarse Stretch"));
}

static void setupMainMenuAccessibility(MainMenu *menu)
{
    menu->menuFile->setAccessibleName(QObject::tr("File Menu"));
    menu->menuCamera->setAccessibleName(QObject::tr("Camera Menu"));
    menu->menuPlanner->setAccessibleName(QObject::tr("Planner Menu"));
    menu->menuImageProcess->setAccessibleName(QObject::tr("Image Process Menu"));
    menu->menuImageRotate->setAccessibleName(QObject::tr("Image Rotate Menu"));
    menu->menuImageMirror->setAccessibleName(QObject::tr("Image Mirror Menu"));
    menu->menuCameraSetup->setAccessibleName(QObject::tr("Camera Setup Menu"));
    menu->menuTools->setAccessibleName(QObject::tr("Tools Menu"));
    menu->menuZoom->setAccessibleName(QObject::tr("Zoom Menu"));
    menu->menuLanguage->setAccessibleName(QObject::tr("Language Menu"));
    menu->menuHelp->setAccessibleName(QObject::tr("Help Menu"));
}

#ifdef Q_OS_WIN
static void setupWin32Accessibility(QWidget *mainWindow)
{
    HWND hwnd = (HWND)mainWindow->winId();
    mainWindow->setAttribute(Qt::WA_NativeWindow);

    HMODULE hOleAcc = LoadLibraryW(L"oleacc.dll");
    if (hOleAcc) {
        HRESULT (WINAPI *pCreateAccObj)(HWND, DWORD, REFIID, void**) =
            (HRESULT (WINAPI*)(HWND, DWORD, REFIID, void**))GetProcAddress(hOleAcc, "CreateStdAccessibleObject");
        if (pCreateAccObj) {
            IAccessible *pAcc = NULL;
            if (SUCCEEDED(pCreateAccObj(hwnd, OBJID_CLIENT, IID_IAccessible, (void**)&pAcc)) && pAcc) {
                pAcc->Release();
            }
        }
        FreeLibrary(hOleAcc);
    }

    NotifyWinEvent(EVENT_OBJECT_CREATE, hwnd, OBJID_WINDOW, INDEXID_OBJECT);
}
#endif
