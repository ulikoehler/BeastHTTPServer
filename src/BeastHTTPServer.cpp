#include "BeastHTTPServer.hpp"

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

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template <class Body, class Allocator>
http::message_generator
handle_request(http::request<Body, http::basic_fields<Allocator>>&& req, const HTTPRoutes& routes)
{
    // Returns a bad request response
    auto const bad_request =
    [&req](beast::string_view why)
    {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a not found response
    auto const not_found =
    [&req](beast::string_view target)
    {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error =
    [&req](beast::string_view what)
    {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if( req.method() != http::verb::get &&
        req.method() != http::verb::head)
        return bad_request("Unknown HTTP-method");

    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return bad_request("Illegal request-target");

    /** 
     * Check if the request matches any of the routes
     */
     for (auto& route : routes) {
        if (route.method == req.method() && route.path == req.target()) {
            return route.handler(req);
        }
    }
    // Nothing found -> 404
    return not_found(std::string(req.target()) + std::string(" - no matching route found"));
    /*http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return res;*/
}

// Handles an HTTP server connection
class HTTPServerSession : public std::enable_shared_from_this<HTTPServerSession>
{
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::shared_ptr<HTTPServerParams> params_;

public:
    // Take ownership of the stream
    HTTPServerSession(
        tcp::socket&& socket,
        const std::shared_ptr<HTTPServerParams>& params)
        : stream_(std::move(socket)), params_(params)
    {
    }

    // Start the asynchronous operation
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(stream_.get_executor(),
                      beast::bind_front_handler(
                          &HTTPServerSession::do_read,
                          shared_from_this()));
    }

    void
    do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(
                &HTTPServerSession::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");

        // Send the response
        send_response(handle_request(std::move(req_), params_->routes));
    }

    void
    send_response(http::message_generator&& msg)
    {
        bool keep_alive = msg.keep_alive();

        // Write the response
        beast::async_write(
            stream_,
            std::move(msg),
            beast::bind_front_handler(
                &HTTPServerSession::on_write, shared_from_this(), keep_alive));
    }

    void
    on_write(
        bool keep_alive,
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(! keep_alive)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // Read another request
        do_read();
    }

    void
    do_close()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

// Accepts incoming connections and launches the sessions
class HTTPServerListener : public std::enable_shared_from_this<HTTPServerListener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<HTTPServerParams> params_;

public:
    HTTPServerListener(
        net::io_context& ioc,
        const std::shared_ptr<HTTPServerParams>& params)
            : ioc_(ioc), acceptor_(net::make_strand(ioc)), params_(params)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(params->endpoint.protocol(), ec);
        if(ec)
        {
            throw beast::system_error(ec, "Failed to open acceptor");
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            throw beast::system_error(ec, "Set reuse_address = true");
            return;
        }

        // Bind to the server address
        acceptor_.bind(params->endpoint, ec);
        if(ec)
        {
            throw beast::system_error(ec, "Failed to bind to address");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            throw beast::system_error(ec, "Failed to listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        do_accept();
    }

private:

    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &HTTPServerListener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            fail(ec, "accept");
            return; // To avoid infinite loop
        }
        else
        {
            // Create the session and run it
            std::make_shared<HTTPServerSession>(
                std::move(socket), params_
            )->run();
        }

        // Accept another connection
        do_accept();
    }
};
//------------------------------------------------------------------------------

BeastHTTPServer::BeastHTTPServer(
    const std::shared_ptr<HTTPServerParams>& params): ctx_(), params_(params)
{
    // Create and launch a listening port
    listener_ = std::make_unique<HTTPServerListener>(ctx_, params_);
    listener_->run();
}

void BeastHTTPServer::StartThreads(unsigned int nthreads) {
    // Run the I/O service on the requested number of threads
    threads_.reserve(nthreads);
    for(auto i = nthreads; i > 0; --i) {
        threads_.emplace_back(
            [this]
            {
                this->ctx_.run();
            }
        );
    }
}

void BeastHTTPServer::RunBlocking() {
    this->ctx_.run();
}