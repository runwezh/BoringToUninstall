#pragma once

#include "AppScanner.h"
#include "UninstallEngine.h"
#include "SafetyChecker.h"
#include "Version.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QGroupBox>
#include <QTimer>
#include <QSettings>

class BTUMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BTUMainWindow(QWidget* parent = nullptr);
    ~BTUMainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // 界面操作
    void onSearchTextChanged(const QString& text);
    void onRefreshClicked();
    void onSelectAllClicked();
    void onUnselectAllClicked();
    void onUninstallClicked();
    void onExitClicked();
    
    // 应用扫描相关
    void onScanStarted();
    void onScanFinished();
    void onApplicationFound(const ApplicationInfo& appInfo);
    void onScanProgress(int current, int total);
    void onScanError(const QString& error);
    
    // 卸载相关
    void onUninstallStarted(const QString& appName);
    void onUninstallFinished(const QString& appName, UninstallResult result);
    void onUninstallProgress(const QString& appName, const UninstallProgress& progress);
    void onUninstallError(const QString& appName, const QString& error);
    void onAllUninstallsFinished();
    
    // 表格操作
    void onTableItemChanged(QTableWidgetItem* item);
    void onTableSelectionChanged();
    void onTableItemDoubleClicked(QTableWidgetItem* item);
    
    // 菜单操作
    void onAboutClicked();
    void onSettingsClicked();
    void onViewLogClicked();
    
    // 定时器
    void updateStatusInfo();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    
    void populateApplicationTable();
    void addApplicationToTable(const ApplicationInfo& appInfo);
    void updateApplicationTable();
    void filterApplications();
    
    QList<ApplicationInfo> getSelectedApplications() const;
    void updateSelectionInfo();
    void setUIEnabled(bool enabled);
    
    void showApplicationDetails(const ApplicationInfo& appInfo);
    QString formatUninstallResult(UninstallResult result) const;
    
    // UI组件
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_topLayout;
    QHBoxLayout* m_bottomLayout;
    
    // 搜索区域
    QLineEdit* m_searchEdit;
    QPushButton* m_refreshButton;
    
    // 应用列表
    QTableWidget* m_appTable;
    
    // 操作按钮
    QCheckBox* m_selectAllCheckBox;
    QPushButton* m_unselectAllButton;
    QPushButton* m_uninstallButton;
    QPushButton* m_exitButton;
    
    // 状态显示
    QLabel* m_statusLabel;
    QLabel* m_selectionLabel;
    QProgressBar* m_progressBar;
    
    // 菜单和工具栏
    QMenuBar* m_menuBar;
    QMenu* m_fileMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;
    QAction* m_exitAction;
    QAction* m_refreshAction;
    QAction* m_settingsAction;
    QAction* m_viewLogAction;
    QAction* m_aboutAction;
    
    // 对话框
    QProgressDialog* m_scanProgressDialog;
    QProgressDialog* m_uninstallProgressDialog;
    
    // 核心组件
    AppScanner* m_scanner;
    UninstallEngine* m_uninstallEngine;
    
    // 数据
    QList<ApplicationInfo> m_allApplications;
    QList<ApplicationInfo> m_filteredApplications;
    
    // 状态
    bool m_isScanning;
    bool m_isUninstalling;
    QString m_currentFilter;
    
    // 设置
    QSettings* m_settings;
    QTimer* m_statusTimer;
    
    // 表格列枚举
    enum TableColumn {
        ColumnCheckBox = 0,
        ColumnName = 1,
        ColumnVersion = 2,
        ColumnPublisher = 3,
        ColumnSize = 4,
        ColumnInstallDate = 5,
        ColumnLocation = 6,
        ColumnCount
    };
};
