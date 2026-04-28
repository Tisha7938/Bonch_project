#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>

class Node;

class NodeInfoWidget : public QWidget {
    Q_OBJECT
public:
    explicit NodeInfoWidget(QWidget *parent = nullptr);
    void setNode(Node *node);

private slots:
    void onApplyClicked();

private:
    Node *currentNode = nullptr;
    QLineEdit *editTest1;
    QLineEdit *editTest2;
    QLineEdit *editTest3;
    QLabel *titleLabel;
    QPushButton *applyButton;
};