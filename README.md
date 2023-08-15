# BeastHTTPServer
Easy-to-use basic C++ webserver library using [boost::beast](https://github.com/boostorg/beast).

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

### Performance

#### Single-threaded hello-world

This performance test was performed with the hello-world example (single-threaded, standard compiler flags, Ubuntu 22.04, Core i7-6700). Test performed on git revision `416caa0`.

```sh
siege -t 10s -c 255 http://localhost:18080
```

Output:
```json
{       "transactions":                        46786,
        "availability":                       100.00,
        "elapsed_time":                         9.17,
        "data_transferred":                     0.54,
        "response_time":                        0.05,
        "transaction_rate":                  5102.07,
        "throughput":                           0.06,
        "concurrency":                        253.59,
        "successful_transactions":             46786,
        "failed_transactions":                     0,
        "longest_transaction":                  0.12,
        "shortest_transaction":                 0.00
}
```

### Multi-threaded hello-world

This performance test was performed with the hello-world example (standard compiler flags, Ubuntu 22.04, Core i7-6700) with

```cpp
server.StartThreads(7);
```

added just before `server.RunBlocking()`. Test performed on git revision `416caa0`.
In effect, this runs 8 threads on my 8-core i7-6700 CPU.

```sh
siege -t 10s -c 255 http://localhost:18080
```

```json
{       "transactions":                        80709,
        "availability":                       100.00,
        "elapsed_time":                         9.43,
        "data_transferred":                     0.92,
        "response_time":                        0.03,
        "transaction_rate":                  8558.75,
        "throughput":                           0.10,
        "concurrency":                        253.48,
        "successful_transactions":             80709,
        "failed_transactions":                     0,
        "longest_transaction":                  0.12,
        "shortest_transaction":                 0.01
}
```

### Optimized hello-world

The source code for this is identical to the multi-threaded hello-world (see above), but with

```cmake
target_compile_options(hello-world PRIVATE -O3 -march=native -ffast-math)
```
added to the `CMakeLists.txt` in the `hello-world` directory. Test performed on git revision `416caa0`.

Using
```
siege -q -t 10s -c 1000 http://localhost:18080
```

we can achieve
```json
{       "transactions":                       132258,
        "availability":                       100.00,
        "elapsed_time":                         9.69,
        "data_transferred":                     1.51,
        "response_time":                        0.02,
        "transaction_rate":                 13648.92,
        "throughput":                           0.16,
        "concurrency":                        253.44,
        "successful_transactions":            132258,
        "failed_transactions":                     0,
        "longest_transaction":                  0.12,
        "shortest_transaction":                 0.00
}
```

More than 13500 requests per second using this very simple program is quite impressive.

### Parallel connection test

This test was performed with the *Optimized hello-world* example (see above). . Test performed on git revision `416caa0`.

`siege` was configured to use 1000 concurrent connections using
```sh
siege -q -t 10s -c 1000 http://localhost:18080
```

The server still maintained a 100% availability and achieved a transaction rate of 1.3 million transactions per second.

```json
{       "transactions":                        86577,
        "availability":                       100.00,
        "elapsed_time":                         9.32,
        "data_transferred":                     0.99,
        "response_time":                        0.11,
        "transaction_rate":                  9289.38,
        "throughput":                           0.11,
        "concurrency":                        981.52,
        "successful_transactions":             86577,
        "failed_transactions":                     0,
        "longest_transaction":                  0.52,
        "shortest_transaction":                 0.00
}
```

### Who?

[Uli Köhler](https://techoverflow.net)