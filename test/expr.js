class Human {
	function constructor(name, age) {
		this.name = name;
		this.age = age;
	}

	function setAge(age) {
		this.age = age;
	}

	function getAge() {
		return this.age;
	}

	function lmao() {

	}

	function sin(number) {
		var res = 0;
		var pow = number;
		var fact = 1;

		var i = 0;
		while(i < 6) {
			res = res + (pow / fact);
			pow = pow * ((0-1) * number * number);
			fact = fact * ((2 * (i + 1)) * (2 * (i + 1) + 1));
			i = i + 1;
		}

		return res;
	}
}

class ListNode {
	function constructor(data, prev, next) {
		this.data = data;
		this.prev = prev;
		this.next = next;
	}

	function setNext(n) {
		this.next = n;
	}

	function getNext() {
		return this.next;
	}
}

class List {
	function constructor() {
		this.head = new ListNode(0, 0, 0);
		this.current = this.head;
	}

	function push(data) {
		var n = new ListNode(data, 0, 0);
		this.current.setNext(n);
	}
}

class Main {
	function main() {
		var h = new Human("Ryan" + " ColdChip", 1);

		var result = 0.0;

		var i = 0;

		while(i < h.getAge()) {
			var result = result + h.sin(i);
			i = i + 1;
		}

		var result = "The result is " + result + " degs";

		var l = new List();
		l.push(new Human("a", 1));
		l.push(new Human("g", 1));
		l.push(new Human("j", 1));
		l.push(new Human("u", 1));

		var n = l.head.getNext();

		while(8 < 9) {
			var kkk = n.data;
			n = n.getNext();
		}

		return 0;
	}
}