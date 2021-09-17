#include <vision/server.hpp>

#include <stdlib.h>

int
main()
{
  vision::Server server;

  if (!server.CanRun())
    return EXIT_FAILURE;

  server.Run();

  return EXIT_SUCCESS;
}
