void child()
{
	char buf[500];
	int len, n;

	if (fread(&len, sizeof(len), 1, stdin) != 1) {
		fprintf(stderr, "read returned %d\n", n);
		return;
	}
	n = fread(buf, 1, len, stdin);
	fwrite(buf, 1, n, stdout);
}
