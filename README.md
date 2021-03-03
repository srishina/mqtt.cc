# mqtt.cpp
MQTT C++ client library

The library is work in progress. Currently MQTT packet parser is implemented.

# How to build the library?

Build system uses cmake and C++17 is needed to compile the library as the source uses std::optional which is available from C++17 onwards

Build and run the unit tests. For testing doctest is used.

```bash
mkdir build
cd build
cmake .. && make -j4
./mqtt_unit_tests
```

# The following classes are used from STL

std::vector, std::string, std::optional, std::shared_ptr, std::unique_ptr

If an exception is thrown, the objects are left in a consistent, usable state whenever possible.

The exception is thrown in the following cases:

* Out of memory error
* MQTT protocol error, for e.g if the packet contains a property that is not allowed by the protocol, then in those cases, an exception is thrown and the connection will be closed. As part of the MQTT specification the presence of duplicate property should force the network connection to be closed. Note: protocol error is handled in the worker thread and the exception is propagated.

No exception is thrown in other scenarios.

Note: The library is work in progress.
