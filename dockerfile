FROM ubuntu:16.04

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install g++ valgrind -y
RUN apt-get update && apt-get install make

# docker build -t memory-test:0.1 
# docker run -ti -v $(pwd):/test memory-test:0.1 bash -c "cd /test/; make && valgrind --leak-check=full ./ircserv 8888 123; rm -f ./ircserv"