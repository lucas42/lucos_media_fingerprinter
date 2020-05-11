FROM gcc:10
WORKDIR /usr/src/myapp

RUN echo "deb http://archive.debian.org/debian/ wheezy main" > /etc/apt/sources.list.d/wheezy.list
RUN apt-get update
RUN apt-get install -y libavformat-dev=6:0.8.17-2 libavcodec-dev=6:0.8.17-2 libavutil-dev=6:0.8.17-2 libchromaprint-dev=0.6-2

COPY . .

RUN ./build.sh
CMD ["./fingerprinter"]