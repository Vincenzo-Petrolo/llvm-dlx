
int acc(int n)
{
	int tmp = 0;

	for (int i = 0; i < n; i++)
	{
		tmp+=i;
	}

	return tmp;
}

int main(void)
{
	int a = 2;
	int b = 1;

	return acc(a) + acc(b);
}


int _start(void) {
	return main();
}
