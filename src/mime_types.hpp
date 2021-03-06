//
// mime_types.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef PALEOSERVER_MIME_TYPES_HPP
#define PALEOSERVER_MIME_TYPES_HPP

#include <string>

namespace http {
namespace paleoserver {
namespace mime_types {

/// Convert a file extension into a MIME type.
std::string extension_to_type(const std::string& extension);
std::string type_to_extension(std::string mime_type);

} // namespace mime_types
} // namespace paleoserver
} // namespace http

#endif // PALEOSERVER_MIME_TYPES_HPP
