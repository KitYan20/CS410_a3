# CS410_a3

### Group Members
Kit Yan and Mario Hysa

### Testing webserv
To test out the webserver, make sure you have a web browser service to use for client side rendering. First you need to get IP network address. You can use this this command on the terminal to get it.
```bash
hostname -I
```
After that, you can test out with any port you want. We used 8080 as our port for the convienence to host our webserver. To run the webserver, make sure you have compiled the program using this command:
```bash
make webserv
```
You can launch the webserver by taking a port number as an argument and launch the webserver on a web browser.
```bash
./webserv 8080
```
Type your specific IP network address into the url box:
http:IP Address:8080

Once you have the webserver up and running, the first thing you will see on screen is the list of the current directory you are in.
#### Testing out all the basic client request
To test out different client request, you just need to add a "/" at the end of the port number and type in the type of request you want. For the CGI script, it only supports Python and Matplotlib when handling Python Script in a CGI file and handling dynamically-created image.
#### Testing HTML File
```
http:IP Address:8080/test.html
```
#### Testing Static Image (JPG or Gif)
```
http:IP Address:8080/test.jpg <-- (Replace the extension with .gif if you want to view a gif)
```
#### Testing CGI Script executing basic shell commands
```
http:IP Address:8080/test.cgi
```
#### Testing Python Script in a CGI file to process raw data and render it to HTML
```
http:IP Address:8080/python.cgi
```
#### Testing dynamically-created image using matplotlib on the server
```
http:IP Address:8080/histogram.cgi?directory=. <--(Replace "." to change the directory)
```
Specifically for request a dynamically image, it would run the python program "my-histogram.py" which will traverse through a directory to list the different types of files in that directory taking a directory as an argument from the client and would parse through the url to get the directory. What histogram.cgi would do is to run a subprocess to execute the "my-histogram.py" program which will create a jpg histogram of the list of files in the directory. Then, it will just have print statements of HTML code which the server will send it back as HTML code to render on the client side to view.

### Physical Computing

#### Testing the Arduino Hardware
Our game is called the Tapping Game where the primary goal is to turn on all the lights under 60 seconds. After the game is over, you will recieve a score back from Arduino to indicate how many rounds you got up to under 60 seconds. To start the game on the Webserver, you run this command on the web:
```
http:IP Address:8080/start
```
There will be a timer to indicate how many seconds you have left on time before the game is over.
