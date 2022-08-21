class Main {
	void main() {
		int number = 1;

		int res = 0;
		int pow = number;
		int fact = 1;

		int i = 0;
		while(i < 6) {
			res = res + (pow / fact);
			pow = pow * ((0-1) * number * number);
			fact = fact * ((2 * (i + 1)) * (2 * (i + 1) + 1));
			i = i + 1;
		}

		int result = res * 10;
	}
}