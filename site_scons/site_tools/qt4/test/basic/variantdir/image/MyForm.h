#include "ui_anUiFile.h"

#include <QWidget>

class MyForm : public QWidget
{
     Q_OBJECT

 public:
     MyForm(QWidget *parent = 0);

 public slots:
     void testSlot();

 private:
     Ui::Form ui;
};

