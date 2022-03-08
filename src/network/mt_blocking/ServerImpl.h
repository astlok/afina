#ifndef AFINA_NETWORK_MT_BLOCKING_SERVER_H
#define AFINA_NETWORK_MT_BLOCKING_SERVER_H

#include <atomic>
#include <thread>
#include <set>
#include <mutex>
#include <condition_variable>

#include <afina/network/Server.h>

namespace spdlog {
class logger;
}

namespace Afina {
namespace Network {
namespace MTblocking {

/**
 * # Network resource manager implementation
 * Server that is spawning a separate thread for each connection
 */
class ServerImpl : public Server {
public:
    ServerImpl(
        std::shared_ptr<Afina::Storage> ps, 
        std::shared_ptr<Logging::Service> pl, 
        int max_threads = std::thread::hardware_concurrency(), 
        long limeout = 5
    );

    ~ServerImpl();

    // See Server.h
    void Start(uint16_t port, uint32_t, uint32_t) override;

    // See Server.h
    void Stop() override;

    // See Server.h
    void Join() override;

protected:
    /**
     * Method is running in the connection acceptor thread
     */
    void OnRun();

private:
    // Logger instance
    std::shared_ptr<spdlog::logger> _logger;

    // Atomic flag to notify threads when it is time to stop. Note that
    // flag must be atomic in order to safely publisj changes cross thread
    // bounds
    std::atomic<bool> running;

    // Server socket to accept connections on
    int _server_socket;

    long _timeout;
    // Thread to run network on
    std::thread _thread;

    std::set<int> _client_sockets;
    std::mutex _sockets_mu;

    int _max_num_threads;

    std::atomic<size_t> _cur_num_threads;

    void process_connection(int client_socket) noexcept;

    std::condition_variable _cv;
    std::mutex _stop_mu;

};

} // namespace MTblocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_BLOCKING_SERVER_H
