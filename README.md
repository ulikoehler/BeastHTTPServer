# BeastHTTPServer
Easy-to-use basic C++ webserver library using [boost::beast](https://github.com/boostorg/beast)

## What?

`BeastHTTPServer` is a library that wraps a boost::beast asynchronous HTTP server into an easy-to-use API.

## How to use?

[hello-world example](examples/hello-world/):
```cpp
#include "BeastHTTPServer.hpp"

const HTTPRoute helloWorldRoute = {
    .method = http::verb::get,
    .path = "/",
    .handler = [](HttpRequest& req) {
        HttpResponse res;
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
```

For more examples, see `examples` directory.
