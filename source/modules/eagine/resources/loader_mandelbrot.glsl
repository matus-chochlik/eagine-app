float mandelbrot(vec2 c) {
	vec2 z = vec2(0.0, 0.0);
	int i = 0, max = 128;
	while((i != max) && (distance(z, c) < 2.0)) {
		vec2 zn = vec2(
			z.x * z.x - z.y * z.y + c.x,
			2.0 * z.x * z.y + c.y);
		z = zn;
		++i;
	}
	return float(i) / float(max);
}

