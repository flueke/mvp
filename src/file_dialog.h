#ifndef UUID_cef78ddc_743e_41d9_9de9_81155960d35c
#define UUID_cef78ddc_743e_41d9_9de9_81155960d35c

#include <QFileDialog>

namespace mesytec
{
namespace mvp
{

class FileDialog: public QFileDialog
{
  Q_OBJECT

  public:
    FileDialog(QWidget *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) override;

    QString get_selected_file_or_dir() const
    { return selectedFiles().value(0, QString()); }

  private slots:
    void handle_open_button_clicked();

  private:
    QPushButton *get_open_button() const;
};

} // ns mvp
} // ns mesytec

#endif
