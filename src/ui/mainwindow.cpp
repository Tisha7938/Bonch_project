#include "mainwindow.h"
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QStandardItemModel>
#include <QStyle>
#include <QTextStream>
#include <QToolTip>
#include <set>
#include "qspinbox.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // --- Настройка вкладок ---
    this->setDockOptions(this->dockOptions() & ~QMainWindow::VerticalTabs);
    this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // --- Иконки и Подсказки ---
    ui->actionUndo->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    ui->actionUndo->setToolTip("Отменить последнее действие (Ctrl+Z)");

    ui->actionRedo->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    ui->actionRedo->setToolTip("Повторить отмененное действие (Ctrl+Y)");

    ui->actionSave_as->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionSave_as->setToolTip("Сохранить граф в файл (Ctrl+Shift+S)");

    ui->actionCopy->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    ui->actionCopy->setToolTip("Копировать выбранные ячейки");

    ui->actionPaste->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
    ui->actionPaste->setToolTip("Вставить данные из буфера");

    ui->actionLaunchAlg->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->actionLaunchAlg->setToolTip("Запустить расчет алгоритма");

    ui->actionDeleteGraph->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    ui->actionDeleteGraph->setToolTip("Полностью удалить граф");

    ui->actionAddNode->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    ui->actionAddNode->setToolTip("Добавить новый узел");

    ui->actionDeleteNode->setIcon(style()->standardIcon(QStyle::SP_DialogDiscardButton));
    ui->actionDeleteNode->setToolTip("Удалить последний узел");

    ui->actionRefreshTables->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->actionRefreshTables->setToolTip("Синхронизировать данные таблиц");

    ui->actionClearConsole->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    ui->actionClearConsole->setToolTip("Очистить историю в консоли");

    connect(ui->actionSave_as, &QAction::triggered, this, &MainWindow::saveToFile);

    connect(ui->actionAddNode, &QAction::triggered, this, [this]() {
        saveState();
        int newAmount = graph.getAmount() + 1;
        graph.resizeGraph(graph.getAmount(), newAmount);
        graph.graphView->initScene();
        updateTables();
        ui->textEdit_Console->appendPlainText("Добавлен узел. Всего: " + QString::number(newAmount));
    });

    connect(ui->actionDeleteNode, &QAction::triggered, this, [this]() {
        if (graph.getAmount() > 0) {
            saveState();
            int newAmount = graph.getAmount() - 1;
            graph.resizeGraph(graph.getAmount(), newAmount);
            graph.graphView->scene()->update();
            updateTables();
            ui->textEdit_Console->appendPlainText("Узел удален. Осталось: " + QString::number(newAmount));
        }
    });

    connect(ui->actionDeleteGraph, &QAction::triggered, this, [this]() {
        saveState();
        graph.resizeGraph(graph.getAmount(), 0);
        graph.graphView->scene()->update();
        updateTables();
        ui->textEdit_Console->appendPlainText("Граф полностью очищен.");
    });

    connect(ui->actionClearConsole, &QAction::triggered, this, &MainWindow::buttonClearConsoleClicked);

    connect(ui->actionUndo, &QAction::triggered, this, [this]() {
        if (currentStateIndex > 0) {
            currentStateIndex--;
            restoreState(undoStack[currentStateIndex]);
            ui->textEdit_Console->appendPlainText("<- Undo");
        } else {
            ui->textEdit_Console->appendPlainText("Нечего отменять.");
        }
    });

    connect(ui->actionRedo, &QAction::triggered, this, [this]() {
        if (currentStateIndex < undoStack.size() - 1) {
            currentStateIndex++;
            restoreState(undoStack[currentStateIndex]);
            ui->textEdit_Console->appendPlainText("-> Redo");
        } else {
            ui->textEdit_Console->appendPlainText("Нечего повторять.");
        }
    });

    connect(ui->actionLaunchAlg, &QAction::triggered, this, [this]() {
        ui->textEdit_Console->appendPlainText("\n=== АНАЛИЗ ГРАФА ===");
        auto edgesList = graph.getListEdges();
        ui->textEdit_Console->appendPlainText("Узлов: " + QString::number(graph.getAmount()));
        ui->textEdit_Console->appendPlainText("Ребер: " + QString::number(edgesList.size()));
        ui->textEdit_Console->appendPlainText("====================\n");
    });

    auto spinBoxes = this->findChildren<QSpinBox *>();
    auto pushButtons = this->findChildren<QPushButton *>();

    for (auto *button: pushButtons) {
        if (button->text().contains("Clear") || button->objectName().contains("clear")) {
            connect(button, &QPushButton::clicked, this, &MainWindow::buttonClearConsoleClicked);
        }
    }

    for (auto *table: this->findChildren<QTableView *>()) {
        table->setAlternatingRowColors(true);
        if (table->objectName().endsWith("Graph")) {
            auto name = table->objectName().replace("table_", "").replace("_Graph", "");
            if (name.startsWith("Matrix")) {
                graphMatrixViews.append(table);
            } else if (name.startsWith("List")) {
                graphListViews.append(table);
                if (name.endsWith("Edges"))
                    table->setModel(new QStandardItemModel(0, 0));
            }

            for (auto *spinBox: spinBoxes) {
                if (spinBox->objectName().startsWith("spin_" + name) && spinBox->objectName().endsWith("NodesCount")) {
                    if (name.startsWith("Matrix")) {
                        connect(spinBox, &QSpinBox::valueChanged, this,
                                std::bind(&MainWindow::setNodesAmountMatrix, this, table, std::placeholders::_1));
                    }
                    graphCountSpins.insert(table->objectName(), spinBox);
                }
            }

            for (auto *button: pushButtons) {
                if (button->objectName().startsWith("button_" + name) && button->objectName().endsWith("_Apply")) {
                    if (name.startsWith("Matrix"))
                        connect(button, &QPushButton::clicked, this,
                                std::bind(&MainWindow::applyGraphMatrix, this, table));
                    else if (name == "ListEdges")
                        connect(button, &QPushButton::clicked, this,
                                std::bind(&MainWindow::applyEdgesList, this, table));
                }
            }
        }
        if (table->model() == nullptr)
            table->setModel(new QStandardItemModel(0, 0));
    }

    connect(ui->actionBandwidth, &QAction::triggered, this, [this](bool checked) {
        checked ? graph.setFlag(GraphFlags::ShowBandwidth) : graph.unsetFlag(GraphFlags::ShowBandwidth);
        graph.graphView->scene()->update();
    });

    connect(ui->actionWeights, &QAction::triggered, this, [this](bool checked) {
        checked ? graph.setFlag(GraphFlags::ShowWeights) : graph.unsetFlag(GraphFlags::ShowWeights);
        graph.graphView->scene()->update();
    });

    connect(ui->actionFlow, &QAction::triggered, this, [this](bool checked) {
        checked ? graph.setFlag(GraphFlags::ShowFlow) : graph.unsetFlag(GraphFlags::ShowFlow);
        graph.graphView->scene()->update();
    });

    auto view = ui->menuView_mode;
    auto docks = this->findChildren<QDockWidget *>();
    if (!docks.empty()) {
        auto dockStack = this->findChild<QDockWidget *>("dock_PlaceHolder");
        QDockWidget *dockTop = nullptr;
        for (auto *dock: docks) {
            if (dock != dockStack) {
                auto act = new QAction(dock->windowTitle());
                act->setCheckable(true);
                act->setChecked(true);
                this->tabifyDockWidget(dockStack, dock);
                if (!dockTop)
                    dockTop = dock;
                docksViewMode.insert(dock->windowTitle(), dock);
                connect(act, &QAction::triggered, this,
                        std::bind(&MainWindow::viewModeChecked, this, std::placeholders::_1));
                view->addAction(act);
            }
        }
        if (dockStack) {
            dockStack->hide();
            dockStack->setDisabled(true);
        }
        if (dockTop)
            dockTop->raise();
    }

    ui->centralwidget->layout()->removeWidget(ui->graphView);
    ui->centralwidget->layout()->addWidget(graph.graphView);
    connect(ui->actionCopy, &QAction::triggered, this, std::bind(&MainWindow::myCopy, this));
    connect(ui->actionPaste, &QAction::triggered, this, std::bind(&MainWindow::myPaste, this));
    connect(ui->actionRefreshTables, &QAction::triggered, this, std::bind(&MainWindow::updateTables, this));

    nodeMovementGroup = new QActionGroup(this);
    nodeMovementGroup->addAction(ui->actionAutomatic);
    nodeMovementGroup->addAction(ui->actionManual);
    connect(ui->actionManual, &QAction::triggered, this, [this](bool checked) {
        if (checked)
            graph.setFlag(GraphFlags::ManualMode);
        graph.graphView->scene()->update();
    });
    connect(ui->actionAutomatic, &QAction::triggered, this, [this](bool checked) {
        if (checked) {
            graph.unsetFlag(GraphFlags::ManualMode);
            graph.graphView->runTimer();
        }
    });
    ui->actionAutomatic->setChecked(true);
    saveState();
}

