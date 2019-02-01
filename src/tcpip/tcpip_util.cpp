//*****************************************************************************
// Copyright 2018-2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

#include "ngraph/ngraph.hpp"
#include "tcpip/tcpip_util.hpp"

using boost::asio::ip::tcp;
using namespace ngraph;

// Connects to server at given hostname / port
// Returns socket of connection
tcp::socket runtime::he::network::connect_to_server(const std::string& hostname,
                                                    const size_t port) {
  boost::asio::io_context io_context;
  std::string str_port = std::to_string(port);

  std::cout << "Connecting to server at " << hostname << ":" << port
            << std::endl;

  tcp::resolver resolver(io_context);
  tcp::resolver::results_type endpoints = resolver.resolve(hostname, str_port);
  tcp::socket socket(io_context);
  boost::asio::connect(socket, endpoints);

  std::cout << "Connected" << std::endl;
  return socket;
}

// Reads data from socket, and writes it back
void runtime::he::network::session(tcp::socket sock) {
  const int max_length = 66000;
  try {
    for (;;) {
      char data[max_length];

      boost::system::error_code error;
      size_t length = sock.read_some(boost::asio::buffer(data), error);
      if (error == boost::asio::error::eof)
        break;  // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error);  // Some other error.

      std::cout << "Server got message length " << length << std::endl;

      boost::asio::write(sock, boost::asio::buffer(data, length));
    }
  } catch (std::exception& e) {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

// Initializes a server at given port
void runtime::he::network::server_init(const size_t port,
                                       const size_t conn_limit) {
  std::cout << "Initializing server at port " << port << std::endl;

  try {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Server initialized with up to " << conn_limit
              << " connections" << std::endl;

    for (size_t i = 0; i < conn_limit; ++i) {
      std::cout << "Starting accept thread " << i << std::endl;
      std::thread(session, acceptor.accept()).detach();
    }

  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

// Sends data to socket
// @param socket socket at which server is listening
// @param data data to send
// @param bytes number of bytes in data to send
void runtime::he::network::send_data(tcp::socket& socket,
                                     const std::string& data) {
  std::cout << "Sending message size: " << data.size() << std::endl;
  boost::system::error_code ignored_error;
  std::string data_size{data.size()};
  // Send data size first.



  boost::asio::write(socket, boost::asio::buffer(data_size), ignored_error);
  boost::asio::write(socket, boost::asio::buffer(data), ignored_error);
  std::cout << "Sent message size: " << data.size() << std::endl;
}

// Receives data
// @param socker socker at which we receive data from
// @return numbers of bytes of data received
void runtime::he::network::receive_data(tcp::socket& socket, void* data) {
  boost::array<char, 128> buf;
  boost::system::error_code error;

  size_t len = socket.read_some(boost::asio::buffer(buf), error);

  std::cout << "Socket received " << len << " bytes of data" << std::endl;

  if (error == boost::asio::error::eof) {
    std::cerr << "Connected closed!";
    throw boost::system::system_error(error);
  } else if (error) {
    throw boost::system::system_error(error);  // Some other error.
  }
  std::cout << "Data is " << buf.data() << std::endl;
  std::cout << "Got data size " << buf.size() << std::endl;

  return;  // -1;  // buf.size();
}