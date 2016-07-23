var assert = require('assert');
var fs = require('fs');
var child_process = require('child_process');


var first_decrement = 2;
var decrements = [1, 1, 3, 1, 3, 1, 5, 3, 3, 9, 3, 1, 3, 19, 15, 1, 5, 1, 3, 9, 3, 15, 3, 39, 5, 39, 57, 3, 35, 1, 5, 9, 41, 31, 5, 25, 45, 7, 87, 21, 11, 57, 17, 55, 21, 115, 59, 81, 27, 129, 47, 111, 33, 55, 5, 13, 27, 55, 93, 1, 57, 25, 59, 49, 5, 19, 23, 19, 35, 231, 93, 69, 35, 97, 15, 33, 11, 67, 65, 51, 57, 55, 35, 19, 35, 67, 299, 1, 33, 45, 83, 25, 3, 15, 17, 141, 51, 115, 15, 69, 33, 97, 17, 13, 117, 1, 59, 31, 21, 37, 75, 133, 11, 67, 3, 279, 5, 69, 119, 73, 3, 67, 59, 9, 137, 1, 159, 25, 5, 69, 347, 99, 45, 45, 113, 13, 105, 187, 27, 9, 111, 69, 83, 151, 153, 145, 167, 31];

assert(first_decrement + decrements.length === 150);

var num_fields = 10;

// End config


var exec = function(setup, equ)
{
	var out = child_process.spawnSync('python', ['-c', setup + 'print(' + equ + ')']).stdout.toString();
	var res = out.trim();

	process.stderr.write(equ + ' -> ' + res + '\n');

	return res;
};

var calc = function(equ, mod)
{
	var setup = '';
	setup += 'def wrap(n):\n\tif n < 0:\n\t\tn += (n // ' + mod + ' + 1) * ' + mod + '\n\treturn n % ' + mod + '\n';
	setup += 'def inv(n):\n\treturn pow(n, ' + mod + ' - 2, ' + mod + ')\n';
	return exec(setup, 'wrap(' + equ + ')');
};

var output = [];

output.push('// Generated file, do not modify. Make changes to tests/generate_intmodulomersenne.js');
output.push('');
output.push('#include "catch/single_include/catch.hpp"');
output.push('#include "libSonicSocket/intmodulomersenne.h"');
output.push('');

for (var i = 0; i < num_fields; i++)
{
	var exponent = Math.floor(Math.random() * decrements.length) + first_decrement;
	var decrement = decrements[exponent - first_decrement];
	var type = 'sonic_socket::IntModuloMersenne<' + exponent + ', ' + decrement + '>';

	var mod = exec('', '2 ** ' + exponent + ' - ' + decrement);
	var a = exec('import random\n', 'random.randrange(' + mod + ')');
	var b = exec('import random\n', 'random.randrange(' + mod + ')');
	var c = exec('import random\n', 'random.randrange(' + mod + ')');
	var d = exec('import random\n', 'random.randrange(' + mod + ')');
	var e = exec('import random\n', 'random.randrange(' + mod + ')');
	var f = exec('import random\n', 'random.randrange(' + mod + ')');
	var g = exec('import random\n', 'random.randrange(' + mod + ')');

	output.push('TEST_CASE("' + a + ' _ ' + b + ' (mod 2^' + exponent + ' - ' + decrement + ' == ' + mod + ')", "")');
	output.push('{');
	output.push('    typedef ' + type + ' Field;');
	output.push('');
	output.push('    Field a("' + a + '");');
	output.push('    Field b("' + b + '");');
	output.push('');
	output.push('    Field copy(a);');
	output.push('    REQUIRE(copy == a);');
	output.push('');
	output.push('    Field set;');
	output.push('    set = b;');
	output.push('    REQUIRE(set == b);');
	output.push('');
	output.push('    REQUIRE(a + b == Field("' + calc(a + ' + ' + b, mod) + '"));');
	output.push('    REQUIRE(a - b == Field("' + calc(a + ' - ' + b, mod) + '"));');
	output.push('    REQUIRE(b - a == Field("' + calc(b + ' - ' + a, mod) + '"));');
	output.push('    REQUIRE(a * b == Field("' + calc(a + ' * ' + b, mod) + '"));');
	output.push('    REQUIRE(a / b == Field("' + calc(a + ' * inv(' + b + ')', mod) + '"));');
	output.push('');
	output.push('    Field mut("' + c + '");');
	output.push('    mut += Field("' + d + '");');
	output.push('    REQUIRE(mut == Field("' + calc(c + ' + ' + d, mod) + '"));');
	output.push('    mut -= Field("' + e + '");');
	output.push('    REQUIRE(mut == Field("' + calc(c + ' + ' + d + ' - ' + e, mod) + '"));');
	output.push('    mut *= Field("' + f + '");');
	output.push('    REQUIRE(mut == Field("' + calc('(' + c + ' + ' + d + ' - ' + e + ') * ' + f, mod) + '"));');
	output.push('    mut /= Field("' + g + '");');
	output.push('    REQUIRE(mut == Field("' + calc('(' + c + ' + ' + d + ' - ' + e + ') * ' + f + ' * inv(' + g + ')', mod) + '"));');
	output.push('');
	output.push('    Field *inv = a.inverse();');
	output.push('    REQUIRE(*inv == Field("' + calc('inv(' + a + ')', mod) + '"));');
	output.push('');
	output.push('    REQUIRE(a.to_string() == "' + a + '");');
	output.push('');
	output.push('    Field::Hasher hasher;');
	output.push('    REQUIRE(hasher(a) != hasher(b));');
	output.push('    REQUIRE(hasher(a) == hasher(a));');
	output.push('');
	output.push('    REQUIRE(a == a);');
	output.push('    REQUIRE_FALSE(a == b);');
	output.push('');
	output.push('    REQUIRE(a != b);');
	output.push('    REQUIRE_FALSE(a != a);');
	output.push('');

	while (a.length < b.length) {a = '0' + a;}
	while (b.length < a.length) {b = '0' + b;}

	if (a > b)
	{
		var tmp = a;
		a = b;
		b = tmp;
	}

	output.push('    Field min("' + a + '");');
	output.push('    Field max("' + b + '");');
	output.push('');
	output.push('    REQUIRE(min < max);');
	output.push('    REQUIRE_FALSE(min < min);');
	output.push('    REQUIRE_FALSE(max < min);');
	output.push('');
	output.push('    REQUIRE(min <= max);');
	output.push('    REQUIRE(min <= min);');
	output.push('    REQUIRE_FALSE(max <= min);');
	output.push('');
	output.push('    REQUIRE(max > min);');
	output.push('    REQUIRE_FALSE(min > min);');
	output.push('    REQUIRE_FALSE(min > max);');
	output.push('');
	output.push('    REQUIRE(max >= min);');
	output.push('    REQUIRE(min >= min);');
	output.push('    REQUIRE_FALSE(min >= max);');
	output.push('}');
	output.push('');
}

process.stdout.write(output.join('\n'));