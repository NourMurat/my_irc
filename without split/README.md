# IRC
_____________________________________________________________________________________________

## - To do the Server you have to know the following steps:

 ### non-blocking file descriptors:
   non-blocking file descriptors help your computer do multiple tasks without getting stuck waiting for each task to be done.

### How to use non-blocking file descriptors :
1) Open a File Descriptor with socket().
2) Set Non-blocking Mode with fcntl().
3) Use Non-blocking Operations with read(), write(), recv(), send().

### the equivalent of the poll() function:
   The select() function is often used in C/C++98 to achieve similar I/O multiplexing capabilities as poll().
   It helps you to listens for activity on different (fd) and tells you when there's something happening.

### TCP/IP:
   (Transmission Control Protocol/Internet Protocol) is a set of communication protocols that computers use to talk to each other over a network.
   TCP and IP work like a team to make sure your data gets from one computer to another in an accurate way.

   TCP it has the content of the message and IP it has the address.

_____________________________________________________________________________________________
 ## Functions Explanation :
 ____________________________________________________________________________________________
 
 1) ***poll() :*** It helps your computer wait for things to happen without using too much energy checking all the time.
    when you're using recv() or send() to read or write data with non-blocking file descriptors poll() to wait until there's something to read or write.

        #include <poll.h>
 
        int poll(struct pollfd *fds, nfds_t nfds, int timeout);

   * **fds :** An array of struct pollfd structures, each representing a file descriptor that you want to monitor and the events you're interested in (such as reading, writing, or errors).
   * **struct pollfd :** is a specific type of struct used with the poll() function. It holds information about a file descriptor that you want to monitor and the events you're interested in for that descriptor. 
   
    > struct pollfd {
    > 
    > int   fd;
    > 
    > short events; // pollin, pollout and pollerr
    > 
    > short revents;
    };
 
   * **nfds :** The number of file descriptors in the array.
   * **timeout :**  The maximum amount of time to wait for an event to occur, It's specified in milliseconds.-1 for an indefinite wait, 0 for a non-blocking check, or a positive value for a specific timeout.
  
 > **Difference between poll() and pollfd()():**
"poll" is the action you take to listen for activity on many compurters at once, and "pollfd" is the helper that helps you do that by keeping track of each computer status.

**POLLIN:** This symbolic constant represents the presence of data available for reading within a given file descriptor. It signifies that the file descriptor is ready for input operations and can be read without blocking.

**"revents,"** you can see the current status of each door you're listening to and decide what action to take based on what's happening. It's like getting updates from your helper about what's happening at the doors you care about.

-------------------------

2) ***socket() :*** create a communication endpoint, or a socket, that allows different computer processes to send and receive data over a network.
   
       #include <sys/types.h>
       #include <sys/socket.h>
      
       int socket(int domain, int type, int protocol);
   * **domain:** Specifies the communication domain, such as AF_INET for IPv4 or AF_INET6.
   * **type:** Specifies the socket type, which can be SOCK_STREAM for reliable, connection-oriented communication (like a phone call) or SOCK_DGRAM for connectionless, unreliable              communication (like sending letters).
   * **protocol:** Specifies the protocol to be used, which is usually set to 0 to automatically select the appropriate protocol based on the domain and type.
-------------------------

3) ***fcntl() :*** used to perform various operations on a file descriptor. like change the file's access mode, non-blocking behavior, or to get information about the file.

       #include <fcntl.h>

       int fcntl(int fd, int cmd, ... /* arg */ );
   * **fd :** file descriptor of the file you want to work with.
   * **cmd :** the command you want to give to the file. It tells the function what you want to do, like getting or setting different properties of the file.
   * **arg :** optional argument that you might need for some commands. It gives the function more information about what you want to do.
-------------------------

 4) ***bind() ::*** use to set up the address for their computer so others can connect to it over the network.
         
         #include <sys/types.h>
         #include <sys/socket.h>
         
         int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    * **sockfd :** is the fd of the socket that you want to attach an address to.
    * **addr :** This is where you put the address information.
    * **addrlen :** This is the size of the address structure.
      
      > struct sockaddr_in {
      > 
      > short      sin_family; // Address family (like IPv4 or IPv6)
      > 
      > u_short    sin_port;   // Port number
      > 
      > struct in_addr sin_addr; // IP address
};
-------------------------

5) ***send() :*** send messages to other computers.
   
          #include <sys/types.h>
          #include <sys/socket.h>
          
          ssize_t send(int sockfd, const void *buf, size_t len, int flags);
   
    * **sockfd:** The socket file descriptor that you're using to send the message.
    * **buf:** The message you want to send. It's like the words you want your friend to hear.
    * **len:** The length of the message in bytes. It's like how long your message is.
    * **flags:** Special options for sending. You can usually set this to 0.
  -------------------------

  6) ***listen() :*** helps your computer be ready to accept new connections from other computers.

    #include <sys/types.h>
    #include <sys/socket.h>
     
    int listen(int sockfd, int backlog);
    
   * **sockfd :** fd that you're using to listen for incoming connections.
   * **backlog :** How many new connections your computer can remember at once. It's like how many seats you have at your party.
-------------------------

7) ***recv() :*** is like opening a special mailbox to see if your computer got any messages from other computers.

       #include <sys/types.h>
       #include <sys/socket.h>
       
       ssize_t recv(int sockfd, void *buf, size_t len, int flags);

  * **sockfd :** The socket fd that you're using to receive the message.
  * **buf:** A place where the message will be put.
  * **len:** How much space is in the buffer to hold the message.
  * **flags:** Special options for receiving. You can usually set this to 0.
-------------------------

8) ***htons() :*** -> _Host To Network Short_
    make sure numbers are understood correctly when sending them between computers on a network IP/TCP.

       #include <arpa/inet.h>

       uint16_t htons(uint16_t hostshort);

you would provide it with a 16-bit unsigned integer (like a port number) that you want to convert to network byte order. The function will then return the value in the appropriate byte order for sending over a network.

-------------------------

9) ***accept() :***
    facilitates the process of transitioning from the initial listening state to actively communicating with a client. It encapsulates the mechanics of acknowledging and         
     accommodating incoming connection requests, ultimately facilitating seamless and reliable network-based interactions.

       #include <sys/types.h>
       #include <sys/socket.h>
       
       int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
-------------------------
10) ***struct FindByFD :***
    function object used with std::find_if() to find a user by their file descriptor. It's not a standard function in C++,
    but a custom one that you or someone else defined in your code.
A predicate is a function that returns a boolean value based on some condition.

        struct FindByFD {
          int target_fd;
      
          FindByFD(int fd) : target_fd(fd) {}
      
          bool operator()(const User& user) const {
              return user.fd() == target_fd;
          }
      };



________________________________________________________________________________________________
**Vector**
 - Creating a place (the vector) to store a bunch of "User". This vector will help you keep track of these users.

       #include <vector>
       std::vector<type_name> vector_name;

_______________________________________________________________________________________________

### The Netcat ( nc ) command

command-line utility for reading and writing data between two computer networks. The communication happens using either TCP or UDP

_______________________________________________________________________________________________
