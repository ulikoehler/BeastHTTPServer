/*
 * BeastHTTPServer - A simple HTTP server based on boost::beast
 *
 * Copyright (c) 2023 Uli Köhler
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * GitHub repository: https://github.com/ulikoehler/BeastHTTPServer
 * Author: Uli Köhler
 * Web: https://techoverflow.net
 */
#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

typedef http::request<http::string_body> HTTPRequest;
typedef http::response<http::string_body> HTTPResponse;

struct HTTPRoute {
    http::verb method = http::verb::get;
    std::string path;
    std::function<HTTPResponse(HTTPRequest& req)> handler;
};
typedef std::vector<HTTPRoute> HTTPRoutes;

class HTTPServerListener;

struct HTTPServerParams {
    tcp::endpoint endpoint;
    HTTPRoutes routes;
};

class BeastHTTPServer {
public:
    BeastHTTPServer(
        const std::shared_ptr<HTTPServerParams>& params
    );

    /**
     * @brief Start asynchronous threads to process IO context messages
     * This function does not block but returns after starting the threads
     */
    void StartThreads(
        unsigned int nthreads = std::thread::hardware_concurrency()
    );
    void Stop();


    /**
     * @brief Runs the IO context in the current thread.
     * This will block the current thread until the server is terminated.
     */
    void RunBlocking();

private:
    bool started;
    net::io_context ctx_;
    std::shared_ptr<HTTPServerParams> params_;
    // Initialized in Start()
    std::shared_ptr<HTTPServerListener> listener_ = nullptr;
    std::vector<std::thread> threads_;
};