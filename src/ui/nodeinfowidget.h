#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTimer>
#include <memory>

class Node;
class QButtonGroup;
class IMaintenanceStrategy;

class NodeInfoWidget : public QWidget {
    Q_OBJECT
public:
    explicit NodeInfoWidget(QWidget *parent = nullptr);
    void setNode(Node *node);

private slots:
    void onStrategyToggled(bool checked);
    void refreshView();

private:
    Node *currentNode = nullptr;
    QLabel *idValue = nullptr;
    QLabel *stateValue = nullptr;
    QLabel *reliabilityValue = nullptr;
    QLabel *summaryValue = nullptr;
    QPlainTextEdit *edgeListValue = nullptr;
    QPlainTextEdit *inboxValue = nullptr;

    QCheckBox *strategyBasicCheck = nullptr;
    QCheckBox *strategyNoControlCheck = nullptr;
    QCheckBox *strategyOfflineCheck = nullptr;
    QButtonGroup *strategyGroup = nullptr;
    QTimer *refreshTimer = nullptr;

    bool guardStrategyUpdate = false;

    void clearView();
    void syncStrategyChecks();
    std::unique_ptr<IMaintenanceStrategy> createStrategyFor(QObject *checkBox) const;
};
