#include<iostream>
#include<time.h>
#include<mpi.h>
using namespace std;
int main(int argc, char* argv[]){

	int ProcNum, ProcRank;
	double times;
	int numOfReaders = 0;
	int numOfWriters = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	numOfReaders = atoi(argv[1]);
	numOfWriters = atoi(argv[2]);


	if (ProcRank % 2 == 0)
	{
		//writer
		cout << "Writer " << ProcRank << " come" << endl;

		count_write[ProcRank] = 1;
		for (int i = 0; i < ProcNum; i++)
		{
			if ((count_write[i] == 1) && (ProcRank != i))
			{
				int j = 0;
				MPI_Recv(&j, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &Status);
				cout << "Writer " << ProcRank << " wait" << endl;
			}
		}
		cout << "Writer " << ProcRank << " write" << endl;
		count_write[ProcRank] = 0;
		cout << "Writer " << ProcRank << " gone" << endl;
	}
	else
	{
		//reader
		cout << "Reader " << ProcRank << " come" << endl;
		for (int i = 0; i < ProcNum; i++)
		{
			if (count_write[i] == 1)
			{
				int j = 0;
				MPI_Recv(&j, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &Status);
				cout << "Reader " << ProcRank << " wait" << endl;

			}
		}
		cout << "Reader " << ProcRank << " read" << endl;
		cout << "Reader " << ProcRank << " gone" << endl;

	}






	times = MPI_Wtime();
	cout << "Time = " << MPI_Wtime() - times;
	MPI_Finalize();
	return 0;
}
