/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
 *             chenglu <chenglu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <DMainWindow>
#include <QSettings>
#include <DTitlebar>
#include <DFileWatcher>
#include <QElapsedTimer>
#include <DIconButton>

#include "homepage.h"
#include "uncompresspage.h"
#include "compresspage.h"
#include "compresssetting.h"
#include "progress.h"
#include "compressor_success.h"
#include "compressor_fail.h"
#include "archive_manager.h"
#include "archivemodel.h"
#include "encryptionpage.h"
#include "progressdialog.h"
#include "extractpausedialog.h"
#include "settingdialog.h"
#include "encodingpage.h"
#include <DIconButton>
#include "archivesortfiltermodel.h"
#include "batchextract.h"
#include "batchcompress.h"
#include "openloadingpage.h"
#include <DFileWatcher>
#include <QElapsedTimer>
#include <QQueue>


#define TITLE_FIXED_HEIGHT 50
#define HEADBUS "/QtDusServer/registry"

DWIDGET_USE_NAMESPACE

enum Page_ID {
    PAGE_HOME,
    PAGE_UNZIP,
    PAGE_ZIP,
    PAGE_ZIPSET,
    PAGE_ZIPPROGRESS,
    PAGE_UNZIPPROGRESS,
    PAGE_ZIP_SUCCESS,
    PAGE_ZIP_FAIL,
    PAGE_UNZIP_SUCCESS,
    PAGE_UNZIP_FAIL,
    PAGE_ENCRYPTION,
    PAGE_DELETEPROGRESS,
    PAGE_MAX,
    PAGE_LOADING
};

enum EncryptionType {
    Encryption_NULL,
    Encryption_Load,
    Encryption_Extract,
    Encryption_SingleExtract,// “提取”
    Encryption_ExtractHere,
    Encryption_TempExtract,
    Encryption_TempExtract_Open,
    Encryption_TempExtract_Open_Choose,
    Encryption_DRAG
};

enum WorkState {
    WorkNone,
    WorkProcess,
};

class QStackedLayout;
class TimerWatcher;
enum JobState {
    JOB_NULL,
    JOB_ADD,
    JOB_DELETE,
    JOB_DELETE_MANUAL,//手动delete，而非消息通知的delete
    JOB_CREATE,
    JOB_LOAD,
    JOB_COPY,
    JOB_BATCHEXTRACT,
    JOB_EXTRACT,
    JOB_TEMPEXTRACT,
    JOB_MOVE,
    JOB_COMMENT,
    JOB_BATCHCOMPRESS,
};

class MainWindow;
class Settings_Extract_Info;
/**
 * this can help us to get the map of all mainwindow created.
 * @brief The GlobalMainWindowMap struct
 */
struct GlobalMainWindowMap {
public:
    void insert(const QString &strWinId, MainWindow *wnd)
    {
        if (this->mMapGlobal.contains(strWinId) == false) {
            this->mMapGlobal.insert(strWinId, wnd);
        }
    }

    MainWindow *getOne(const QString &strWinId)
    {
        if (this->mMapGlobal.contains(strWinId) == false) {
            return nullptr;
        } else {
            return this->mMapGlobal[strWinId];
        }
    }

    void remove(const QString &strWinId)
    {
        if (this->mMapGlobal.contains(strWinId) == true) {
            this->mMapGlobal.remove(strWinId);
        }
    }

    void clear()
    {
        this->mMapGlobal.clear();
    }

    /**
     * @brief mMapGlobal
     * @ key: winId
     * @ value: pointer of mainWindow
     */
    QMap<QString, MainWindow *> mMapGlobal = {};
};

struct OpenInfo {
    enum ENUM_OPTION {
        CLOSE = 0,//正常关闭
        OPEN = 1,//打开
        QUERY_CLOSE_CANCEL = 2//询问后，关闭取消
    };

    // 逻辑子窗口的WinId
    QString strWinId = "";
    // 逻辑子窗口的状态
    ENUM_OPTION option = OPEN;
    // 逻辑子窗口的job
    KJob *pJob = nullptr;
};

/**
 * @brief The MainWindow_AuxInfo struct
 * @see 存放MainWindow的重要辅助信息
 */
struct MainWindow_AuxInfo {
    /**
     * @brief infomation
     * @see节点详情
     * @ key :strModexIndex,see as modelIndexToStr()
     * @ value :the pointer of open info
     */
    QMap<QString, OpenInfo *> information;
    /**
     * @brief parentAuxInfo
     * @see 逻辑父面板辅助信息节点
     */
    MainWindow_AuxInfo *parentAuxInfo = nullptr;
};

static QVector<qint64> m_tempProcessId;
class QStackedLayout;

class MonitorAdaptor;

class MainWindow : public DMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.archive.mainwindow.monitor")

