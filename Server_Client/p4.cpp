// p4.cpp

#include <iostream>
#include <fstream>
#include <cstdint>
#include <thread>
#include <signal.h>

// Standard *NIX Headers
#include <unistd.h>
// Socket Functionality
#include <sys/socket.h>
// TCP/IP Protocol Functionaltiy
#include <arpa/inet.h>


// Clients Need An IP Address And A Port Number To Connect To
void client(in_addr_t ip,in_port_t port);

// Servers Pick An Arbitrary Port Number And Reports Its Port
// Number To The User
void server();

// Functions For Parsing IP Addresses And Port Numbers From
// C-Strings
in_addr_t parse_ip(char*   ip_str);
in_port_t parse_port(char* port_str);

/*
  Returns A Socket File Descriptor That Is Connected To The
  Given IP & Port
*/
int connect_to(in_addr_t ip, in_port_t port);

// Returns A Socket Bound To An Arbitrary Port
int arbitrary_socket();

// Returns The Port Of The Socket Referenced By The Input File Descriptor
in_port_t get_port(int socket_fd);



// Postconditions:
//   1.) Return True If socket_fd Is Alive And Error-Free
bool is_socket_valid(int socket_fd) 
{
  
  int error = 0;                   // Grab A Handler For Our Error Code
  socklen_t len = sizeof(error);   // Grab A Handler For Our Error's Length

  /*
    Tracks The Current Socket's Error And Place It In Our error Buffer;
    If getsockopt(...) Doesn't Find An Actual Live Socket It'll Return Non-Zero.
  */
  int retval = getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error, &len);    

  // A Non-Zero Return Is Considered A Error
  if (retval != 0 || error != 0) 
  {
    return false;                  // Socket is invalid                                                      
  }

  
  return true;                     // Socket is valid

}


// Preconditions:
//   1.) buffer Is Intended To Catch All Bytes From Transaction
//   2.) Will Catch Up To length Bytes From Transaction
// Postconditions:
//   1.) Return True If Transaction Was Successful; Else False
//   2.) buffer Will Store Char's Of Transaction
bool recv_all(int socket_fd, void* buffer, size_t length) 
{

  // Is Socket Valid To Communicate Across?
  if (!is_socket_valid(socket_fd)) 
  {
    std::cerr << "Socket Connection Is Invalid..." << std::endl;
    return false;
  }

  // Prep Our Pointer To 8-Bit Chunks
  char* ptr = static_cast<char*>(buffer);
  size_t bytes_received = 0;

  // Keep Collecting Bytes Until We Have Gotten All Them
  while (bytes_received < length) 
  {
    // Is Socket Valid To Communicate Across?
    if (!is_socket_valid(socket_fd)) 
    {
      std::cerr << "Socket Connection Broke Mid-Transaction..." << std::endl;
      return false;
    }

    // Listen On Socket Port For Remaining Bytes
    ssize_t n = recv(socket_fd, ptr + bytes_received, length - bytes_received, 0);
    if (n <= 0) 
    {
      std::cerr << "Empty Socket..." << std::endl;
      return false;
    }

    bytes_received += n;
  }

  // Return True Saying We Got Our Bytes (Buffer Will Now Implicitly Store Our Data)
  return true;

};


// Preconditions:
//   1.) buffer Is Intended To Contain All Bytes For Transaction
//   2.) Will Send Out Up To length Bytes For Transaction
//   3.) buffer Is Inferred To Be Of char* Type (8-Bit Chunks)
// Postconditions:
//   1.) Return True If Transaction Was Successful; Else False
bool send_all(int socket_fd, const void* buffer, size_t length) 
{

  // Is Socket Valid To Communicate Across?
  if (!is_socket_valid(socket_fd)) 
  {
    std::cerr << "Socket Connection Is Invalid..." << std::endl;
    return false;
  }

  // Prep Our Pointer To Send Out 8-bit Chunks
  const char* ptr = static_cast<const char*>(buffer);
  size_t bytes_sent = 0;

  // Keep Sending Bytes Until We Have Sent All Them
  while (bytes_sent < length) 
  {
    // Is Socket Valid To Communicate Across?
    if (!is_socket_valid(socket_fd)) 
    {
      std::cerr << "Socket Connection Broke Mid-Transaction..."  << std::endl;
      return false;
    }

    // Stream On Socket Port With Remaining Bytes
    ssize_t n = send(socket_fd, ptr + bytes_sent, length - bytes_sent, 0);
    if (n <= 0) 
    {
      std::cerr << "Empty Socket..." << std::endl;
      return false;
    }

    bytes_sent += n;
  }
  
  // Return True Saying We Sent All Bytes Successfully
  return true;

};


