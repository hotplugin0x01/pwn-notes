#define BUFSZ 8
char* values[256];

void assignkey(unsigned char key)
{
	int n;
	char buf[BUFSZ + 1];
	char *v;

	if (values[key] != NULL)
		free(values[key]);

	n = read(0, buf, BUFSZ);
	if (n <= 0) {
		fprintf(stderr, "Error/EOF while reading value\n");
		return;
	}

	buf[n] = '\0';
	v = malloc(n + 1);

	if (v == NULL) {
		fprintf(stderr, "out of memory\n");
		return;
	}
	
	memcpy(v, buf, n + 1);
	values[key] = v;
}

void deletekey(unsigned char key)
{
	char *v = values[key];

	if (v == NULL) {
		fprintf(stderr, "No such key: %c\n", key);
		return;
	}
	free(v);
}

void child()
{
	char cmd;
	while (read(0, &cmd, 1) > 0) {
		unsigned char key;
		if (cmd != 'a' && cmd != 'd' && cmd != 's' && cmd != 'q') {
			if (cmd != '\n')
				fprintf(stderr, "Unknown command: '%c'\n", cmd);
			continue;
		}
		if (cmd == 'q')
			break;
		if (read(0, &key, 1) <= 0) {
			fprintf(stderr, "Error/EOF while reading key\n");
			return;
		}
		switch (cmd) {
		case 'a':
			assignkey(key);
			break;
		case 'd':
			deletekey(key);
			break;
		case 's':
			fprintf(stderr, "sorry, not implemented yet\n");
			break;
		default:
			break;
		}
	}
	system("echo goodbye");
}
