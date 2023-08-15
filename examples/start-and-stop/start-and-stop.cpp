#include "BeastHTTPServer.hpp"
#include <iostream>

using namespace std;

const HTTPRoute helloWorldRoute = {
    .method = http::verb::get,
    .path = "/",
    .handler = [](HTTPRequest& req) {
        HTTPResponse res;
        res.body() = "Hello world!";
        res.prepare_payload();
        return res;
    }
};

int main(int argc, char* argv[])
{
    auto params = std::make_shared<HTTPServerParams>(HTTPServerParams{
        .endpoint = tcp::endpoint(
            net::ip::make_address("0.0.0.0"),
            18080
        ),
        .routes = {
            helloWorldRoute
        }
    });

    // Runs only in the main thread
    BeastHTTPServer server(params);
    server.StartThreads(1);

    // Wait for 15 seconds
    cout << "Server started. Waiting 15 seconds..." << endl;
    std::this_thread::sleep_for(std::chrono::seconds(15));

    // Stop!
    cout << "Stopping server..." << endl;
    server.Stop();

    return EXIT_SUCCESS;
}