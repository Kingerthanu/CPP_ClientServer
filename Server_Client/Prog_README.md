
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

p4.cpp:

Application:
  - This Application Simulates A File Transfer Server Using Multi-Threaded Parallelism For Handling Various Client Transactions.

Capabilities:
  - Server Mode: Starts A Server That Listens For Client Connections On An Arbitrary Port.
  - Client Modes: Clients Can Perform The Following Operations On The Server:
    - Client Message: p4 client               - Gets Message From Client And Reports The Message In Server's Terminal.
    - Check File: p4 check <ip> <port> <path> - Checks If A File Exists At The Given Path On The Server.
    - Load File: p4 load <ip> <port> <path> - Loads And Prints The Content Of The File From The Server.
    - Store File: p4 store <ip> <port> <path> - Stores The Content From The Clients Standard Input To The File On The Server.
    - Delete File: p4 delete <ip> <port> <path> - Deletes The File At The Given Path On The Server.

Inferences:
  - The Server Continues To Run Until Killed By A Signal Such As SIGINT (Triggered By CTRL + C).
  - Multiple Clients Can Connect To The Server And Perform Transactions Concurrently.
  - Expects File Paths To Be Valid And Properly Formatted.
  - Handles Potential Network Errors And Socket Failures Gracefully.

Behavior:
  - Server Mode:
    - Listens For Connections On An Arbitrary Port.
    - Accepts Client Connections And Creates A New Thread To Handle Each Client Request.
    - Prints The Port Number It Is Listening On At Startup.
  - Client Modes:
    - Client: Connects To The Server And Sends Client Standard Input To Be Reported.
    - Check: Connects To The Server And Checks For The Existence Of A File.
    - Load: Connects To The Server, Retrieves The File Content, And Prints It To The Terminal.
    - Store: Connects To The Server And Sends The Content From The Standard Input To Be Stored In The File.
    - Delete: Connects To The Server And Deletes The Specified File.

Call Format:
  - Server: ./p4 server
  - Check File: ./p4 check <ip> <port> <path>
  - Load File: ./p4 load <ip> <port> <path>
  - Store File: ./p4 store <ip> <port> <path>
  - Delete File: ./p4 delete <ip> <port> <path>
  - Client Message: ./p4 client

Edge Cases:
  - Exit Codes:
    - 0 -> Proper Functionality.
    - 1 -> Improper Functionality Or Error Encountered.

Exception Types:
  - Usage:
    - p4 <mode> [additional args...] - Thrown If Invalid Argument Amount Is Supplied.
  - File Errors:
    - The File Doesn't Exist - Thrown If A Non-Existent File Path Is Supplied.
    - Cannot Open File - Thrown If The File Cannot Be Opened.
    - Cannot Close File - Thrown If The File Cannot Be Closed.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Function Details:

  client(in_addr_t ip, in_port_t port)
    - This Function Connects To The Server At The Given IP Address And Port Number.
    - It Then Reads A One-Line Message From The User And Sends It To The Server.

  server()
    - This Function Starts The Server, Which Listens For Client Connections On An Arbitrary Port.
    - It Uses Signal Handling To Clean Up Resources On Exit.
    - It Continuously Accepts Client Connections And Spawns A New Thread To Handle Each Client Request.

  parse_ip(char* ip_str)
    - This Function Parses A String Into An IP Address Of Type in_addr_t.
    - Throws A Runtime Error If The Parsing Fails.

  parse_port(char* port_str)
    - This Function Parses A String Into A Port Number Of Type in_port_t.
    - Throws A Runtime Error If The Parsing Fails.

  connect_to(in_addr_t ip, in_port_t port)
    - This Function Creates A Socket And Connects It To The Given IP Address And Port Number.
    - Throws A Runtime Error If The Connection Fails.

  arbitrary_socket()
    - This Function Creates A Socket Bound To An Arbitrary Port.
    - Throws A Runtime Error If The Binding Fails.

  get_port(int socket_fd)
    - This Function Retrieves The Port Number That The Given Socket Is Bound To.
    - Throws A Runtime Error If The Retrieval Fails.

  is_socket_valid(int socket_fd)
    - This Function Checks If The Given Socket Is Alive And Error-Free.
    - Returns True If The Socket Is Valid, Otherwise Returns False.

  recv_all(int socket_fd, void* buffer, size_t length)
    - This Function Receives A Specified Number Of Bytes From The Given Socket And Stores Them In The Buffer.
    - Returns True If The Transaction Was Successful, Otherwise Returns False.

  send_all(int socket_fd, const void* buffer, size_t length)
    - This Function Sends A Specified Number Of Bytes From The Buffer To The Given Socket.
    - Returns True If The Transaction Was Successful, Otherwise Returns False.

  clientFileStoreServer(const int& socket_fd, const std::string& serverRequestPath)
    - This Function Stores Data From The Client's Standard Input To The Server At The Specified Path.
    - Handles Socket Communication And Closes The Socket Before Exiting.

  clientFileDeleteServer(const int& socket_fd, const std::string& serverRequestPath)
    - This Function Deletes A File At The Specified Path On The Server.
    - Handles Socket Communication And Closes The Socket Before Exiting.

  clientFileCheckServer(const int& socket_fd, const std::string& serverRequestPath)
    - This Function Checks If A File Exists At The Specified Path On The Server.
    - Handles Socket Communication And Closes The Socket Before Exiting.

  clientFileLoadServer(const int& socket_fd, const std::string& serverRequestPath)
    - This Function Loads And Retrieves The Content Of A File From The Server At The Specified Path.
    - Handles Socket Communication And Closes The Socket Before Exiting.

  fileCheckServer(int connection_fd)
    - This Function Checks For The Existence Of A File At The Specified Path On The Server And Sends The Result Back To The Client.
    - Handles Socket Communication And Closes The Socket Before Exiting.

  fileLoadServer(int connection_fd)
    - This Function Loads A File From The Specified Path On The Server And Sends Its Content Back To The Client.
    - Handles Socket Communication And Closes The Socket Before Exiting.

  fileStoreServer(int connection_fd)
    - This Function Receives Data From The Client And Stores It In A File At The Specified Path On The Server.
    - Handles Socket Communication And Closes The Socket Before Exiting.

  fileDeleteServer(int connection_fd)
    - This Function Deletes A File At The Specified Path On The Server And Sends The Result Back To The Client.
    - Handles Socket Communication And Closes The Socket Before Exiting.

  clientCommunicationServer(int connection_fd)
    - This Function Receives A Message From The Client And Prints It To The Server's Terminal.
    - Handles Socket Communication And Closes The Socket Before Exiting.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
