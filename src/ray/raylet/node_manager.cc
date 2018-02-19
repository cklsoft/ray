#include <iostream>

#include <boost/bind.hpp>

#include "common.h"
#include "node_manager.h"

using namespace std;
namespace ray {

NodeServer::NodeServer(boost::asio::io_service& io_service, const std::string &socket_name)
    : acceptor_(io_service, boost::asio::local::stream_protocol::endpoint(socket_name)),
      socket_(io_service),
      worker_pool_(0) {
  // Start listening for clients.
  doAccept();
}

void NodeServer::doAccept() {
  acceptor_.async_accept(socket_,
      boost::bind(&NodeServer::handleAccept, this, boost::asio::placeholders::error)
      );
}

void NodeServer::handleAccept(const boost::system::error_code& error) {
  if (!error) {
    // Accept a new client.
    // TODO(swang): Remove the client upon disconnection.
    auto new_connection = new ClientConnection(std::move(socket_), boost::bind(&WorkerPool::AddWorkerConnection, &worker_pool_, _1));
    new_connection->ProcessMessages();
    clients_.push_back(std::unique_ptr<ClientConnection>(new_connection));
  }
  // We're ready to accept another client.
  doAccept();
}

} // end namespace ray

#ifndef NODE_MANAGER_TEST
int main(int argc, char *argv[]) {
  CHECK(argc == 2);

  boost::asio::io_service io_service;
  ray::NodeServer server(io_service, std::string(argv[1]));
  io_service.run();
  return 0;
}
#endif