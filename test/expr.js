class Dog {
	function constructor() {
	}

	function bark() {
	}
}

class Human {
	function constructor() {
		this.age = 56;
		this.pet = new Dog();
	}
}

class Main {
	function main() {
		var h = new Human();
		var age = h.age;

		var p = h.pet.bark;
		p(1);
	}
}