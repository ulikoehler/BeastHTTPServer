## start-and-stop example

This example is based on the [hello-world](../hello-world/) example and demonstrates a server that starts and automatically stops 15 seconds later

### How to compile

```bash
cmake .
make
./start-and-stop
```

Then open `http://localhost:18080/` in your browser.
This will show

```
Hello world!
```
... until 15 seconds later, it cleanly shuts down