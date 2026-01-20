FROM ubuntu:20.04

RUN apt-get update && apt-get install -y gcc-9 g++-9 make

WORKDIR /app

COPY . .

RUN make

CMD ["bash"]