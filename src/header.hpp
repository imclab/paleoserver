//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef PALEOSERVER_HEADER_HPP
#define PALEOSERVER_HEADER_HPP

#include <string>

namespace http {
namespace paleoserver {

struct header
{
  std::string name;
  std::string value;
};

} // namespace paleoserver
} // namespace http

#endif // PALEOSERVER_HEADER_HPP