public:
    static int m_windowcount;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    /**
     * @brief closeClean
     * @param event
     * @see 每次关闭窗口尽量手动释放更多的内存，因为我们的窗口close()实际上执行的是hide();
     */
    void closeClean(QCloseEvent *event);
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void InitUI();
    void InitConnection();
    void initTitleBar();
    QMenu *createSettingsMenu();
    void loadArchive(const QString &files);
    void creatArchive(QMap<QString, QString> &Args);
    void creatBatchArchive(QMap<QString, QString> &Args, QMap<QString, QStringList> &filetoadd);
    void addArchive(QMap<QString, QString> &Args);
    void addArchiveEntry(QMap<QString, QString> &args, Archive::Entry *pWorkEntry);

    /**
     * @brief removeEntryVector
     * @param vectorDel
     * @param isManual,true:by action clicked; false: by message emited.
     * @see true:手动删除；false:子面板消息通知删除。
     */
    void removeEntryVector(QVector<Archive::Entry *> &vectorDel, bool isManual);
    void moveToArchive(QMap<QString, QString> &Args);

    void transSplitFileName(QString &fileName); // *.7z.003 -> *.7z.001

    void ExtractPassword(QString password);
    void ExtractSinglePassword(QString password);
    void LoadPassword(QString password);
    void WatcherFile(const QString &files);
    void renameCompress(QString &filename, QString fixedMimeType);
    QString getLoadFile();
    qint64 getDiskFreeSpace();
    qint64 getMediaFreeSpace();

    bool applicationQuit();
    QString getAddFile();
    bool isWorkProcess();
    bool checkSettings(QString file);
    //log
    //void initalizeLog(QWidget *widget);
    //void logShutDown();
    void bindAdapter();
    //static Log4Qt::Logger *getLogger();
    OpenInfo::ENUM_OPTION option = OpenInfo::OPEN;
    QString *strChildMndExtractPath = nullptr;//保存的有次级面板解压路径（用户解压路径，非临时路径），该变量其他地方用不到
    QString *strParentArchivePath = nullptr;//保存第一级压缩包路径
private:
    void saveWindowState();
    void loadWindowState();
    QString modelIndexToStr(const QModelIndex &modelIndex);//added by hsw 20200525
    int queryDialogForClose();
protected:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

public slots:
    //accept subwindows drag files and return tips string
    bool onSubWindowActionFinished(int mode, const qint64 &pid, const QStringList &urls);

    bool popUpChangedDialog(const qint64 &pid);

    bool createSubWindow(const QStringList &urls);

private slots:
    void setEnable();
    void setDisable();
    void refreshPage();
    void onSelected(const QStringList &);
    void onRightMenuSelected(const QStringList &);
    void onCompressNext();
    void onCompressPressed(QMap<QString, QString> &Args);
    void onUncompressStateAutoCompress(QMap<QString, QString> &Args);
    // added by hsw 20200525
    void onUncompressStateAutoCompressEntry(QMap<QString, QString> &Args, Archive::Entry *pWorkEntry = nullptr);
    void onCancelCompressPressed(Progress::ENUM_PROGRESS_TYPE compressType);
    void onTitleButtonPressed();
    void onCompressAddfileSlot(bool status);

    void slotLoadingFinished(KJob *job);
    void slotExtractionDone(KJob *job);
    void slotShowPageUnzipProgress();
    void slotextractSelectedFilesTo(const QString &localPath);
    void SlotProgress(KJob *job, unsigned long percent);
    void SlotProgressFile(KJob *job, const QString &filename);
    void SlotNeedPassword();
    void SlotExtractPassword(QString password);
    void slotCompressFinished(KJob *job);
    void slotJobFinished(KJob *job);
    void slotExtractSimpleFiles(QVector<Archive::Entry *> fileList, QString path, EXTRACT_TYPE type);
    void slotExtractSimpleFilesOpen(const QVector<Archive::Entry *> &fileList, const QString &programma);
    void slotKillExtractJob();
    void slotFailRetry();
    void slotBatchExtractFileChanged(const QString &name);
    void slotBatchExtractError(const QString &name);
    void slotClearTempfile();
    void slotquitApp();
    void onUpdateDestFile(QString destFile);
    void onCompressPageFilelistIsEmpty();

    void slotCalDeleteRefreshTotalFileSize(const QStringList &files);

    /**
     * @brief slotUncompressCalDeleteRefreshTotoalSize
     * @param vectorDel
     * @param isManual,true:by action clicked; false: by message emited.
     */
    void slotUncompressCalDeleteRefreshTotoalSize(QVector<Archive::Entry *> &vectorDel, bool isManual);

    void resetMainwindow();
    void slotBackButtonClicked();
    void slotResetPercentAndTime();
    void slotFileUnreadable(QStringList &pathList, int fileIndex);//compress file is unreadable or file is a link
    void slotStopSpinner();
    void slotWorkTimeOut();

    void deleteFromArchive(const QStringList &files, const QString &archive);
    void closeExtractJobSafe();
