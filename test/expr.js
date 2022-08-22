function main() {
	var number = 0.3;

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

	var result = res * 10;

	var actual = sin(number) * 10;

	var wtf = 77 > 9;
}

