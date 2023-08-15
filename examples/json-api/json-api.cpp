#include "BeastHTTPServer.hpp"
#include <boost/json.hpp>

namespace json = boost::json;

const HTTPRoute helloWorldRoute = {
    .method = http::verb::get,
    .path = "/api/example",
    .handler = [](HttpRequest& req) {
        HttpResponse res;


        json::object obj{
            {"message", "Hello World!"},
        };

        res.body() = json::serialize(obj);
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