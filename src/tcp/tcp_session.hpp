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

#pragma once
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include "tcp/tcp_message.hpp"

using boost::asio::ip::tcp;

namespace ngraph {
namespace runtime {
namespace he {
class TCPSession : public std::enable_shared_from_this<TCPSession> {
 public:
  TCPSession(
      tcp::socket socket,
      std::function<TCPMessage(const runtime::he::TCPMessage&)> message_handler)
      : m_socket(std::move(socket)),
        m_message_callback(std::bind(message_handler, std::placeholders::_1)) {}

  void start() { do_read_header(); }

 private:
  void do_read_header() {
    auto self(shared_from_this());
    std::cout << "Server reading message header" << std::endl;
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_message.header_ptr(),
                            runtime::he::TCPMessage::header_length),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec & m_message.decode_header()) {
            std::cout << "Server read message header" << std::endl;
            std::cout << "Length of message header is " << length << " bytes "
                      << std::endl;
            do_read_body();
          } else {
            if (ec) {
              std::cout << "Server Error reading message: " << ec.message()
                        << std::endl;
              std::cout << "Closing TCP server by throwing exception"
                        << std::endl;  // TODO: see boost asio server example
                                       // for better stopping of server
              throw std::exception();
            }
            if (!m_message.decode_header()) {
              std::cout << "Cant decode message header" << std::endl;
            }
          }
        });
  }

  void do_read_body() {
    std::cout << "Server reading message body" << std::endl;
    auto self(shared_from_this());
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_message.body_ptr(), m_message.body_length()),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            std::cout << "Server read message body length " << length
                      << std::endl;

            m_message.decode_body();

            auto response = m_message_callback(m_message);
            std::cout << "Server about to write response" << std::endl;
            std::cout << "response size " << response.size() << std::endl;
            do_write(response);

            // do_read_header();
          } else {
            std::cout << "Error reading message body: " << ec.message()
                      << std::endl;
          }
        });
  }

  void do_write(const TCPMessage& message) {
    auto self(shared_from_this());
    std::cout << "Server about to write message of size " << message.size()
              << std::endl;
    boost::asio::async_write(
        m_socket, boost::asio::buffer(message.header_ptr(), message.size()),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            std::cout << "Server wrote message size " << length << std::endl;
            // do_read_header();
          } else {
            std::cout << "Error writing message in session: " << ec.message()
                      << std::endl;
          }
        });
  }

  TCPMessage m_message;
  tcp::socket m_socket;

  // How to handle the message
  std::function<runtime::he::TCPMessage(const runtime::he::TCPMessage&)>
      m_message_callback;
};  // namespace he
}  // namespace he
}  // namespace runtime
}  // namespace ngraph
