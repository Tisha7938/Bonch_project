#include "nodeinfowidget.h"
#include "node.h"

NodeInfoWidget::NodeInfoWidget(QWidget *parent) : QWidget(parent) {
    setObjectName("NodeInfoWidget");

    setStyleSheet(
            "QLineEdit { border: 2px solid black; padding: 4px; font-size: 12px; }"
            "QLabel { font-weight: bold; font-size: 14px; border: none; }"
            "QPushButton { border: 2px solid black; background: #f0f0f0; padding: 8px; font-weight: bold; }"
            "QPushButton:hover { background: #e0e0e0; }"
            );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 20, 15, 20);
    mainLayout->setSpacing(20);

    titleLabel = new QLabel("Редактировать значения", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("border: 2px solid black; padding: 5px;");
    mainLayout->addWidget(titleLabel);

    QFormLayout *form = new QFormLayout();
    form->setSpacing(15);

    editTest1 = new QLineEdit(this);
    editTest1->setPlaceholderText("сюда вводить чето");
    editTest2 = new QLineEdit(this);
    editTest3 = new QLineEdit(this);

    form->addRow("test 1:", editTest1);
    form->addRow("test 2:", editTest2);
    form->addRow("test..:", editTest3);

    mainLayout->addLayout(form);

    applyButton = new QPushButton("Apply Changes", this);
    connect(applyButton, &QPushButton::clicked, this, &NodeInfoWidget::onApplyClicked);
    mainLayout->addWidget(applyButton);

    mainLayout->addStretch();
}

void NodeInfoWidget::setNode(Node *node) {
    currentNode = node;
    if (currentNode) {
        editTest1->setText(currentNode->nodeData.test1);
        editTest2->setText(currentNode->nodeData.test2);
        editTest3->setText(currentNode->nodeData.test3);
    } else {
        editTest1->clear();
        editTest2->clear();
        editTest3->clear();
    }
}

void NodeInfoWidget::onApplyClicked() {
    if (currentNode) {
        currentNode->nodeData.test1 = editTest1->text();
        currentNode->nodeData.test2 = editTest2->text();
        currentNode->nodeData.test3 = editTest3->text();
    }
}