/*
 * tcp_stream.h
 *
 *  Created on: Jul 15, 2013
 *      Author: king
 */

#ifndef TCP_STREAM_H_
#define TCP_STREAM_H_

#include<istream>
#include<ostream>

namespace wbase { namespace common { namespace net {

class tcp_stream : public std::istream, public std::ostream {

};

} } }



#endif /* TCP_STREAM_H_ */
