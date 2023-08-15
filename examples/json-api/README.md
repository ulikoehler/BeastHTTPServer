## hello-world example

HTTP server with JSON api example.
This example uses `boost::json`

### How to compile

```bash
cmake .
make
./json-api
```

Then open `http://localhost:18080/api/example` in your browser.
This will show

```
{"message":"Hello, World!"}
```