MainWindow::~MainWindow() {
    delete nodeMovementGroup;
    delete ui;
}

void MainWindow::saveToFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить граф", "", "Graph Files (*.gr);;All Files (*)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }

    QTextStream out(&file);
    unsigned int n = graph.getAmount();
    out << "Nodes: " << n << "\n\n";

    auto saveMatrix = [&](const QString &title, const Matrix2D &m) {
        out << title << ":\n";
        for (unsigned int i = 0; i < n; ++i) {
            for (unsigned int j = 0; j < n; ++j) {
                out << m[i][j] << (j == n - 1 ? "" : "\t");
            }
            out << "\n";
        }
        out << "\n";
    };

    saveMatrix("Adjacency Matrix", graph.getMatrixAdjacent());
    saveMatrix("Flow Matrix", graph.getMatrixFlow());
    saveMatrix("Bandwidth Matrix", graph.getMatrixBandwidth());

    file.close();
    ui->textEdit_Console->appendPlainText("Граф успешно сохранен в: " + fileName);
}

void MainWindow::saveState() {
    while (undoStack.size() > currentStateIndex + 1)
        undoStack.removeLast();
    GraphState s;
    s.amount = graph.getAmount();
    s.adj = graph.getMatrixAdjacent();
    s.flow = graph.getMatrixFlow();
    s.band = graph.getMatrixBandwidth();
    undoStack.append(s);
    currentStateIndex++;
}

