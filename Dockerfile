FROM alpine:latest

COPY . /src/mp1

WORKDIR /src/mp1

RUN apk add --no-cache g++ gcc make cmake git nano

RUN make


