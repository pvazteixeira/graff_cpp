#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <zmq.hpp>

#include "json.hpp"

using nlohmann::json;

const double PI = 3.141592653589793238463;

struct GraffConfig {
  std::string userId;
  std::string robotId;
  std::string sessionId;
};

std::string getStatus(const GraffConfig &cfg) {
  std::cout << "here: " << cfg.sessionId <<std::endl;

  json request;
  request["userId"] = cfg.userId;
  request["robotId"] = cfg.robotId;
  request["sessionId"] = cfg.sessionId;
  request["type"] = "getStatus";
  std::string request_str = request.dump(2); // serialize
  return request_str;
}


int main(int argCount, char **argValues) {
  //  Prepare our context and socket
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REQ);

  std::cout << "Connecting to navi server…" << std::endl;
  socket.connect("tcp://localhost:5555");

  std::string robot_id = "FeitorBot";
  std::string session_id = "FeitorBotTestSession001";

  GraffConfig gff;
  gff.userId = "Pedro";
  gff.robotId = "PedroBot";
  gff.sessionId = "ThisIsACTest001";

  std::string stsr = getStatus(gff);

  zmq::message_t msg(stsr.length());
  memcpy(msg.data(), stsr.c_str(), stsr.length());
  socket.send(msg);

  std::cout << "Asked for status" << std::endl;

  for (int i = 0; i < 6; ++i) {
    // create request
    json request;
    request["robot_id"] = robot_id;
    request["session_id"] = session_id;
    request["type"] = "AddOdometry2D";
    request["measurement"] = {10.0, 0.0, PI / 3.0};
    request["covariance"] = {{0.1, 0.0, 0.1}, {0.1, 0.0, 0.1}, {0.1, 0.0, 0.1}};
    std::string request_str = request.dump(2); // serialize

    // send it
    zmq::message_t msg(request_str.length());
    memcpy(msg.data(), request_str.c_str(), request_str.length());
    socket.send(msg);

    std::cout << "Sent request " << i << "…";

    // wait for reply
    zmq::message_t reply;
    if (socket.recv(&reply) < 0) {
      std::cerr << "something went wrong :(";
      break;
    }
    std::string result =
        std::string(static_cast<char *>(reply.data()), reply.size());
    std::cout << result << "\n";

    if (result.compare("OK")) {
      std::cerr << "server is unhappy with request." << "\n";
      break;
    }
  }
  return(0);
}
