#include <QAction>
#include <QApplication>
#include <QLocale>
#include <QMenu>
#include <QTextBrowser>
#include <mesytec-mvlc/util/logging.h>
#include <spdlog/sinks/qt_sinks.h>

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
  if (app.arguments().contains("--debug"))
    mesytec::mvlc::set_global_log_level(spdlog::level::debug);
  if (app.arguments().contains("--trace"))
    mesytec::mvlc::set_global_log_level(spdlog::level::trace);

  mvp::MVPLabGui gui;

  #if 1
  // This way of setting up the qt sink for loggers actually does work. Care has
  // to be taken with the push_back() of the the sink: it's not thread-safe!
  //const auto MaxLines = 1u << 20;
  //auto sink = std::make_shared<spdlog::sinks::qt_color_sink_mt>(gui.getLogview(), MaxLines);
  auto sink = std::make_shared<spdlog::sinks::qt_sink_mt>(gui.getLogview(), "append");
  sink->set_pattern("%H:%M:%S: %v");
  sink->set_level(spdlog::level::info);
  const auto LoggerNames = { "mvlc", "mvlc_mvp_lib" };

  if (auto logger = mesytec::mvlc::default_logger())
    logger->sinks().push_back(sink);

  for (const auto &ln: LoggerNames)
    if (auto logger = mesytec::mvlc::get_logger(ln))
      logger->sinks().push_back(sink);
  #endif

  // Hide the "advanced" widget and the view menu. This is the only thing making
  // 'mvp' different from 'mvplab'.
  gui.findChild<QAction *>("actionShowAdvanced")->setChecked(false);
  gui.findChild<QMenu *>("menu_View")->deleteLater();
  gui.show();

  return app.exec();
}