//    void addToArchive(const QStringList &files, const QString &archive);//废弃，added by hsw 20200528

signals:
    void sigquitApp();
    void sigZipAddFile();
    void sigCompressedAddFile();
    void sigZipReturn();
    void sigZipSelectedFiles(const QStringList &files);
    void loadingStarted();
    void sigUpdateTableView(const QFileInfo &);
    void sigTipsWindowPopUp(int, const QStringList &);
    void sigTipsUpdateEntry(int, QVector<Archive::Entry *> &vectorDel);
    void deleteJobComplete(Archive::Entry *pEntry);

private:
    MonitorAdaptor *m_adaptor = nullptr;
    Archive *m_archive_manager = nullptr;
    ArchiveModel *m_model = nullptr;
    ArchiveSortFilterModel *m_filterModel = nullptr;
    QString m_decompressfilename;
    QString m_decompressfilepath;
    QString m_loadfile;
    QString m_addFile;

    void setCompressDefaultPath();
    void setQLabelText(QLabel *label, const QString &text);
    QJsonObject creatShorcutJson();

    QStringList CheckAllFiles(QString path);
    void deleteCompressFile(/*QStringList oldfiles, QStringList newfiles*/);
    void deleteDecompressFile(QString destDirName = "");
    /**
     * @brief startCmd
     * @param executeName
     * @param arguments
     * @see 启动一个命令，完成后自动销毁
     */
    bool startCmd(const QString &executeName, QStringList arguments);
    void removeFromParentInfo(MainWindow *);
private:
//    DLabel *m_logo;
    QPixmap m_logoicon;
//    QFrame *m_titleFrame;
//    DLabel *m_titlelabel;
    DWidget *m_mainWidget;
    QStackedLayout *m_mainLayout;
    HomePage *m_homePage;
    UnCompressPage *m_UnCompressPage;
    CompressPage *m_CompressPage;
    CompressSetting *m_CompressSetting;
    Progress *m_Progess;
    Compressor_Success *m_CompressSuccess;
    Compressor_Fail *m_CompressFail;
    EncryptionPage *m_encryptionpage;
    ProgressDialog *m_progressdialog;
    SettingDialog *m_settingsDialog = nullptr;
    OpenLoadingPage *m_pOpenLoadingPage;
    EncodingPage *m_encodingpage;
    QSettings *m_settings;
    Page_ID m_pageid;

    QVector<Archive::Entry *> m_extractSimpleFiles;

    DIconButton *m_titlebutton = nullptr;

    KJob *m_pJob = nullptr;// 指向所有Job派生类对象
    EncryptionType m_encryptiontype = Encryption_NULL;
    bool m_isrightmenu = false;
    WorkState m_workstatus = WorkNone;
    JobState m_jobState = JOB_NULL;

    int m_timerId = 0;

    QAction *m_openAction;

    QString createCompressFile_;

    QString m_pathstore;
    bool m_initflag = false;
    int m_startTimer = 0;
    int m_watchTimer = 0;

    DFileWatcher *m_fileManager = nullptr;
    int openTempFileLink = 0;
    QEventLoop *pEventloop = nullptr;
    DSpinner *m_spinner = nullptr;

    TimerWatcher *m_pWatcher = nullptr;
    // bool m_openType = false; //false解压 true打开
    bool IsAddArchive = false;

    GlobalMainWindowMap *pMapGlobalWnd = nullptr;//added by hsw 20200521
    MonitorAdaptor *pAdapter = nullptr;//added by hsw 20200521
    /**
     * @brief pCurAuxInfo
     * @see 当前面板辅助信息
     */
    MainWindow_AuxInfo *pCurAuxInfo = nullptr;//added by hsw 20200525
    Settings_Extract_Info *pSettingInfo = nullptr;//added by hsw 20200619

private:
    void calSelectedTotalFileSize(const QStringList &files);
    void calSelectedTotalEntrySize(QVector<Archive::Entry *> &vectorDel);
    qint64 calFileSize(const QString &path);
    void calSpeedAndTime(unsigned long compressPercent);

    QString getDefaultApp(QString mimetype);
    void setDefaultApp(QString mimetype, QString desktop);
    int promptDialog();
    QReadWriteLock m_lock;
    QString program;
    QMap<qint64, QStringList> m_subWinDragFiles;
    int m_mode = 0;
    qint64 m_curOperChildPid = 0;

#ifdef __aarch64__
    qint64 maxFileSize_ = 0;
#endif

};














