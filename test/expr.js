class Human {
	function constructor(name) {
		this.name = name;
	}

	function setAge(age) {
		this.age = age;
	}

	function getAge() {
		return this.age;
	}
}

class Main {
	function main() {
		var h = new Human("Ryan" + " ColdChip");
		h.setAge(32);

		

		return 0;
	}
}