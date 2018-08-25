var n = 6;
var out = "";
for (var i = 0; i < n; ++i)
	for (var j = i + 1; j < n; ++j)
		out += "(" + i + ", " + j + ") ";

console.log(out);
