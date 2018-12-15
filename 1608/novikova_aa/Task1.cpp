#include "mpi.h"	
#include <iostream>
#include <cmath>	
#include <cstdlib>	
#include <cstdio>	
#include <fstream>

using namespace std;

void ShowMatrix(int *M, const int m)
{
	cout << endl;
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < m; j++)
		{
			cout << M[m * i + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

int *GetMatrix(const int m)
{
	int *res = new int[m * m];
	for (int i = 0; i < m * m; ++i)
		res[i] = rand() % 100 + 50;
	return res;
}

void main(int argc, char *argv[])
{
	int n;
	int *A = 0, *B = 0, *C = 0, *D = 0;

	int proc_num, proc_rank1;

	double paral_start, paral_end;
	double lin_start, lin_end;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank1);

	if (proc_rank1 == 0)
	{

		cout << "matrix size: ";
		cin >> n;
		cout << endl;

		srand(MPI_Wtime());
		A = GetMatrix(n);
		B = GetMatrix(n);
		C = new int[n * n]();
		D = new int[n * n]();

		if (n <= 10)
		{
			ShowMatrix(A, n);
			ShowMatrix(B, n);
		}

		lin_start = MPI_Wtime();
		MultMatrix(A, B, D, n);
		lin_end = MPI_Wtime();
		printf("Linear program time: %f\n", lin_end - lin_start, " milliseconds ");
		if (n <= 10)
		{
			ShowMatrix(D, n);
		}

		paral_start = MPI_Wtime();

	}

	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	ParalProg(A, B, C, n, proc_num);

	if (proc_rank1 == 0)
	{
		paral_end = MPI_Wtime();
		printf("Parallel program time: %f\n", paral_end - paral_start, " milliseconds ");
		if (n <= 10)
		{
			ShowMatrix(C, n);
		}

		bool flag = true;
		for (int i = 0; i < n * n; ++i)
			if (C[i] != D[i])
			{
				flag = false;
				break;
			}

		if (flag)
			printf("Results are equal\n");
		else
			printf("Results are not\n");

		printf("Boost: %f\n", (lin_end - lin_start) / (paral_end - paral_start));

	}

	MPI_Finalize();
}