
int acc(int n)
{
	int tmp = 0;

	for (int i = 0; i < n; i++)
	{
		tmp+=i;
	}

	return tmp;
}

int partition(int *arr, int low, int high)
{
	int pivot = arr[high];
	int i = low - 1;

	for (int j = low; j <= high - 1; j++)
	{
		if (arr[j] < pivot)
		{
			i++;
			int tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
		}
	}

	int tmp = arr[i + 1];
	arr[i + 1] = arr[high];
	arr[high] = tmp;

	return i + 1;
}

int quick_sort(int *arr, int low, int high)
{
	if (low < high)
	{
		int pi = partition(arr, low, high);

		quick_sort(arr, low, pi - 1);
		quick_sort(arr, pi + 1, high);
	}
}

unsigned div(unsigned a, unsigned b)
{
	return a / b;
}

int main(void)
{
	int a = 2;
	int b = 1;

	int array [5] = {5, 4, 3, 2, 1};

	quick_sort(array, 0, 4);

	return div((unsigned) (acc(a) + acc(b)), (unsigned)array[0]);
}


int _start(void) {
	return main();
}
