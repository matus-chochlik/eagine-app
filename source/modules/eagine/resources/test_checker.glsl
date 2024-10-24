float checker(vec2 c) {
	c = c * 8.0;
	return float((int(c.x) % 2 + int(c.y) % 2) % 2);
}

