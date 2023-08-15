#include "BeastHTTPServer.hpp"

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
    server.RunBlocking();

    return EXIT_SUCCESS;
}