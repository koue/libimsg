# FreeBSD port of OpenBSD imsg

The imsg functions provide a simple mechanism for communication between local
processes using sockets. Each transmitted message is guaranteed to be presented
to the receiving program whole. They are commonly used in privilege separated
processes, where processes with different rights are required to cooperate.

http://cvsweb.openbsd.org/cgi-bin/cvsweb/src/lib/libutil/

## Installation

`make && make install`

## Usage

`cd test/ && make && make test`
