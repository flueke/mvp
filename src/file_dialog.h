#ifndef UUID_cef78ddc_743e_41d9_9de9_81155960d35c
#define UUID_cef78ddc_743e_41d9_9de9_81155960d35c

#include <QFileDialog>

namespace mvp
{

class FileDialog: public QFileDialog
{
  Q_OBJECT

  public:
    FileDialog(QWidget *parent = nullptr);

  private:
};

} // ns mvp

#endif
