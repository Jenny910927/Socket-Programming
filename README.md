# Socket-Programming
資工四 B10902080 胡家榆



### Development Environment

- **Operating System**: Windows 11
- **Tools**: WSL2 + VSCode
- **Compiler**: g++ (Ubuntu 11.4.0-1ubuntu1~22.04) version 11.4.0

------

### Compilation Instructions

Using the Makefile to compile the C++ files.

For all:

```bash
make
```

For the server:

```bash
make server
```

For the client:

```bash
make client
```

------

### Usage Guide

1. **Run the Server**:
   Ensure that the specified port (default: **port 8800**) is not in use. Then, start the server:

   ```bash
   ./server
   ```

2. **Run the Client**:
   After the server is running, launch the client to establish a connection:

   ```bash
   ./client
   ```

------

### Additional Information

No additional setup or configuration is required to run this project.
