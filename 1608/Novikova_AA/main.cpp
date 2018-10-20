#include "mpi.h"
#include <time.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	int *array = nullptr, *tarray = nullptr;;
	int ProcNum, ProcRank;//Число и ранг процессов
	int N = 0, M = 0; // Колонки и строки 
	int *result = nullptr; 	// Результат 
	MPI_Status status;
	double times;
	int k = 0, l = 0;

	/*Начало MPI кода*/
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);


	if (ProcRank == 0) 
	{
		if (argc < 3)
		{
		cout << "Enter number of colums = ";
		cin >> N;
		cout << "Enter number of rows = ";
		cin >> M;
		}
		else
		{
			N = atoi(argv[1]);
			M = atoi(argv[2]);
		}

		array = new int[N*M];
		srand(time(0));
		//Заполняем матрицу рандомными числами 
		for (int i = 0; i<N*M; i++)
			array[i] = rand() % 10;

		result = new int[M];
		for (int i = 0; i< M; i++)
			result[i] = 0;

		tarray = new int[N*M];
		int h = 0;
		for (int j = 0; j < M; j++){
			for (int i = j; i < N*M; i+=M){
				tarray[h] = array[i];
				h++;
			}
		}

	}
	
	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD); //передаем размерности всем процессам
	MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);//передаем размерности всем процессам

	if (ProcRank == 0)
	{
		

		k = M / ProcNum; //количество строк для каждого процесса
		l = M % ProcNum; //оставшееся количество, первому процессу

		//делим строки матрицы 
		for (int i = 1; i< ProcNum; i++)
			MPI_Send(tarray + (i)*k*N, k*N, MPI_INT, i, i, MPI_COMM_WORLD);
	}

	times = MPI_Wtime();
	//вычисляем суммы
	if (ProcRank != 0)
	{
		k = M / ProcNum;
		int *tmp = new int[N*k];
		int *res = new int[k];
		int sum;

		MPI_Recv(tmp, N*k, MPI_INT, 0, ProcRank, MPI_COMM_WORLD, &status);

		for (int i = 0; i < k; i++)
		{
			sum = 0;
			for (int j = 0; j < N; j++)
				sum += tmp[j + N*i];
			res[i] = sum;
		}
		MPI_Send(res, k, MPI_INT, 0, ProcRank, MPI_COMM_WORLD);
		delete tmp;
		delete res;
	}
	else
	{
		int sum;
		for (int i = 0; i < k; i++)
		{
			sum = 0;
			for (int j = 0; j<N; j++)
				sum += tarray[j + i*N];
			result[i] = sum;
		}

		if (l != 0)
		{
			for (int i = 0; i < l; i++)
			{
				sum = 0;
				for (int j = 0; j < N; j++)
					sum += tarray[k*ProcNum*N + j + i*N];
				result[i + k*ProcNum] = sum;
			}
		}
	}
	
	printf("Time = %.10f\n", MPI_Wtime() - times);

	if (ProcRank == 0)
	{
		for (int i = 1; i< ProcNum; i++)
		{
			MPI_Recv(result + i*k, k, MPI_INT, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &status);
		}
		cout<<"\n";
		for (int i = 0; i<M; i++)
		{
			for (int j = 0; j<N; j++)
			{
				cout << tarray[i*M + j] << " ";
			}
			cout << "sum = " << result[i]<<"\n";
		}
	}

	MPI_Finalize();
	return 0;
}