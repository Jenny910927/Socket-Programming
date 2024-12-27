# Socket-Programming
B10902080 胡家榆



### Development Environment

- **Operating System**: Windows 11
- **Tools**: WSL2 + VSCode
- **Compiler**: g++ (Ubuntu 11.4.0-1ubuntu1~22.04) version 11.4.0

------

### Compilation Instructions

To compile the C++ files, use the following commands:

For the server:

```bash
g++ -g server.cpp -o server
```

For the client:

```bash
g++ -g client.cpp -o client
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
