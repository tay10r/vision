#pragma once

#ifndef VISION_SERVER_OBSERVER_H
#define VISION_SERVER_OBSERVER_H

namespace vision {

class ServerObserver
{
public:
  virtual ~ServerObserver() = default;

  virtual void OnAddressParseFailure(const char* addr) = 0;

  virtual void OnStart() = 0;

  virtual void OnShutdown() = 0;

  virtual void OnConnection() = 0;
};

} // namespace vision

#endif // VISION_SERVER_OBSERVER_H
