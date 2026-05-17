#include "nodeinfowidget.h"
#include "node.h"
#include "edge.h"
#include "strategybasiccontrol.h"
#include "strategynocontrolcheck.h"
#include "strategyofflinecheck.h"

#include <QButtonGroup>
#include <QFormLayout>
#include <QFrame>
#include <QVBoxLayout>

NodeInfoWidget::NodeInfoWidget(QWidget *parent) : QWidget(parent) {
    setObjectName("NodeInfoWidget");

    setStyleSheet(
            "#NodeInfoWidget { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f8fafc, stop:1 #eef2f7); }"
            "QLabel { font-size: 13px; border: none; }"
            "QLabel#titleLabel { font-size: 15px; font-weight: 700; }"
            "QLabel#stateBadge { border-radius: 10px; padding: 2px 10px; font-weight: 600; }"
            "QPlainTextEdit { border: 1px; border-radius: 8px; padding: 6px; "
            "font-family: Consolas, 'Courier New', monospace; font-size: 12px; }"
            "QCheckBox { font-size: 13px; spacing: 8px; padding: 4px; }"
            "QCheckBox::indicator { width: 15px; height: 15px; }"
            "QCheckBox::indicator:unchecked { border: 1px; border-radius: 4px; background: #fff; }"
            "QCheckBox::indicator:checked { border: 1px; border-radius: 4px; background: #2563eb; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    auto *titleLabel = new QLabel("Node Inspector", this);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);

    summaryValue = new QLabel("Выберите вершину на графе", this);
    summaryValue->setStyleSheet("color: #475569; font-size: 12px;");
    mainLayout->addWidget(summaryValue);

    auto *divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #cbd5e1;");
    mainLayout->addWidget(divider);

    auto *form = new QFormLayout();
    form->setSpacing(8);

    idValue = new QLabel("-", this);
    stateValue = new QLabel("-", this);
    stateValue->setObjectName("stateBadge");
    reliabilityValue = new QLabel("-", this);
    edgeListValue = new QPlainTextEdit(this);
    inboxValue = new QPlainTextEdit(this);
    edgeListValue->setReadOnly(true);
    inboxValue->setReadOnly(true);
    edgeListValue->setMaximumHeight(90);
    inboxValue->setMaximumHeight(130);

    form->addRow("id:", idValue);
    form->addRow("state:", stateValue);
    form->addRow("reliability:", reliabilityValue);
    form->addRow("edgeList:", edgeListValue);
    form->addRow("inbox:", inboxValue);

    mainLayout->addLayout(form);

    auto *strategyLabel = new QLabel("Strategy (single select):", this);
    strategyLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(strategyLabel);

    strategyBasicCheck = new QCheckBox("BasicControl", this);
    strategyNoControlCheck = new QCheckBox("NoControlCheck", this);
    strategyOfflineCheck = new QCheckBox("OfflineCheck", this);

    strategyGroup = new QButtonGroup(this);
    strategyGroup->setExclusive(false);
    strategyGroup->addButton(strategyBasicCheck);
    strategyGroup->addButton(strategyNoControlCheck);
    strategyGroup->addButton(strategyOfflineCheck);

    connect(strategyBasicCheck, &QCheckBox::toggled, this, &NodeInfoWidget::onStrategyToggled);
    connect(strategyNoControlCheck, &QCheckBox::toggled, this, &NodeInfoWidget::onStrategyToggled);
    connect(strategyOfflineCheck, &QCheckBox::toggled, this, &NodeInfoWidget::onStrategyToggled);

    mainLayout->addWidget(strategyBasicCheck);
    mainLayout->addWidget(strategyNoControlCheck);
    mainLayout->addWidget(strategyOfflineCheck);
    mainLayout->addStretch();

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(250);
    connect(refreshTimer, &QTimer::timeout, this, &NodeInfoWidget::refreshView);
    refreshTimer->start();

    clearView();
}

void NodeInfoWidget::setNode(Node *node) {
    currentNode = node;
    refreshView();
}

void NodeInfoWidget::onStrategyToggled(bool checked) {
    if (guardStrategyUpdate || !currentNode || !currentNode->model()) {
        return;
    }

    auto *senderCheck = qobject_cast<QCheckBox *>(sender());
    if (!senderCheck) {
        return;
    }

    guardStrategyUpdate = true;
    if (checked) {
        for (auto *button: strategyGroup->buttons()) {
            if (button != senderCheck) {
                button->setChecked(false);
            }
        }
        currentNode->model()->setStrategy(createStrategyFor(senderCheck));
    } else {
        bool anyChecked = false;
        for (auto *button: strategyGroup->buttons()) {
            if (button->isChecked()) {
                anyChecked = true;
                break;
            }
        }
        if (!anyChecked) {
            senderCheck->setChecked(true);
        }
    }
    guardStrategyUpdate = false;
}

void NodeInfoWidget::refreshView() {
    if (!currentNode || !currentNode->model()) {
        clearView();
        return;
    }

    auto *model = currentNode->model();
    idValue->setText(QString::number(model->id()));

    switch (model->state()) {
        case NodeModel::State::Operational:
            stateValue->setText("Operational");
            stateValue->setStyleSheet("background: #dcfce7; color: #166534; border-radius: 10px; padding: 2px 10px; font-weight: 600;");
            break;
        case NodeModel::State::Failed:
            stateValue->setText("Failed");
            stateValue->setStyleSheet("background: #fee2e2; color: #991b1b; border-radius: 10px; padding: 2px 10px; font-weight: 600;");
            break;
        case NodeModel::State::Maintenance:
            stateValue->setText("Maintenance");
            stateValue->setStyleSheet("background: #fef3c7; color: #92400e; border-radius: 10px; padding: 2px 10px; font-weight: 600;");
            break;
    }

    reliabilityValue->setText(QString::number(model->reliability(), 'f', 4));

    QStringList edges;
    for (auto *edge: currentNode->edges()) {
        edges << QString("%1 -> %2").arg(edge->sourceNode()->getIndex()).arg(edge->destNode()->getIndex());
    }
    edgeListValue->setPlainText(edges.isEmpty() ? "-" : edges.join('\n'));

    QStringList inboxMessages;
    const auto &inbox = model->inbox();
    inboxMessages.reserve(static_cast<int>(inbox.size()));
    for (const auto &message: inbox) {
        inboxMessages << QString::fromStdString(message);
    }
    inboxValue->setPlainText(inboxMessages.isEmpty() ? "-" : inboxMessages.join('\n'));

    summaryValue->setText(QString("Node #%1 | edges: %2 | inbox: %3")
                                  .arg(model->id())
                                  .arg(edges.size())
                                  .arg(inboxMessages.size()));

    syncStrategyChecks();
}

void NodeInfoWidget::clearView() {
    idValue->setText("-");
    stateValue->setText("-");
    stateValue->setStyleSheet("background: #e2e8f0; color: #475569; border-radius: 10px; padding: 2px 10px; font-weight: 600;");
    reliabilityValue->setText("-");
    summaryValue->setText("Выберите вершину на графе");
    edgeListValue->setPlainText("-");
    inboxValue->setPlainText("-");

    guardStrategyUpdate = true;
    strategyBasicCheck->setChecked(false);
    strategyNoControlCheck->setChecked(false);
    strategyOfflineCheck->setChecked(false);
    strategyBasicCheck->setEnabled(false);
    strategyNoControlCheck->setEnabled(false);
    strategyOfflineCheck->setEnabled(false);
    guardStrategyUpdate = false;
}

void NodeInfoWidget::syncStrategyChecks() {
    auto *model = currentNode ? currentNode->model() : nullptr;
    const auto *strategy = model ? model->strategy() : nullptr;

    const bool enabled = model != nullptr;
    strategyBasicCheck->setEnabled(enabled);
    strategyNoControlCheck->setEnabled(enabled);
    strategyOfflineCheck->setEnabled(enabled);

    guardStrategyUpdate = true;
    strategyBasicCheck->setChecked(dynamic_cast<const StrategyBasicControl *>(strategy) != nullptr);
    strategyNoControlCheck->setChecked(dynamic_cast<const StrategyNoControlCheck *>(strategy) != nullptr);
    strategyOfflineCheck->setChecked(dynamic_cast<const StrategyOfflineCheck *>(strategy) != nullptr);
    guardStrategyUpdate = false;
}

std::unique_ptr<IMaintenanceStrategy> NodeInfoWidget::createStrategyFor(QObject *checkBox) const {
    if (checkBox == strategyBasicCheck) {
        return std::make_unique<StrategyBasicControl>();
    }
    if (checkBox == strategyNoControlCheck) {
        return std::make_unique<StrategyNoControlCheck>();
    }
    return std::make_unique<StrategyOfflineCheck>();
}
