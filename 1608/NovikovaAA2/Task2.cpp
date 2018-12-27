#include "mpi.h"
#include <iostream>
#include <ctime>
#include <Windows.h>
#include <algorithm>

#define BUF 0
#define READ_REQUEST 1
#define FINISH_READ 2
#define WRITE_REQUEST 3
#define REQUEST 4

using namespace std;

void main(int argc, char** argv)
{
	time_t t;
	int request = -2, response = 0, recieve = 1;
	int data = 0, index = 0;
	int ProcNum, ProcRank;
	MPI_Status status;
	MPI_Request mpiRequest;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	if (ProcRank == 0) // proc with buf
	{
		int wc = (ProcNum - 1) / 2 + (ProcNum - 1) % 2;
		int rc = 0;
		cout << "Writers count = " << wc << endl;

		while (true){
			if (recieve){
				MPI_Irecv(&request, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST, MPI_COMM_WORLD, &mpiRequest);
				recieve = 0;
			}
			if (!recieve){
				MPI_Test(&mpiRequest, &index, &status);
				if ((index) && (request == WRITE_REQUEST)){
					if (!rc){
						response = 1;
						MPI_Send(&response, 1, MPI_INT, status.MPI_SOURCE, BUF, MPI_COMM_WORLD);
						cout << "Writer #" << status.MPI_SOURCE << "is writing" << endl;
						MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, status.MPI_SOURCE, MPI_COMM_WORLD, &status);
						recieve = 1;
					}
					else{
						response = 0;
						MPI_Send(&response, 1, MPI_INT, status.MPI_SOURCE, BUF, MPI_COMM_WORLD);
						cout << "Writer #" << status.MPI_SOURCE << "is waiting" << endl;
						recieve = 1;
					}
				}
				if ((index) && (request == READ_REQUEST)){
					cout << "Reader #" << status.MPI_SOURCE << "is reading" << endl;
					rc++;
					MPI_Isend(&data, 1, MPI_INT, status.MPI_SOURCE, READ_REQUEST, MPI_COMM_WORLD, &request);
					recieve = 1;
				}
				if ((index) && (request == FINISH_READ)){
					rc--;
					cout << "Reader #" << status.MPI_SOURCE << "is finished" << endl;
					recieve = 1;
				}

			}
		}
		MPI_Finalize();
	}
	//writer
	else if (ProcRank % 2 == 1) {
		request = WRITE_REQUEST;
		srand((unsigned)time(&t));
		data = ProcRank;
		while (true){
				MPI_Send(&request, 1, MPI_INT, BUF, REQUEST, MPI_COMM_WORLD);
				MPI_Recv(&response, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
				if (response){
					MPI_Send(&data, 1, MPI_INT, BUF, ProcRank, MPI_COMM_WORLD);
				}
				Sleep(15000 + 100 * ProcRank);
		}
	}
	else{
		while (true){
				request = READ_REQUEST;
				MPI_Send(&request, 1, MPI_INT, BUF, REQUEST, MPI_COMM_WORLD);
				MPI_Recv(&data, 1, MPI_INT, BUF, READ_REQUEST, MPI_COMM_WORLD, &status);
				Sleep(500 * ProcRank);
				request = FINISH_READ;
				MPI_Send(&request, 1, MPI_INT, BUF, REQUEST, MPI_COMM_WORLD);
				Sleep(1000 + 80 * ProcRank);
		}
	}


	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Finalize();
}