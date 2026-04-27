#pragma once

#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QList>
#include <QFileDialog>
#include <QTextStream>
#include <QDockWidget>
#include "graph.h"
#include "qactiongroup.h"
#include "qpushbutton.h"
#include "qshortcut.h"
#include "qspinbox.h"
#include "qtableview.h"

// Форвард-декларация
class NodeInfoWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct GraphState {
    unsigned int amount;
    Matrix2D adj;
    Matrix2D flow;
    Matrix2D band;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    Graph graph;
    ~MainWindow();

private slots:
    void buttonClearConsoleClicked();
    void viewModeChecked(bool checked);
    void myCopy();
    void myPaste();
    void setNodesAmountMatrix(QTableView *table, int newAmount);
    void saveToFile();

private:
    Ui::MainWindow *ui;
    QMap<QWidget *, QPushButton *> pins;
    QMap<QString, QWidget *> docksViewMode;

    QList<GraphState> undoStack;
    int currentStateIndex = -1;
    void saveState();
    void restoreState(const GraphState& state);

    // Тест прикинь

    void unpinTab(int index);
    void pinTab();
    void pasteClipboardToTable(QTableView *dest);
    void copyTableToClipboard(QTableView *src);
    void applyGraphMatrix(QTableView *table);
    void applyEdgesList(QTableView *table);
    void updateEdgesList(QTableView *list);
    void updateTables();

    template<typename T>
    void setTableFromMatrix(QTableView *table, T &matrix, int height = -1, int width = -1);

    QActionGroup *nodeMovementGroup;
    QList<QTableView *> graphMatrixViews;
    QList<QTableView *> graphListViews;
    QMap<QString, QSpinBox *> graphCountSpins;

           // --- Указатели для новой боковой панели ---
    QDockWidget *dock_NodeInfo;
    NodeInfoWidget *nodeSidebar;
};