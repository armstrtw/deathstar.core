#include <stdint.h>
#include <iostream>
#include <zmq.hpp>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

// worker ready signal, increment capacity
void process_worker_signal(zmq::socket_t& worker_signal, int& number_of_workers) {
  zmq::message_t worker_signal_msg;
  worker_signal.recv(&worker_signal_msg);
  ++number_of_workers;
}

// capacity request
void process_status_request(zmq::socket_t& status, int& number_of_workers) {
  // null request
  zmq::message_t incoming_msg;
  status.recv(&incoming_msg);

  // send current capacity
  zmq::message_t status_msg(sizeof(number_of_workers));
  memcpy(status_msg.data(), &number_of_workers, sizeof(number_of_workers));
  status.send(status_msg);
}

// relay data from workers to frontend
void process_backend(zmq::socket_t& frontend, zmq::socket_t& backend) {
  int64_t more;
  size_t moresz;
  moresz = sizeof(more);

  while(true) {
    zmq::message_t worker_msg;
    backend.recv(&worker_msg);
    backend.getsockopt(ZMQ_RCVMORE, &more, &moresz);
    frontend.send(worker_msg, more ? ZMQ_SNDMORE : 0);
    if(!more) {
      break;
    }
  }
}

// job allocation, relay request to workers
void process_frontend(zmq::socket_t& frontend, zmq::socket_t& backend, int& number_of_workers) {
  int64_t more;
  size_t moresz;
  moresz = sizeof(more);

  // decrement available workers
  --number_of_workers;

  // process incoming using multipart aware code
  while(true) {
    zmq::message_t client_msg;
    frontend.recv(&client_msg);
    frontend.getsockopt(ZMQ_RCVMORE, &more, &moresz);
    backend.send(client_msg, more ? ZMQ_SNDMORE : 0);
    if(!more) {
      break;
    }
  }
}

void deathstar(zmq::socket_t& frontend, zmq::socket_t& backend, zmq::socket_t& worker_signal, zmq::socket_t& status) {
  int number_of_workers(0);

  // loosely copied from lazy pirate (ZMQ guide)
  while (1) {
    zmq_pollitem_t items [] = {
      { worker_signal, 0, ZMQ_POLLIN, 0 },
      { status, 0, ZMQ_POLLIN, 0 },
      { backend,  0, ZMQ_POLLIN, 0 },
      { frontend, 0, ZMQ_POLLIN, 0 }
    };
    // frontend only if we have available workers
    // otherwise poll both worker_signal and backend
    cout << "starting poll with numworkers: " << number_of_workers << endl;
    int rc = zmq_poll(items, number_of_workers ? 4 : 3, -1);
    
    //  Interrupted
    if(rc == -1) {
      break;
    }

    //  worker_signal -- worker came online
    if(items[0].revents & ZMQ_POLLIN) {
      std::cout << "process_worker_signal (workers before): " << number_of_workers << std::endl;
      process_worker_signal(worker_signal, number_of_workers);
      std::cout << "process_worker_signal (workers after): " << number_of_workers << std::endl;
    }

    //  status -- client status request
    if(items[1].revents & ZMQ_POLLIN) {
      std::cout << "process_status_req: " << number_of_workers << std::endl;
      process_status_request(status, number_of_workers);
    }

    //  backend -- send data from worker to client
    if(items[2].revents & ZMQ_POLLIN) {
      std::cout << "process_backend: " << number_of_workers << std::endl;
      process_backend(frontend, backend);
    }

    // frontend -- send data from client to worker
    if (items[3].revents & ZMQ_POLLIN) {
      std::cout << "process_frontend: " << number_of_workers << std::endl;
      process_frontend(frontend, backend, number_of_workers);
    }
  }
}

int main (int argc, char* argv[]) {
  if(argc != 5) {
    cout << "usage: deathstar master worker worker_signal status" << endl;
    return 1;
  }

  zmq::context_t context(1);
  zmq::socket_t frontend(context,ZMQ_ROUTER);
  zmq::socket_t status(context,ZMQ_REP);
  zmq::socket_t backend(context,ZMQ_DEALER);
  zmq::socket_t worker_signal(context,ZMQ_PULL);


  try {
    frontend.bind(argv[1]);
    status.bind(argv[2]);
    backend.bind(argv[3]);
    worker_signal.bind(argv[4]);
  } catch(std::exception& e) {
    cerr << e.what() << endl;
    return 1;
  }

  // start device
  cout << "starting device" << endl;
  deathstar(frontend, backend, worker_signal, status);
  return 0;
}
