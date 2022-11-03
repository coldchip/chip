class Client {
	function constructor(fd) {
		this.fd = fd;
	}

	function read() {
		read(this.fd);
	}

	function write(data) {
		write(this.fd, data);
	}

	function close() {
		close(this.fd);
	}
}

class Socket {
	function constructor(ip, port) {
		this.ip = ip;
		this.port = port;
		this.socket = socket();
	}

	function bind() {
		bind(this.socket, this.ip, this.port);
	}

	function accept() {
		var fd = accept(this.socket);

		return new Client(fd);
	}
}

class Main {
	function main() {
		var s = new Socket("0.0.0.0", 19132);
		s.bind();

		while(1) {
			var c = s.accept();

			print("new connection\n");

			c.read();
			c.write("HTTP/1.0 200 OK\r\nContent-Length: 5\r\n\r\nHello");
			c.close();
		}

		return 0;
	}
}