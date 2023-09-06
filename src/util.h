#ifndef UUID_f77de5f7_899c_4f3e_925e_8818d474b790
#define UUID_f77de5f7_899c_4f3e_925e_8818d474b790

#include <QException>
#include <QFuture>
#include <QtConcurrent>
#include <QtDebug>
#include <QSerialPort>
#include <QTextStream>
#include <QObject>

#include <exception>
#include <functional>
#include <system_error>

#include <gsl/gsl-lite.hpp>

#define QSL(str) QStringLiteral(str)

class QThread;

namespace mesytec
{
namespace mvp
{

/* On construction moves the given object to the given target thread. On
 * desctruction the object is moved back to its original thread. */
class ThreadMover
{
  public:
    ThreadMover(gsl::not_null<QObject *> object, QThread *target_thread)
      : m_object(object)
      , m_thread(object->thread())
    {
      qDebug() << this << "moving" << object.get() << "to target thread" << target_thread;

      m_object->moveToThread(target_thread);

      if (m_object->thread() != target_thread)
        throw std::runtime_error("initial thread move failed");
    }

    ~ThreadMover()
    {
      qDebug() << this << "moving" << m_object << "to original thread" << m_thread;
      m_object->moveToThread(m_thread);
    }

  private:
      Q_DISABLE_COPY(ThreadMover);
      QObject *m_object = 0;
      QThread *m_thread = 0;
};

class QtExceptionPtr: public QException
{
  public:
    explicit QtExceptionPtr(const std::exception_ptr &ptr)
      : m_ptr(ptr)
    {}

    std::exception_ptr get() const { return m_ptr; }
    void raise() const override { std::rethrow_exception(m_ptr); }
    QtExceptionPtr *clone() const override { return new QtExceptionPtr(*this); }

  private:
    std::exception_ptr m_ptr;
};

// Note: requires specifying the return type T at the point of use
template <typename T, typename Func>
QFuture<T> run_in_thread(Func func, QObject *thread_dependent_obj)
{
  return QtConcurrent::run([=] {
      try {
        ThreadMover tm(thread_dependent_obj, QThread::currentThread());
        qDebug() << "run_in_thread: calling functor";
        return func();
      } catch (...) {
        qDebug() << "run_in_thread: exception caught";
        throw QtExceptionPtr(std::current_exception());
      }
    });
}

template <typename T, typename Func>
T run_in_thread_wait_in_loop(Func func, QObject *thread_dependent_obj,
    QFutureWatcher<void> &fw)
{
  QEventLoop loop;

  auto con = QObject::connect(&fw, &QFutureWatcher<void>::finished, [&]() {
      qDebug() << "exiting local event loop";
      loop.quit();
    });

  ThreadMover tm(thread_dependent_obj, 0);

  auto f = run_in_thread<T>(func, thread_dependent_obj);
  fw.setFuture(f);

  loop.exec();

  QObject::disconnect(con);

  return f.result();
}

template <typename Func>
void run_in_thread_wait_in_loop(Func func, QObject *thread_dependent_obj,
    QFutureWatcher<void> &fw)
{
  QEventLoop loop;

  auto con = QObject::connect(&fw, &QFutureWatcher<void>::finished, [&]() {
      qDebug() << "exiting local event loop";
      loop.quit();
    });

  ThreadMover tm(thread_dependent_obj, 0);

  auto f = run_in_thread<void>(func, thread_dependent_obj);
  fw.setFuture(f);

  loop.exec();

  QObject::disconnect(con);

  f.waitForFinished();
}

const QMap<QSerialPort::SerialPortError, QString> port_error_to_string_data = {
  { QSerialPort::NoError                    ,"NoError"              },
  { QSerialPort::DeviceNotFoundError        ,"DeviceNotFoundError"  },
  { QSerialPort::PermissionError            ,"PermissionError"      },
  { QSerialPort::OpenError                  ,"OpenError"            },
  { QSerialPort::NotOpenError               ,"NotOpenError"         },
  { QSerialPort::ParityError                ,"ParityError"          },
  { QSerialPort::FramingError               ,"FramingError"         },
  { QSerialPort::BreakConditionError        ,"BreakConditionError"  },
  { QSerialPort::WriteError                 ,"WriteError"           },
  { QSerialPort::ReadError                  ,"ReadError"            },
  { QSerialPort::ResourceError              ,"ResourceError"        },
  { QSerialPort::UnsupportedOperationError  ,"UnsupportedOperationError" },
  { QSerialPort::TimeoutError               ,"TimeoutError"         },
  { QSerialPort::UnknownError               ,"UnknownError"         }
};

inline QString port_error_to_string(const QSerialPort::SerialPortError &e)
{
  return port_error_to_string_data.value(e, QString::number(e));
}

class ComError: public std::runtime_error
{
  public:
    ComError(const QString &msg, QSerialPort::SerialPortError ev=QSerialPort::NoError)
      : std::runtime_error("Com Error")
      , error_value(ev)
    {
      message = QString("Com Error: %1 (%2)")
          .arg(msg)
          .arg(port_error_to_string(ev))
          .toLatin1();
    }

    virtual const char * what() const noexcept override
    { return message.constData(); }

  QSerialPort::SerialPortError error_value;
  QByteArray message;
};

ComError make_com_error(
    gsl::not_null<QIODevice *> device,
    bool clear_serial_port_error=true);

template <typename T>
QVector<T> span_to_qvector(const gsl::span<T> &span_)
{
  QVector<T> ret;
  std::copy(span_.begin(), span_.end(), std::back_inserter(ret));
  return ret;
}

} // ns mvp
} // ns mesytec

template <typename U>
QString format_bytes(const U &bytes)
{
  QString ret;
  QTextStream stream(&ret);
  format_bytes(stream, bytes);
  return ret;
}

template <typename T, typename U>
T &format_bytes(T &stream, const U &bytes)
{
  stream << qSetFieldWidth(2) << qSetPadChar('0') << Qt::hex;

  size_t i=0;

  for (uchar c: bytes) {
    stream << qSetFieldWidth(2) << qSetPadChar('0') << Qt::hex;
    stream << c;
    stream << Qt::reset << " ";

    if ((++i % 16) == 0)
      stream << Qt::endl;
  }
  return stream;
}

#endif
