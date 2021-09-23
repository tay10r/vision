#pragma once

#include <QObject>

class QAbstractItemModel;
class QString;
class QCompleter;

namespace vision::gui {

class AutoCompleteEngineImpl;

class AutoCompleteEngine final : public QObject
{
public:
  AutoCompleteEngine(QObject* parent)
    : QObject(parent)
  {}

  AutoCompleteEngine(const AutoCompleteEngine&) = delete;

  AutoCompleteEngine(AutoCompleteEngine&&) = delete;

  ~AutoCompleteEngine();

  void AddModel(const QString&, QAbstractItemModel*);

  void Execute(const QString& url_string);

  QCompleter* GetCompleter();

  QStringList GetResults();

private:
  AutoCompleteEngineImpl& GetImpl();

  AutoCompleteEngineImpl* m_impl = nullptr;
};

} // namespace vision::gui
