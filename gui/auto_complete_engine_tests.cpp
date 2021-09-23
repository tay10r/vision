#include <gtest/gtest.h>

#include "auto_complete_engine.hpp"

#include <QStringList>
#include <QStringListModel>

#include <sstream>

namespace {

std::string
TestAutoComplete(const QString& input);

} // namespace

TEST(AddressBar, AutoComplete)
{
  std::string result = TestAutoComplete("");

  EXPECT_EQ(result,
            "debug://\n"
            "file://\n"
            "tcp://\n");
}

TEST(AddressBar, AutoCompleteTcp)
{
  std::string result = TestAutoComplete("t");

  EXPECT_EQ(result, "tcp://\n");
}

TEST(AddressBar, AutoCompleteTcpItem)
{
  std::string result = TestAutoComplete("tcp://localhost");

  EXPECT_EQ(result,
            "tcp://localhost1\n"
            "tcp://localhost2\n"
            "tcp://localhost3\n");
}

TEST(AddressBar, AutoCompleteFile)
{
  std::string result = TestAutoComplete("f");

  EXPECT_EQ(result, "file://\n");
}

TEST(AddressBar, AutoCompleteDebugItem)
{
  std::string result = TestAutoComplete("debug://");

  EXPECT_EQ(result,
            "debug://bug1\n"
            "debug://bug2\n"
            "debug://bug22\n");
}

TEST(AddressBar, AutoCompleteDebugItem2)
{
  std::string result = TestAutoComplete("debug://bug2");

  EXPECT_EQ(result,
            "debug://bug2\n"
            "debug://bug22\n");
}

namespace {

std::string
TestAutoComplete(const QString& input)
{
  QStringList tcp_items;
  tcp_items << "localhost1";
  tcp_items << "localhost2";
  tcp_items << "localhost3";

  QStringList fs_items;
  fs_items << "file1";
  fs_items << "file2";
  fs_items << "file3";

  QStringList debug_items;
  debug_items << "bug1";
  debug_items << "bug2";
  debug_items << "bug22";

  QStringListModel tcp_item_model(tcp_items);

  QStringListModel fs_item_model(fs_items);

  QStringListModel debug_item_model(debug_items);

  vision::gui::AutoCompleteEngine engine(nullptr);
  engine.AddModel("tcp", &tcp_item_model);
  engine.AddModel("file", &fs_item_model);
  engine.AddModel("debug", &debug_item_model);

  engine.Execute(input);

  QStringList results = engine.GetResults();

  std::ostringstream output_stream;

  for (const auto& result : results)
    output_stream << result.toStdString() << '\n';

  return output_stream.str();
}

} // namespace