// Preconditions:
//   1.) serverRequestPath Is The Server File We Wish To Store Into
//   2.) serverRequestPath Is The Server-Relative Path
//   3.) Will Read Standard Input From Caller Thread
//   4.) Transaction Is Sealed With E.O.F (CTRL + D)
//   5.) Function Is Liable For Socket Closure
// Postconditions:
//   1.) Will Close Socket (socket_fd) Before Leaving Function
//   2.) When E.O.F Is Reached, We Send To Receiver The Data To Write To serverRequestPath
//   3.) Exit Code 1 -> serverRequestPath Is Not In Receiver
//   4.) Exit Code 1 -> Writing To File Path Is Failed
//   5.) Exit Code 1 -> Failed Transaction
//   6.) Exit Code 1 -> Socket Failure
//   7.) Exit Code 0 -> Successful Write To Receiver's File
void clientFileStoreServer(const int& socket_fd, const std::string& serverRequestPath)
{

  // Get Sizes Of Passed Streams To Pass ( "store".size() = 5 )
  uint8_t modeMessage_size = 5;
  uint32_t pathMessage_size = serverRequestPath.size();

  // Ensure Our Mode's Message Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &modeMessage_size, sizeof(modeMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Mode's Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, "store", modeMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Path Message's Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &pathMessage_size, sizeof(pathMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Path Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Path Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, serverRequestPath.c_str(), serverRequestPath.size())) 
  {
    std::cerr << " ⨽ Failed To Send Path Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  
  char buffer[1024];        // buffer of 1024-Bytes
  ssize_t bytes;            // Standard Return Type For Standard Input read(...)
  
  // Keep Reading From Standard Input Until We Get A EOF Or Error
  while((bytes = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0)
  {
    // Send Our Buffer's Size
    if (!send_all(socket_fd, &bytes, sizeof(bytes))) 
    {
      std::cerr << " ⨽ Failed To Send Buffer Size To Server..." << std::endl;
      close(socket_fd);
      exit(1);
    }

    // Send The Actual Buffer
    if (!send_all(socket_fd, buffer, bytes))
    {
      std::cerr << " ⨽ Failed To Send Data To Server..." << std::endl;
      close(socket_fd);
      exit(1);
    }
  }

  // Check If Reading From Standard Input Failed
  if (bytes < 0) 
  {
    std::cerr << "Failed To Read From Standard Input..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Send A Zero-Sized Buffer To Indicate The End Of Transaction
  ssize_t buffer_size = 0;
  if (!send_all(socket_fd, &buffer_size, sizeof(buffer_size))) 
  {
    std::cerr << " ⨽ Failed to send End-Of-Transmission Buffer Size To Server..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  uint8_t success;
  // Receive The Result From The Server
  if (!recv_all(socket_fd, &success, sizeof(success))) 
  {
    std::cerr << " ⨽ Failed To Receive Result From Server..." << std::endl;
    exit(1);
  }

  // Close The Connection To Port
  close(socket_fd);

  if (!success) 
  {
    std::cerr << "Writing To Server Failed..." << std::endl;
    exit(1);
  } 
  else 
  {
    std::cerr << "Writing To Server Succeeded..." << std::endl;
    exit(0);
  }

};


// Preconditions:
//   1.) serverRequestPath Is The Server File We Wish To Delete
//   2.) serverRequestPath Is The Server-Relative Path
//   3.) Function Is Liable For Socket Closure
// Postconditions:
//   1.) Will Close Socket (socket_fd) Before Leaving Function
//   2.) Will Attempt To Delete serverRequestPath From Receiver
//   3.) Exit Code 1 -> serverRequestPath Is Not In Receiver
//   4.) Exit Code 1 -> Deletion Of Path Failed
//   5.) Exit Code 1 -> Failed Transaction
//   6.) Exit Code 1 -> Socket Failure
//   7.) Exit Code 0 -> Successful Deletion Of Receiver's File
void clientFileDeleteServer(const int& socket_fd, const std::string& serverRequestPath)
{

  // Get Sizes Of Passed Streams To Pass ( "delete".size() = 6 )
  uint8_t modeMessage_size = 6;
  uint32_t pathMessage_size = serverRequestPath.size();


  // Ensure Our Mode's Message Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &modeMessage_size, sizeof(modeMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Mode's Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, "delete", modeMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }


  // Ensure Our Path's Message Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &pathMessage_size, sizeof(pathMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Path Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Path Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, serverRequestPath.c_str(), serverRequestPath.size())) 
  {
    std::cerr << " ⨽ Failed To Send Path Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  uint8_t temp;

  // Listen To Our Server For The Return Boolean
  if (!recv_all(socket_fd, &temp, sizeof(temp))) 
  {
    std::cerr << " ⨽ Receival From Server Failed..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Close-Up Our Socket Transaction
  close(socket_fd);

  if (static_cast<bool>(temp) == true) 
  {
    std::cout << "File Was Deleted :)" << std::endl;
    exit(0);
  } 
  else 
  {
    std::cout << "File Not Deleted :<(" << std::endl;
    exit(1);
  }

};


// Preconditions:
//   1.) serverRequestPath Is The Server File We Wish To Check For
//   2.) serverRequestPath Is The Server-Relative Path
//   3.) Function Is Liable For Socket Closure
// Postconditions:
//   1.) Will Close Socket (socket_fd) Before Leaving Function
//   2.) Will Look For serverRequest In Receivers Relative Path
//   3.) Exit Code 1 -> serverRequestPath Is Not In Receiver
//   4.) Exit Code 1 -> Failed Transaction
//   5.) Exit Code 1 -> Socket Failure
//   6.) Exit Code 0 -> Found serverRequestPath In Receiver
void clientFileCheckServer(const int& socket_fd, const std::string& serverRequestPath)
{

  // Get Sizes Of Passed Streams To Pass ( "check".size() = 5 )
  uint8_t modeMessage_size = 5;
  uint32_t pathMessage_size = serverRequestPath.size();


  // Ensure Our Mode's Message Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &modeMessage_size, sizeof(modeMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Mode's Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, "check", modeMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Path's Message Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &pathMessage_size, sizeof(pathMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Path Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Path Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, serverRequestPath.c_str(), serverRequestPath.size())) 
  {
    std::cerr << " ⨽ Failed To Send Path Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  uint8_t temp;

  // Listen To Our Server For The Return Boolean
  if (!recv_all(socket_fd, &temp, sizeof(temp))) 
  {
    std::cerr << " ⨽ Receival From Server Failed..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Close-Up Our Socket Transaction
  close(socket_fd);

  if (static_cast<bool>(temp) == true) 
  {
    std::cout << "File Path Is Inside Our Server :)" << std::endl;
    exit(0);
  } 
  else 
  {
    std::cout << "File Path Is Not Inside Our Server :<(" << std::endl;
    exit(1);
  }

};


// Preconditions:
//   1.) serverRequestPath Is The Server File We Wish To Load From
//   2.) serverRequestPath Is The Server-Relative Path
//   3.) Function Is Liable For Socket Closure
// Postconditions:
//   1.) Will Close Socket (socket_fd) Before Leaving Function
//   2.) Will Look For serverRequest In Receivers Relative Path
//   3.) Exit Code 1 -> serverRequestPath Is Not In Receiver
//   4.) Exit Code 1 -> Failed Transaction
//   5.) Exit Code 1 -> Socket Failure
//   6.) Exit Code 0 -> Found serverRequestPath In Receiver And Given To Caller
void clientFileLoadServer(const int& socket_fd, const std::string& serverRequestPath)
{

  // Get Sizes Of Passed Streams To Pass ( "load".size() = 4 )
  uint8_t modeMessage_size = 4;
  uint32_t pathMessage_size = serverRequestPath.size();

  // Ensure Our Mode's Message Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &modeMessage_size, sizeof(modeMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Mode's Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, "load", modeMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }


  // Ensure Our Path's Message Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &pathMessage_size, sizeof(pathMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Path Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Path Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, serverRequestPath.c_str(), serverRequestPath.size())) 
  {
    std::cerr << " ⨽ Failed To Send Path Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Grab File Size From Our Receiver (Max Sized Files Of 4GB)
  uint32_t file_size;
  if (!recv_all(socket_fd, &file_size, sizeof(file_size)))
  {
    std::cerr << " ⨽ Failed To Receive File Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // If Our File Size Is Zero, Could Be Empty Or Non-Existent
  if (file_size == 0) 
  {
    // Wait For Accompanying Byte Stating If Empty, Or Non Existent File
    uint8_t nonExistent;
    if (!recv_all(socket_fd, &nonExistent, sizeof(nonExistent)))
    {
      std::cerr << " ⨽ Failed To Receive If File Is Empty Or Non-Existent..." << std::endl;
      close(socket_fd);
      exit(1);
    }

    if(nonExistent == 1)
    {
      std::cerr << "File Doesn't Exist..." << std::endl;
      close(socket_fd);
      exit(1);
    }
  }


  // Grab The File's Contents
  char* file_contents = new char[file_size + 1];
  if (!recv_all(socket_fd, file_contents, file_size))
  {
    std::cerr << " ⨽ Failed To Receive File Contents..." << std::endl;
    delete[] file_contents;
    close(socket_fd);
    exit(1);
  }

  file_contents[file_size] = '\0'; // Null-terminate the string

  // Print the file contents
  std::cout << file_contents;

  delete[] file_contents;
  close(socket_fd);
  exit(0);

};


// desc : Parses a string to an ip address
// pre  : ip_str points to a valid c_string
// post : Returns the parsed ip or throws a runtime error
in_addr_t parse_ip(char*   ip_str) 
{
  // The 'in_addr_t' type represents an ip address, and can be
  // parsed from a string using 'inet_addr'
  in_addr_t ip_addr = inet_addr(ip_str);
  // If the parsing failed, the INADDR_NONE value will be produced
  if (ip_addr == INADDR_NONE) 
  {
    throw std::runtime_error("Failed to convert input ip address.");     
  }
  return ip_addr;
};

// desc : Parses a string to a port number
// pre  : port_str points to a valid c_string
// post : Returns the parsed port number or throws a runtime exception
in_port_t parse_port(char* port_str) 
{
  // Parse port number from argument
  in_port_t port = atoi(port_str);
  // 'atoi' returns zero on error. Port zero is not a 'real' port.
  if(port == 0) 
  {
    throw std::runtime_error("Invalid port argument.");     
  }
  return port;
};


// desc : Returns a tcp/ip socket
// pre  : None
// post : Returns a tcp/ip socket or throws a runtime exception
int make_tcp_ip_socket() 
{
  // Make a socket, which is a special type of file that acts as a
  // holding area for sent/recieved data.
  //
  //  - PF_INET means the Port Family is for InterNET
  //  - SOCK_STREAM indicates it should use the TCP protocol
  int socket_fd = socket(PF_INET,SOCK_STREAM,0);
  // If the fd is negative, socket allocation failed
  if(socket_fd < 0)
  {
    throw std::runtime_error("Could not allocate socket.");
  }
  return socket_fd;
};


// desc : Returns a socket connected to the given ip address and
//        port number
// pre  : ip is a valid ip address. port is a valid port number
// post : If an error is encountered, a runtime exception is thrown
int connect_to(in_addr_t ip, in_port_t port) 
{

  // Set up socket address data structure, which we will use
  // to tell the OS what we want to do with our socket
  sockaddr_in socket_addr;
  // AF_INET means the Address Family is for InterNET
  socket_addr.sin_family = AF_INET;
  // Set the ip address to connect to
  socket_addr.sin_addr.s_addr = ip;
  // Set the port to connect to
  // htons converts endianness from host to network
  socket_addr.sin_port = htons(port);

  // Make socket to connect through
  int socket_fd = make_tcp_ip_socket();

  // Tell the OS we want to connect to the ip/port indicated by
  // socket_addr through the socket represented by the file
  // descriptor 'socket_fd'
  int status = connect(socket_fd,(sockaddr*)&socket_addr,sizeof(socket_addr));
  // If output is negative, the connection was not successful.
  if(status < 0) 
  {
    // Make sure socket get cleaned up
    close(socket_fd);
    throw std::runtime_error("Connection failed.");
  }
  return socket_fd;

};

// desc : Returns a socket bound to an arbitrary port
// pre  : None
// post : If an error is returned, a runtime exception is thrown
int arbitrary_socket() 
{

  // Set up socket address data structure, which we will use
  // to tell the OS what we want to do with our socket
  sockaddr_in socket_addr;
  // AF_INET means the Address Family is for InterNET
  socket_addr.sin_family = AF_INET;
  // Indicate we are willing to connect with any ip address
  socket_addr.sin_addr.s_addr = INADDR_ANY;
  // Use zero-valued port to tell OS to pick any available
  // port number
  socket_addr.sin_port = 0;

  // Make a socket to listen through
  int socket_fd = make_tcp_ip_socket();

  // Bind socket to an arbitrary available port
  int status = bind(socket_fd, (struct sockaddr *) &socket_addr, sizeof(sockaddr_in));

  if(status < 0) 
  {
    throw std::runtime_error("Binding failed.");
  }
  return socket_fd;

};


// desc : Returns the port that the provided file descriptor's
//        socket is bound to
// pre  : The provided socket file descriptor is valid
// post : If an error is encountered, a runtime exception is thrown
in_port_t get_port(int socket_fd) 
{

  // A receptacle for the syscall to write the port number
  sockaddr_in socket_addr;
  // You need to supply the size of the receptacle through
  // a pointer. This seems rather silly, but is onetheless necessary.
  socklen_t socklen = sizeof(sockaddr_in);
  // Get the "name" (aka port number) of socket
  int status = getsockname(socket_fd, (struct sockaddr *) &socket_addr, &socklen);

  if (status < 0) 
  {
    throw std::runtime_error("Failed to find socket's port number.");
  }
  // Flip endianness from network to host
  return ntohs(socket_addr.sin_port);

};


// desc : Connects to server and sends a one-line message
// pre  : ip is a vaid ip address and port is a valid port number
// post : If an error is encountered, a runtime exception is thrown
void client(in_addr_t ip, in_port_t port) 
{
  /*
    Attempt To Connect To Server Through A New Socket.
    Return Early If This Fails.
  */
  int socket_fd = connect_to(ip, port);
  if(socket_fd < 0) 
  {
    return;
  }

  // Get A One-Line Message From The User
  std::string message;
  std::getline(std::cin,message);


  // Get Sizes Of Passed Streams To Pass ( "client".size() = 5 )
  uint8_t modeMessage_size = 6;

  // Transmit Message Size Ahead Of Time, So Server Can
  // Pre-Allocate Message Storage
  uint32_t message_size = message.size();

  // Ensure Our Mode's Message Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &modeMessage_size, sizeof(modeMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Mode's Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, "client", modeMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Send Mode Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }


  // Ensure Our Message's Size Is Properly Passed To Our Server
  if (!send_all(socket_fd, &message_size, sizeof(message_size))) 
  {
    std::cerr << " ⨽ Failed To Send Path Message Size..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // Ensure Our Message Is Properly Passed To Our Server
  if (!send_all(socket_fd, message.c_str(), message.size())) 
  {
    std::cerr << " ⨽ Failed To Send Path Message..." << std::endl;
    close(socket_fd);
    exit(1);
  }

  // close connection
  close(socket_fd);
  exit(0);

};


// Preconditions:
//   1.) Function Is Liable For Socket Closure
//   2.) Expects Receival Of Relative Path On Server To Look For
// Postconditions:
//   1.) Will Close Socket (connection_fd) Before Leaving Function
//   2.) Will Look For pathMessage In Relative Path
//   3.) Exit Code 1 -> pathMessage Is Not Found
//   4.) Exit Code 1 -> Failed Transaction
//   5.) Exit Code 1 -> Socket Failure
//   6.) Exit Code 0 -> Found pathMessage And Told Receiver
void fileCheckServer(int connection_fd) 
{

  // Retrieve Our Path's Message Size
  uint32_t pathMessage_size;
  if (!recv_all(connection_fd, &pathMessage_size, sizeof(pathMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message Size..." << std::endl;
    close(connection_fd);
    return;
  }

  // Now Grab Our Path Characters
  char* pathMessage = new char[pathMessage_size + 1];
  if (!recv_all(connection_fd, pathMessage, pathMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message..." << std::endl;

    // Inform User We Didn't Find File As We Fail Out
    uint8_t exists = 0;
    if (!send_all(connection_fd, &exists, sizeof(exists))) 
    {
      std::cerr << " ⨽ Failed To Send Exit Code To Client..." << std::endl;
    }

    delete[] pathMessage;
    close(connection_fd);
    return;
  }

  // Set Our End Of String Char At End Of Path String
  pathMessage[pathMessage_size] = '\0';
  std::string filePath(pathMessage);
  delete[] pathMessage;

  // If our File Exists, Set To 1, Else 0.
  uint8_t exists = ((std::ifstream(filePath)) ? 1 : 0);

  if (!send_all(connection_fd, &exists, sizeof(exists))) 
  {
    std::cerr << " ⨽ Failed To Send Result To Client..." << std::endl;
  }

  // Close Up Our Handshake With Client
  close(connection_fd);
  return;

}


// Preconditions:
//   1.) Function Is Liable For Socket Closure
//   2.) Expects Receival Of Relative Path On Server To Look For
// Postconditions:
//   1.) Will Close Socket (connection_fd) Before Leaving Function
//   2.) Will Look For pathMessage In Relative Path And Pass Character Stream To Receiver
//   3.) Exit Code 1 -> pathMessage Is Not Found
//   4.) Exit Code 1 -> Failed Transaction
//   5.) Exit Code 1 -> Socket Failure
//   6.) Exit Code 0 -> Found pathMessage And Gave Receiver Character Stream
void fileLoadServer(int connection_fd) 
{

  // Retrieve Our Path Message's Size
  uint32_t pathMessage_size;
  if (!recv_all(connection_fd, &pathMessage_size, sizeof(pathMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message Size..." << std::endl;
    close(connection_fd);
    return;
  }

  // Now Grab Our Path Characters
  char* pathMessage = new char[pathMessage_size + 1];
  if (!recv_all(connection_fd, pathMessage, pathMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message..." << std::endl;
    delete[] pathMessage;
    close(connection_fd);
    return;
  }

  // Set Our End Of String Char At End Of Path String
  pathMessage[pathMessage_size] = '\0';
  std::string filePath(pathMessage);
  delete[] pathMessage;

  // Open Our File At End
  std::ifstream file(filePath, std::ios::binary | std::ios::ate);

  // If File Can Be Opened, Grab Size Of File
  std::streamsize size = file.tellg();

  // If Our File Cannot Be Opened; Tell Our Client
  if (size == -1)
  {
    uint32_t file_size = 0;
    uint8_t  nonExistent = 1;

    if(!send_all(connection_fd, &file_size, sizeof(file_size)))
    {
      std::cerr << " ⨽ Couldn't Inform Receivee Non-Openable..." << std::endl;
    }
    if(!send_all(connection_fd, &nonExistent, sizeof(nonExistent)))
    {
      std::cerr << " ⨽ Couldn't Inform Receivee Non-Openable..." << std::endl;
    }

    close(connection_fd);
    return;
  }
  else if (size == 0)
  {
    uint32_t file_size = 0;
    uint8_t  nonExistent = 0;

    if(!send_all(connection_fd, &file_size, sizeof(file_size)))
    {
      std::cerr << " ⨽ Couldn't Inform Receivee Non-Openable..." << std::endl;
    }
    if(!send_all(connection_fd, &nonExistent, sizeof(nonExistent)))
    {
      std::cerr << " ⨽ Couldn't Inform Receivee Non-Openable..." << std::endl;
    }

    close(connection_fd);
    return;
  }
  
  // Go Back To Start Of File After Getting Size
  file.seekg(0, std::ios::beg);

  // Read The File's Contents
  char* buffer = new char[size];

  if (file.read(buffer, size)) 
  {
    // Max Size Of 4GB File
    uint32_t file_size = static_cast<uint32_t>(size);
    if (!send_all(connection_fd, &file_size, sizeof(file_size)))
    {
      std::cerr << " ⨽ Couldn't Inform Receivee Of File Size..." << std::endl;
    }
    if (!send_all(connection_fd, buffer, size))
    {
      std::cerr << " ⨽ Couldn't Inform Receivee Of File Contents..." << std::endl;
    }
  }

  delete[] buffer;
  file.close();
  close(connection_fd);
  return;

}


// Preconditions:
//   1.) Function Is Liable For Socket Closure
//   2.) Expects Receival Of Relative Path On Server To Look For
// Postconditions:
//   1.) Will Close Socket (connection_fd) Before Leaving Function
//   2.) Will Look For pathMessage In Relative Path And Append Character Stream From Receiver Into It
//   3.) Exit Code 1 -> pathMessage Is Not Found
//   4.) Exit Code 1 -> Failed Transaction
//   5.) Exit Code 1 -> Socket Failure
//   6.) Exit Code 0 -> Found pathMessage And Receiver Character Stream Emplaced In File Directory
void fileStoreServer(int connection_fd) 
{

  // Retrieve Our Path's Message Size
  uint32_t pathMessage_size;
  if (!recv_all(connection_fd, &pathMessage_size, sizeof(pathMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message Size..." << std::endl;
    close(connection_fd);
    return;
  }

  // Now Grab Our Path Characters
  char* pathMessage = new char[pathMessage_size + 1];
  if (!recv_all(connection_fd, pathMessage, pathMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message..." << std::endl;
    delete[] pathMessage;
    close(connection_fd);
    return;
  }

  // Set Our End Of String Char At End Of Path String
  pathMessage[pathMessage_size] = '\0';
  std::string filePath(pathMessage);
  delete[] pathMessage;

  // Open The File For Write
  std::ofstream file(filePath, std::ios::binary);
  
  // Loop To Receive Data And Write To File
  while (true) 
  {

    // Standard Return Type For Clients Standard Input read(...)
    ssize_t bufferSize;
    if (!recv_all(connection_fd, &bufferSize, sizeof(bufferSize))) 
    {
      std::cerr << " ⨽ Failed To Receive Buffer Message Size..." << std::endl;
      break;
    }

    // If Buffer Size Is Zero, End Of Transmission
    if (bufferSize == 0)
    {
      break;
    }

    // Grab All Data From Client's Standard Input
    char* buffer = new char[bufferSize];
    if (!recv_all(connection_fd, buffer, bufferSize)) 
    {
      std::cerr << " ⨽ Failed To Receive Buffer..." << std::endl;
      delete[] buffer;
      break;
    }

    // Write The Data To Our File
    file.write(buffer, bufferSize);
    delete[] buffer;

    if (file.fail()) 
    {
      std::cerr << "Failed To Write To File..." << std::endl;
      break;
    } 

  }

  // Check If Writing Failed
  uint8_t success = (file.fail() ? 0 : 1);
  if (!send_all(connection_fd, &success, sizeof(success)))
  {
    std::cerr << " ⨽ Couldn't Inform Receivee Of Success Boolean..." << std::endl;
  }

  file.close();
  close(connection_fd);
  return;

}


// Preconditions:
//   1.) Function Is Liable For Socket Closure
//   2.) Expects Receival Of Relative Path On Server To Look For
// Postconditions:
//   1.) Will Close Socket (connection_fd) Before Leaving Function
//   2.) Will Look For pathMessage In Relative Path And Delete It If Found; Informing Receiver True Or False
//   3.) Exit Code 1 -> pathMessage Is Not Found
//   4.) Exit Code 1 -> Failed Transaction
//   5.) Exit Code 1 -> Socket Failure
//   6.) Exit Code 0 -> Found pathMessage And Told Receiver
void fileDeleteServer(int connection_fd) 
{

  // Retrieve Our Path Message's Size
  uint32_t pathMessage_size;
  if (!recv_all(connection_fd, &pathMessage_size, sizeof(pathMessage_size))) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message Size..." << std::endl;
    close(connection_fd);
    return;
  }

  // Now Grab Our Path Characters
  char* pathMessage = new char[pathMessage_size + 1];
  if (!recv_all(connection_fd, pathMessage, pathMessage_size)) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message..." << std::endl;
    delete[] pathMessage;
    close(connection_fd);
    return;
  }

  // Set Our End Of String Char At End Of Path String
  pathMessage[pathMessage_size] = '\0';
  std::string filePath(pathMessage);
  delete[] pathMessage;

  /*
    If We Can Remove File Path, Send 1, Else, 0 
    remove(...) Under The Hood Will Unlink, So Currently Opened Transactions With The fd Will Still Work
  */
  uint8_t result = (remove(filePath.c_str()) == 0) ? 1 : 0;
  send_all(connection_fd, &result, sizeof(result));
  close(connection_fd);
  return;
}


// Preconditions:
//   1.) Function Is Liable For Socket Closure
//   2.) Expects Receival Of Relative Path On Server To Look For
// Postconditions:
//   1.) Will Close Socket (connection_fd) Before Leaving Function
//   2.) Will Print The Characters Sent By Client In Caller's Terminal
//   3.) Exit Code 1 -> Failed Transaction
//   4.) Exit Code 1 -> Socket Failure
//   5.) Exit Code 0 -> Successfully Printed To Terminal The Clients Message
void clientCommunicationServer(int connection_fd)
{

  // Retrieve Our Message's Size
  uint32_t message_size;
  if (!recv_all(connection_fd, &message_size, sizeof(message_size))) 
  {
    std::cerr << " ⨽ Failed To Receive Client Message Size..." << std::endl;
    close(connection_fd);
    return;
  }

  // Now Grab Our Message's Characters
  char* clientBuffer = new char[message_size];
  if (!recv_all(connection_fd, clientBuffer, message_size)) 
  {
    std::cerr << " ⨽ Failed To Receive Path Message..." << std::endl;
    delete[] clientBuffer;
    close(connection_fd);
    return;
  }

  // Set Our End Of String Char At End Of Path String
  std::string clientMessage(clientBuffer);
  delete[] clientBuffer;

  std::cout << clientMessage << std::endl;
  close(connection_fd);
  return;

};

// Place It Lower For Restricted Scope
int server_socket_fd;

void server_cleanup_handler(int) 
{

  // Close Our Server Socket To Shut Everything Down
  std::cout << "\nServer Shutting Down..." << std::endl;
  close(server_socket_fd);
  exit(0);

}

// desc : Listens on an arbitrary port (announced through stdout)
//        for connections, recieving messages as 32-bit string
//        lengths followed my a sequence of characters matching that
//        length.
// pre  : None
// post : If a listening socket cannot be set up, a runtime exception
//        is thrown. If a connection fails or disconnects early, the
//        error is announced but the server continues operation.
void server() 
{

  // Tell Server That It Will Use This Handler For A SIGINT
  signal(SIGINT, server_cleanup_handler);

  // Make An Arbitrary Socket To Listen Through
  server_socket_fd = arbitrary_socket();
  int port = get_port(server_socket_fd);
  std::cout << "Setup Server At Port "<< port << std::endl;

  /*
    Tell OS To Start Listening At Port For Its Set Protocol
    (In This Case, TCP IP), With A Waiting Queue Of Size 1.
    Additional Connection Requests That Cannot Fit In The
    Queue Will Be Refused.
  */
  int status = listen(server_socket_fd, 1);
  if( status < 0 ) 
  {
    std::cout << "Listening Failed." << std::endl;
    return;
  }
  /*
    A Receptacle To Store Information About Sockets We Will
    Accept Connections Through
  */
  sockaddr_storage storage;
  socklen_t socket_len = sizeof(sockaddr_storage);

  // Pre-Define Our Repeatedly Used Client-Provided Mode Of Action
  uint8_t modeMessage_size;
  char* modeMessage;

  while(true)
  {

    // Wait Until A Connection Request Arrives At The Server Entry-Point Socket
    int connection_fd = accept(server_socket_fd, (sockaddr*)&storage, &socket_len);

    /*
      Ignore Failures. Clients Can Do Weird Things Sometimes, And
      It's Not A Good Idea To Simply Crash Whenever That Happens.
    */
    if(connection_fd < 0)
    {
      std::cout << "Could Not Accept Connection." << std::endl;
      continue;
    }

    if (!recv_all(connection_fd, &modeMessage_size, sizeof(modeMessage_size))) 
    {
      std::cerr << " ⨽ Failed To Receive Mode Message Size." << std::endl;
      close(connection_fd);
      continue;
    }

    modeMessage = new char[modeMessage_size + 1];
    if (!recv_all(connection_fd, modeMessage, modeMessage_size)) 
    {
      std::cerr << " ⨽ Failed To Receive Mode Message." << std::endl;
      delete[] modeMessage;
      close(connection_fd);
      continue;
    }

    modeMessage[modeMessage_size] = '\0';
    std::string mode(modeMessage);
    delete[] modeMessage;


    /*
      Determine Which Routine Our Worker Thread Will Do...
      Threads Will Cleanup Their Socket Entry Points Internally.
    */
    if (mode == "check") 
    {
      std::thread(fileCheckServer, connection_fd).detach();
    } 
    else if (mode == "load") 
    {
      std::thread(fileLoadServer, connection_fd).detach();
    } 
    else if (mode == "store") 
    {
      std::thread(fileStoreServer, connection_fd).detach();
    }
    else if (mode == "delete") 
    {
      std::thread(fileDeleteServer, connection_fd).detach();
    }
    else if (mode == "client")
    {
      std::thread(clientCommunicationServer, connection_fd).detach();
    }
    else
    {
      std::cout << "Invalid Transaction Type For Server..." << std::endl;
      close(connection_fd);
    }

  }

};


int main(int argc, char *argv[]) 
{

  // First Argument Must Be Supplied To Indicate Mode (client / server)
  if(argc < 2)
  {
    std::cout << "Usage: p4 [mode] [options ...]" << std::endl;
    exit(1);
  }

  // Switch arg Handling And Execution Based Upon The Mode
  std::string mode = argv[1];
  if (mode == "server")
  {
    /*
      Servers Need No Arguments, And Simply Report The Port And
      IP Address They End Up Having
    */
    if(argc != 2)
    {
      std::cout << "Usage: p4 server" << std::endl;
      exit(1);
    }

    server();
  } 
  else if (mode == "client") 
  {
    // Clients Need A IP Address And Port To Connect To
    if(argc != 4)
    {
      std::cout << "Usage: p4 client <ip> <port>" << std::endl;
      exit(1);
    }

    /*
      Parse Functions Will Kill The Program Early If The
      Address Or Port Arguments Are Invalid.
    */
    client(parse_ip(argv[2]), parse_port(argv[3]));
  } 
  else if (mode == "check")
  {
    // Ensure We Got All Our Neccessary Args
    if(argc != 5)
    {
      std::cout << "Usage: p4 check <ip> <port> <path>" << std::endl;
      exit(1);
    }

    // Attempt To Connect To The User Provided IP and Port #'s
    int socket_fd = connect_to(parse_ip(argv[2]), parse_port(argv[3]));
    if (socket_fd < 0) 
    {
      std::cerr << "Failed To Connect To Server..." << std::endl;
      exit(1);
    }

    // Execute Our Check Routine After Doing Our Input Sanitization
    clientFileCheckServer(socket_fd, std::string(argv[4]));
  } 
  else if (mode == "load")
  {
    // Ensure We Got All Our Neccessary Args
    if(argc != 5)
    {
      std::cout << "Usage: p4 load <ip> <port> <path>" << std::endl;
      exit(1);
    }

    // Attempt To Connect To The User Provided IP and Port #'s
    int socket_fd = connect_to(parse_ip(argv[2]), parse_port(argv[3]));
    if (socket_fd < 0) 
    {
      std::cerr << "Failed To Connect To Server..." << std::endl;
      exit(1);
    }

    // Execute Our Check Routine After Doing Our Input Sanitization
    clientFileLoadServer(socket_fd,std::string (argv[4]));
  }
  else if (mode == "store")
  {
    // Ensure We Got All Our Neccessary Args
    if(argc != 5)
    {
      std::cout << "Usage: p4 store <ip> <port> <path>" << std::endl;
      exit(1);
    }

    // Attempt To Connect To The User Provided IP and Port #'s
    int socket_fd = connect_to(parse_ip(argv[2]), parse_port(argv[3]));
    if (socket_fd < 0) 
    {
      std::cerr << "Failed To Connect To Server..." << std::endl;
      exit(1);
    }

    // Execute Our Check Routine After Doing Our Input Sanitization
    clientFileStoreServer(socket_fd,std::string (argv[4]));  
  }
  else if (mode == "delete")
  {
    // Ensure We Got All Our Neccessary Args
    if(argc != 5)
    {
      std::cout << "Usage: p4 store <ip> <port> <path>" << std::endl;
      exit(1);
    }

    // Attempt To Connect To The User Provided IP and Port #'s
    int socket_fd = connect_to(parse_ip(argv[2]), parse_port(argv[3]));
    if (socket_fd < 0) 
    {
      std::cerr << "Failed To Connect To Server..." << std::endl;
      exit(1);
    }

    // Execute Our Check Routine After Doing Our Input Sanitization
    clientFileDeleteServer(socket_fd, std::string (argv[4]));
  }
  else 
  {
    std::cout << "Mode '" << mode << "' Not Recognized" << std::endl;
  }
  
  exit(0);

};