void MainWindow::restoreState(const GraphState &state) {
    graph.resizeGraph(graph.getAmount(), state.amount);
    graph.setMatrixAdjacent(const_cast<Matrix2D &>(state.adj));
    graph.setMatrixFlow(const_cast<Matrix2D &>(state.flow));
    graph.setMatrixBandwidth(const_cast<Matrix2D &>(state.band));
    graph.graphView->initScene();
    updateTables();
}

void MainWindow::buttonClearConsoleClicked() { ui->textEdit_Console->clear(); }

void MainWindow::pasteClipboardToTable(QTableView *dest) {
    if (!dest->hasFocus())
        return;
    QString text = QApplication::clipboard()->text();
    QStandardItemModel *model = static_cast<QStandardItemModel *>(dest->model());
    QModelIndexList indexes = dest->selectionModel()->selectedIndexes();
    if (indexes.empty())
        return;
    int row = indexes.first().row();
    int col = indexes.first().column();
    auto rows = text.split('\n', Qt::SkipEmptyParts);
    for (int i = 0; i < rows.size() && (row + i) < model->rowCount(); ++i) {
        auto cols = rows[i].split('\t');
        for (int j = 0; j < cols.size() && (col + j) < model->columnCount(); ++j) {
            model->item(row + i, col + j)->setText(cols[j]);
        }
    }
}

void MainWindow::viewModeChecked(bool checked) {
    auto *dock = docksViewMode[qobject_cast<QAction *>(sender())->text()];
    dock->setVisible(checked);
}

void MainWindow::setNodesAmountMatrix(QTableView *table, int newAmount) {
    QStandardItemModel *model = static_cast<QStandardItemModel *>(table->model());
    int oldAmount = model->columnCount();
    if (newAmount < 0)
        newAmount = 0;
    if (oldAmount < newAmount) {
        int delta = newAmount - oldAmount;
        model->insertColumns(oldAmount, delta);
        model->insertRows(oldAmount, delta);
        for (int i = oldAmount; i < newAmount; ++i) {
            model->setHeaderData(i, Qt::Horizontal, i + 1);
            model->setHeaderData(i, Qt::Vertical, i + 1);
            table->setColumnWidth(i, 25);
            table->setRowHeight(i, 25);
            for (int j = 0; j < newAmount; ++j) {
                model->setItem(i, j, new QStandardItem("0"));
                if (i != j)
                    model->setItem(j, i, new QStandardItem("0"));
            }
        }
    } else {
        model->setColumnCount(newAmount);
        model->setRowCount(newAmount);
    }
}

