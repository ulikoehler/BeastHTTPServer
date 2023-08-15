# BeastHTTPServer
Easy-to-use basic C++ webserver library using [boost::beast](https://github.com/boostorg/beast)

## What & Why?

`BeastHTTPServer` is a library that wraps a boost::beast asynchronous HTTP server into an easy-to-use API. While boost::beast is a great library, it provides it high-level functionality in a rather low-level way and is rather intimidating to use if you *just* want to get a webserver running in C++. `BeastHTTPServer` aims to provide a simple API in order to do just that. It is not intended to wrap all boost::beast functionality, but made easy to modify if you want to roll your own variant.

Currently, `BeastHTTPServer` works for creating simple HTTP servers with basic routing.

Pull requests are welcome. I will provide some more usage examples in the future. Please note that this library is still in early development and the API might change. Please consider copying the code into your own project so your project doesn't break on changes.

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

For more examples, see the `examples` directory.

### Who

[Uli Köhler](https://techoverflow.net)