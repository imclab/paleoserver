//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"


#include <iostream>

#include <mapnik/version.hpp>
#include <mapnik/map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>


#if MAPNIK_VERSION >= 800
   #include <mapnik/box2d.hpp>
   #define Image32 image_32
   #define ImageData32 image_data_32
   #define Envelope box2d
#else
   #include <mapnik/envelope.hpp>
   #define zoom_to_box zoomToBox
   #define image_32 Image32
   #define image_data_32 ImageData32
   #define box2d Envelope
#endif

#include "uri_parser.hpp"

// parsing
// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>


using namespace mapnik;

namespace http {
namespace paleoserver {

#if MAP_PER_IO
request_handler::request_handler(const std::string& doc_root)
  : doc_root_(doc_root) {}
#else
request_handler::request_handler(const std::string& doc_root)
  : doc_root_(doc_root),
    map_() {}
#endif

#if MAP_PER_IO
#else
void request_handler::set_map(mapnik::Map mapnik_map)
{
    map_ = mapnik_map;
}
#endif

#if MAP_PER_IO
void request_handler::handle_request(const request& req, reply& rep, mapnik::Map map_)
#else
void request_handler::handle_request(const request& req, reply& rep)
#endif
{
  // Decode url to path.
  std::string request_path;
  if (!url_decode(req.uri, request_path))
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }
  
  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos
      || request_path == "/favicon.ico")
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  if (request_path[request_path.size() - 1] == '/')
  {
    std::string msg("Ready to accept Query");
    rep = reply::reply_html(msg);
    return;      
  }

  // If path ends in slash (i.e. is a directory) then add "index.html".
  //if (request_path[request_path.size() - 1] == '/')
  //{
  //  request_path += "index.html";
  //}


  size_t breakpoint = request_path.find_first_of("?");
  if (breakpoint == string::npos) breakpoint = request_path.length();
  std::string query(request_path.substr(breakpoint+1, request_path.length()));
  
  if (query.empty())
  {
    std::string msg("Ready to accept Query");
    rep = reply::reply_html(msg);
    return;      
  }
  
#if PALEO_DEBUG
  std::clog << "query: " << query << "\n";
#endif

  wms_query wms_query(query);

  boost::optional<unsigned> w = wms_query.width();
  boost::optional<unsigned> h = wms_query.height();

  if (!w || !h)
  {
      rep = reply::reply_html("missing width or height");
      //rep = reply::stock_reply(reply::bad_request);
      return;  
  }

  map_.resize(*w,*h);

  boost::optional<std::string> bbox_string = wms_query.get_bbox_string();
  if (!bbox_string)
  {
      rep = reply::reply_html("missing bbox");
      //rep = reply::stock_reply(reply::bad_request);
      return;  
  }
  
  boost::optional<Envelope<double> > bbox = wms_query.parse_bbox_string(*bbox_string);
  
  if (!bbox)
  {
      rep = reply::reply_html("failed to parse bbox");
      //rep = reply::stock_reply(reply::bad_request);
      return;  
  }

  image_32 im(*w,*h);

#if MAP_PER_IO
  map_.zoom_to_box(*bbox);
  agg_renderer<image_32> ren(map_,im);

#else

    #if MAP_REQUEST
      // requires ripped apart mapnik:Map object...
      mapnik::request r_(*w,*h);
      r_.set_srs(map_.srs());
      r_.set_buffer_size(128);
      boost::optional<color> const& bg = map_.background();
      if (bg) r_.set_background(*bg);
      
      r_.zoom_to_box(*bbox);
      // todo, pass only layers and styles?
      agg_renderer<image_32> ren(map_,im,r_);
    #else
    
      map_.zoom_to_box(*bbox);
      agg_renderer<image_32> ren(map_,im);
    
    #endif

#endif

#if PALEO_DEBUG
  std::clog << "rendering... \n";
#endif

  ren.apply();
  rep.content = save_to_string(im, "png");;
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type("png");
}

bool request_handler::url_decode(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 3 <= in.size())
      {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value)
        {
          out += static_cast<char>(value);
          i += 2;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if (in[i] == '+')
    {
      out += ' ';
    }
    else
    {
      out += in[i];
    }
  }
  return true;
}

} // namespace paleoserver
} // namespace http