void MainWindow::copyTableToClipboard(QTableView *src) {
    if (!src->hasFocus())
        return;
    QModelIndexList indexes = src->selectionModel()->selectedIndexes();
    if (indexes.empty())
        return;
    std::sort(indexes.begin(), indexes.end());
    QString text;
    for (int i = 0; i < indexes.count(); ++i) {
        text.append(src->model()->data(indexes[i]).toString());
        if (i + 1 < indexes.count()) {
            text.append(indexes[i + 1].row() != indexes[i].row() ? "\n" : "\t");
        }
    }
    QApplication::clipboard()->setText(text);
}

void MainWindow::applyGraphMatrix(QTableView *table) {
    saveState();
    QStandardItemModel *model = static_cast<QStandardItemModel *>(table->model());
    int n = model->rowCount();
    Matrix2D m(n, QList<double>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            m[i][j] = model->item(i, j)->text().toDouble();
    }
    if (table->objectName().contains("Adj"))
        graph.setMatrixAdjacent(m);
    else if (table->objectName().contains("Flow"))
        graph.setMatrixFlow(m);
    else if (table->objectName().contains("Bandwidth"))
        graph.setMatrixBandwidth(m);
    graph.graphView->initScene();
    updateTables();
}

void MainWindow::applyEdgesList(QTableView *table) {
    saveState();
    auto model = static_cast<QStandardItemModel *>(table->model());
    for (int i = 0; i < model->rowCount(); ++i) {
        int u = model->item(i, 0)->text().toInt();
        int v = model->item(i, 1)->text().toInt();
        double w = model->item(i, 2)->text().toDouble();
        if (w > 0) {
            graph.setEdgeWeight(u, v, w);
            graph.setEdgeBandwidth(u, v, model->item(i, 3)->text().toDouble());
            graph.setEdgeFlow(u, v, model->item(i, 4)->text().toDouble());
        } else
            graph.removeEdge(u, v);
    }
    updateTables();
}

void MainWindow::updateEdgesList(QTableView *list) {
    auto edges = graph.getListEdges();
    auto model = static_cast<QStandardItemModel *>(list->model());
    model->setRowCount(edges.size());
    model->setColumnCount(5);
    QStringList headers = {"U", "V", "Weight", "Band", "Flow"};
    model->setHorizontalHeaderLabels(headers);
    for (int i = 0; i < edges.size(); i++) {
        for (int j = 0; j < 5; j++)
            model->setItem(i, j, new QStandardItem(edges[i][j].toString()));
    }
}

void MainWindow::updateTables() {
    unsigned int n = graph.getAmount();
    for (auto &spin: graphCountSpins)
        spin->setValue(n);
    for (auto &table: graphMatrixViews) {
        QString name = table->objectName();
        if (name.contains("Adj"))
            setTableFromMatrix(table, graph.getMatrixAdjacent(), n, n);
        else if (name.contains("Flow"))
            setTableFromMatrix(table, graph.getMatrixFlow(), n, n);
        else if (name.contains("Bandwidth"))
            setTableFromMatrix(table, graph.getMatrixBandwidth(), n, n);
    }
    for (auto &list: graphListViews)
        updateEdgesList(list);
}

void MainWindow::myCopy() {
    if (auto *t = qobject_cast<QTableView *>(focusWidget()))
        copyTableToClipboard(t);
}
void MainWindow::myPaste() {
    if (auto *t = qobject_cast<QTableView *>(focusWidget()))
        pasteClipboardToTable(t);
}

template<typename T>
void MainWindow::setTableFromMatrix(QTableView *table, T &matrix, int h, int w) {
    auto *model = static_cast<QStandardItemModel *>(table->model());
    model->setRowCount(h);
    model->setColumnCount(w);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (!model->item(i, j))
                model->setItem(i, j, new QStandardItem());
            model->item(i, j)->setText(QString::number(matrix[i][j]));
        }
    }
}
