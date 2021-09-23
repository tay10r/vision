#include "auto_complete_engine.hpp"

#include <QCompleter>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QUrl>

#include <map>

namespace vision::gui {

class AutoCompleteEngineImpl final
{
  friend AutoCompleteEngine;

  using ConstModelIterator =
    std::map<QString, QAbstractItemModel*>::const_iterator;

  QStringList m_scheme_list;

  QStringListModel m_scheme_model;

  std::map<QString, QAbstractItemModel*> m_model_map;

  QCompleter m_completer;

  AutoCompleteEngineImpl(QObject* parent)
    : m_scheme_model(parent)
    , m_completer(parent)
  {}

  QAbstractItemModel* GetItemModel(const QUrl&);

  static QChar At(const QString&, int);

  QString RemoveScheme(const QString& url_string, const QString& scheme);

  QString FindCurrentScheme();
};

AutoCompleteEngineImpl&
AutoCompleteEngine::GetImpl()
{
  if (!m_impl)
    m_impl = new AutoCompleteEngineImpl(this);

  return *m_impl;
}

AutoCompleteEngine::~AutoCompleteEngine()
{
  delete m_impl;
}

QCompleter*
AutoCompleteEngine::GetCompleter()
{
  return &GetImpl().m_completer;
}

void
AutoCompleteEngine::AddModel(const QString& scheme, QAbstractItemModel* model)
{
  auto& impl = GetImpl();

  impl.m_scheme_list << (scheme + "://");

  impl.m_scheme_list.sort();

  impl.m_scheme_model.setStringList(impl.m_scheme_list);

  impl.m_model_map.emplace(scheme, model);
}

void
AutoCompleteEngine::Execute(const QString& url_string)
{
  auto& impl = GetImpl();

  const QUrl url(url_string);

  QAbstractItemModel* model = impl.GetItemModel(url);

  if (!model) {

    impl.m_completer.setModel(&impl.m_scheme_model);

    impl.m_completer.setCompletionPrefix(url_string);

  } else {

    impl.m_completer.setModel(model);

    const QString suffix = impl.RemoveScheme(url_string, url.scheme());

    impl.m_completer.setCompletionPrefix(suffix);
  }
}

QStringList
AutoCompleteEngine::GetResults()
{
  auto& impl = GetImpl();

  const QString url_scheme = impl.FindCurrentScheme();

  const QAbstractItemModel* completion_model =
    impl.m_completer.completionModel();

  const int result_count = completion_model->rowCount(QModelIndex());

  const bool is_scheme_model = impl.m_completer.model() == &impl.m_scheme_model;

  QStringList results;

  for (int i = 0; i < result_count; i++) {

    const QModelIndex index = completion_model->index(i, 0);

    const QString result = completion_model->data(index).toString();

    if (is_scheme_model)
      results << result;
    else
      results << url_scheme + "://" + result;
  }

  return results;
}

QString
AutoCompleteEngineImpl::FindCurrentScheme()
{
  QAbstractItemModel* current_model = m_completer.model();
  if (current_model == &m_scheme_model)
    return QString();

  for (const auto& entry : m_model_map) {
    if (entry.second == current_model)
      return entry.first;
  }

  return QString();
}

QAbstractItemModel*
AutoCompleteEngineImpl::GetItemModel(const QUrl& url)
{
  const QString scheme = url.scheme();

  const ConstModelIterator it = m_model_map.find(scheme);

  if (it == m_model_map.end())
    return nullptr;
  else
    return it->second;
}

QChar
AutoCompleteEngineImpl::At(const QString& str, int i)
{
  if ((i >= 0) && (i < str.size()))
    return str[i];
  else
    return QChar(0);
}

QString
AutoCompleteEngineImpl::RemoveScheme(const QString& url_string,
                                     const QString& scheme)
{
  if (!url_string.startsWith(scheme))
    return QString();

  using ulong = unsigned long;

  if ((scheme.size() + (sizeof("://") - 1)) >= ulong(url_string.size()))
    return QString();

  if ((At(url_string, scheme.size() + 0) != ':') ||
      (At(url_string, scheme.size() + 1) != '/') ||
      (At(url_string, scheme.size() + 2) != '/'))
    return QString();

  const int clip_length = scheme.size() + sizeof("://") - 1;

  return QString(url_string.data() + clip_length,
                 url_string.size() - clip_length);
}

} // namespace vision::gui
