# Terminal-Based Attendance System

## Overview

This is a simple terminal-based client-server attendance system written in C. The system allows clients to connect to a server, mark their attendance, and view the attendance list.

## Usage

### Client

1. Open a terminal and compile the client:

    ```bash
    gcc client.c -o client
    ```

2. Run the client:

    ```bash
    ./client
    ```

3. Enter your name when prompted to mark attendance.

### Server

1. Open another terminal and compile the server:

    ```bash
    gcc server.c -o server
    ```

2. Run the server:

    ```bash
    ./server
    ```

3. The server listens on port 8080 and stores attendance in a file named `attendance.txt`. Each client connection appends the student's name to the file.

## Notes

- This is a basic example. In a real-world scenario, consider enhancing security, error handling, and using a database for storing attendance records.
- Ensure that you have the necessary permissions to create and write to files in the working directory.
- The server runs indefinitely; you may manually stop it with a keyboard interrupt (`Ctrl+C`).
