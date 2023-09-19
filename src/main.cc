#include <QAction>
#include <QApplication>
#include <QLocale>
#include <mesytec-mvlc/util/logging.h>

#include "gui.h"
#include "git_version.h"

using namespace mesytec;

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(libmvp_resources);
  using mvp::PortInfoList;
  qRegisterMetaType<PortInfoList>("PortInfoList");

  // force C locale to avoid issues with number separator characters
  QLocale::setDefault(QLocale::c());

  QApplication app(argc, argv);

  app.setOrganizationName("mesytec");
  app.setOrganizationDomain("mesytec.com");
  app.setApplicationName("mvp");
  app.setApplicationDisplayName(QSL("mvp - Mesytec (VME) Programmer"));
  app.setApplicationVersion(mvp::mvp_version());

  mesytec::mvlc::set_global_log_level(spdlog::level::info);

  mvp::MVPLabGui gui;
  // Hide the "advanced" widget
  gui.getToggleAdvancedAction()->setChecked(false);
  gui.show();

  return app.exec();
}
