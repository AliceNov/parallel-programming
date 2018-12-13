#include "mpi.h"
#include <iostream>
#include <ctime>
#include <Windows.h>
#include <algorithm>

#define PUT 0
#define READ_REQUEST 1
#define FINISH_READ 3
#define WRITE_REQUEST 4
#define REQUEST 5
#define WORK_STOP 6

using namespace std;

int main(int argc, char **argv) {

	int ProcNum, ProcRank;
	time_t t;
	int WC = 1, RC = 0;
	int ReceiveOn = 1, request = -1, result = 0, data = 0, iter = 2;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	srand(time(NULL));

	int activp = ProcNum - 1;

	if (ProcRank == 0) {
		while (true) {
			if (ReceiveOn) {
				MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST, MPI_COMM_WORLD, &status);
				ReceiveOn = 0;
			}
			else if(!ReceiveOn){
				if (request == WRITE_REQUEST) {
					if (!RC) {
						result = 1;
						MPI_Send(&result, 1, MPI_INT, status.MPI_SOURCE, PUT, MPI_COMM_WORLD);
						cout << "Writer work, Process number #" << status.MPI_SOURCE << endl;
						MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, status.MPI_SOURCE, MPI_COMM_WORLD, &status);
						ReceiveOn = 1;
					}
					else {
						result = 0;
						MPI_Send(&result, 1, MPI_INT, status.MPI_SOURCE, PUT, MPI_COMM_WORLD);
						cout << "ACCESS DENIED " << endl;
						ReceiveOn = 1;
					}
				}
				if (request == READ_REQUEST) {
					cout << "Reader is reading, Process number #" << status.MPI_SOURCE << endl;
					RC++;
					MPI_Send(&data, 1, MPI_INT, status.MPI_SOURCE, READ_REQUEST, MPI_COMM_WORLD);
					ReceiveOn = 1;
				}
				if (request == FINISH_READ) {
					RC--;
					cout << "Reader finish read, Process number #" << status.MPI_SOURCE << endl;
					ReceiveOn = 1;
				}
				if (request = WORK_STOP) {
					cout << "Process stop" <<  endl;
					activp--;
				}
				if (!activp)
					break;
			}
		}
	}

	//Writers
	if (ProcRank > 0 && ProcRank <= WC) {
		request = WRITE_REQUEST;
		data = ProcRank;
		
		while (iter) {
			
				MPI_Send(&request, 1, MPI_INT, PUT, REQUEST, MPI_COMM_WORLD);
			MPI_Recv(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			cout << "I send, Process number #" << result<< endl;
			if (result) {
				MPI_Send(&data, 1, MPI_INT, PUT, ProcRank, MPI_COMM_WORLD);
			}
			Sleep(rand() * WC % 15000 + 3000);
			
		
				iter--;
	}
		request = WORK_STOP;
		MPI_Send(&request, 1, MPI_INT, PUT, REQUEST, MPI_COMM_WORLD);
	}
	//Readers
	if (ProcRank > WC) {
		
		while (iter) {
				request = READ_REQUEST;
				MPI_Send(&request, 1, MPI_INT, PUT, REQUEST, MPI_COMM_WORLD);
				MPI_Recv(&data, 1, MPI_INT, PUT, READ_REQUEST, MPI_COMM_WORLD, &status);
				Sleep(rand() * WC % 15000 + 3000);
				request = FINISH_READ;
				MPI_Send(&request, 1, MPI_INT, PUT, REQUEST, MPI_COMM_WORLD);
				Sleep(rand() * WC % 15000 + 3000);
			iter--;
		}
		request = WORK_STOP;
		MPI_Send(&request, 1, MPI_INT, PUT, REQUEST, MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}
