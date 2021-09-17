#include <vision/interpreter.hpp>
#include <vision/server.hpp>

#include <fstream>
#include <iostream>

#include <stdlib.h>

namespace {

class RenderFunction final : public vision::Function
{
public:
  auto Call(const std::vector<const vision::Value*>& args)
    -> std::unique_ptr<vision::Value>
  {
    if (args.size() != 2)
      return nullptr;

    (void)args;

    return nullptr;
  }
};

} // namespace

int
main(int argc, char** argv)
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <file.txt>" << std::endl;
    return EXIT_FAILURE;
  }

  std::ifstream file(argv[1]);

  if (!file.good()) {
    std::cerr << "Failed to open '" << argv[1] << "'." << std::endl;
    return EXIT_FAILURE;
  }

  vision::Server server;

  server.DefineFunction("render", new RenderFunction());

  std::string line;

  while (std::getline(file, line)) {
    server.Process(line.data(), line.size());
  }

  return EXIT_SUCCESS;
}
