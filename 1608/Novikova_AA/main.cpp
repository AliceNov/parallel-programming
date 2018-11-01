#define _CRT_NO_WARNINGS
#include "mpi.h"
#include <time.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	int *matrix = nullptr, *tarray = nullptr;
	int ProcNum, ProcRank;
	int N = 0, M = 0;
	int *result = nullptr;
	MPI_Status status;
	double t;
	int h = 0, v = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);


	if (ProcRank == 0)
	{
		N = atoi(argv[1]);
		M = atoi(argv[2]);
	
		matrix = new int[N*M];
		srand(time(0));
		for (int i = 0; i < N*M; i++)
			matrix[i] = rand() % 10;

		result = new int[M];
		for (int i = 0; i < M; i++)
			result[i] = 0;

		tarray = new int[N*M];
		int h = 0;
		for (int j = 0; j < M; j++)
			for (int i = j; i < N*M; i += M)
			{
				tarray[h] = matrix[i];
				h++;
			}
	}


	MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if (ProcRank == 0)
	{


		h = M / ProcNum; //количество строк для каждого процесса
		v = M % ProcNum; //оставшееся количество, первому процессу
		t = MPI_Wtime();
		for (int i = 1; i < ProcNum; i++)
			MPI_Send(tarray + (i)*h*N, h*N, MPI_INT, i, i, MPI_COMM_WORLD);
	}

	if (ProcRank != 0)
	{
		h = M / ProcNum;
		int *tmp = new int[N*h];
		int *res = new int[h];
		int sum;

		MPI_Recv(tmp, N*h, MPI_INT, 0, ProcRank, MPI_COMM_WORLD, &status);

		for (int i = 0; i < h; i++)
		{
			sum = 0;
			for (int j = 0; j < N; j++)
				sum += tmp[j + N * i];
			res[i] = sum;
		}
		MPI_Send(res, h, MPI_INT, 0, ProcRank, MPI_COMM_WORLD);
		delete tmp;
		delete res;
	}
	else
	{
		int sum;
		for (int i = 0; i < h; i++)
		{
			sum = 0;
			for (int j = 0; j < N; j++)
				sum += tarray[j + i * N];
			result[i] = sum;
		}

		if (v != 0)
		{
			for (int i = 0; i < v; i++)
			{
				sum = 0;
				for (int j = 0; j < N; j++)
					sum += tarray[h*ProcNum*N + j + i * N];
				result[i + h * ProcNum] = sum;
			}
		}
	}
	
	if (ProcRank == 0)
	{
		for (int i = 1; i < ProcNum; i++)
		{
			MPI_Recv(result + i * h, h, MPI_INT, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
		}
		cout<<"\n";
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < M; j++)
			{
				cout<< matrix[i*M + j]<<" ";
			}
			cout<<"\n";
		}

		for (int j = 1; j <= M; j++)
		{
			cout<<"Sum of "<<j<<" column: "<< result[j - 1];
			cout << "\n";
		}
		cout << "Time = " << MPI_Wtime() - t;
		cout << "\n";

	}

	if (matrix != nullptr)
		delete matrix;
	if (tarray != nullptr)
		delete tarray;
	if (result != nullptr)
		delete result;

	MPI_Finalize();
	return 0;
}