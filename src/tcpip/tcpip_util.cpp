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
using boost::asio::ip::tcp;

// Connects to server at given hostname / port
// Returns socket of connection
void connect_to_server(const std::string& hostname, const int32_t port) {
  boost::asio::io_context io_context;

  char* str_port = "33000";

  tcp::resolver resolver(io_context);
  tcp::resolver::results_type endpoints =
      resolver.resolve("localhost", str_port);

  tcp::socket socket(io_context);
  boost::asio::connect(socket, endpoints);

  std::cout << "Connected" << std::endl;

  /*
    std::cout << "Connecting to " << hostname << ":" << port << std::endl;
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    std::string port_str = std::to_string(port);
    std::cout << "port str" << port_str << std::endl;
    tcp::resolver::results_type endpoints = resolver.resolve("localhost",
    "1025"); std::cout << "endpoints okay" << std::endl; tcp::socket
    socket(io_context); std::cout << "socket okay" << std::endl;
    boost::asio::connect(socket, endpoints);

    std::cout << "Connected to " << hostname << ":" << port_str << std::endl;
  */
  return;
}

// Initializes a server at given port
void server_init(const int port) {
  std::cout << "Initializing server at port WTF?! " << port << std::endl;
  std::cout << port << std::endl;

  try {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Server initialized" << std::endl;

    for (;;) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);
    }

  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

// Sends data to socket
// @param socket socket at which server is listening
// @param data data to send
// @param bytes number of bytes in data to send
void send_data(tcp::socket& socket, const std::string& data) {
  std::cout << "Sending message: " << data << std::endl;
  boost::system::error_code ignored_error;
  boost::asio::write(socket, boost::asio::buffer(data), ignored_error);
  std::cout << "Sent message: " << data;
}

// Receives data
// @param socker socker at which we receive data from
// @return numbers of bytes of data received
void receive_data(tcp::socket& socket, void* data) {
  std::cout << "Receiving data";

  boost::array<char, 128> buf;
  boost::system::error_code error;

  size_t len = socket.read_some(boost::asio::buffer(buf), error);

  std::cout << "got " << len << " data" << std::endl;

  if (error == boost::asio::error::eof) {
    std::cerr << "Connected closed!";
    throw boost::system::system_error(error);
  } else if (error) {
    throw boost::system::system_error(error);  // Some other error.
  }
  std::cout << "Got data " << buf.size() << std::endl;

  return;  // -1;  // buf.size();
}