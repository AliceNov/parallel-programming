#include <iostream>
#include <time.h>
#include <mpi.h>
#include <cmath>

using namespace std;

int ShowMatrix(double* A, int N){
	cout << endl;
	for (int i = 0; i<N*N; i += N)
	{
		for (int j = 0; j< N; j++)
			cout << A[i + j] << " ";
		cout << endl;
	}
	cout << endl;
	return 1;
}
double* GetRandMatrix(int N){
	double *result = new double[N*N];
	srand(time(0));
	for (int i = 0; i< N*N; i++)
		result[i] = (rand() % 10000) / 1000.0f;
	return result;
}
int CheckMatrixForEqual(double *A, double *C, int N){
	for (int i = 0; i<N*N; i++)
		if (abs(A[i] - C[i]) > 0.000001)
			return 0;
	return 1;
}
void MultiplayMatrix(double* A, double* B, double* C, const int n)
{
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			for (int k = 0; k < n; ++k)
				C[i * n + j] += A[i * n + k] * B[k * n + j];
}

int main(int argc, char** argv){
	double *A = nullptr, *B = nullptr, *C = nullptr;
	double timeP, timeL;
	int N, size, root;
	int procNum, rank;
	int left_rank, right_rank, up_rank, down_rank;
	MPI_Status status;
	MPI_Datatype blockType;
	MPI_Comm grid;

	if (argc>1)
		N = atoi(argv[1]);
	else
	{
		cout << "Try again.Usage:Task1.exe  n(n is size of matrices)" << endl;
		return 2;
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);

	//Create a grid
	int dims[2];
	dims[0] = dims[1] = (int)(sqrt(procNum) + 0.5);
	if ((dims[0] * dims[0] != procNum) || (N % dims[0] != 0))
	{
		MPI_Finalize();
		printf("Number of processes must be a perfect square\n");
		printf("Matrices must be fully devisible to processes\n");
		exit(-1);
	}
	int periods[2] = { 1, 1 };
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &grid);
	MPI_Comm_rank(grid, &rank);
	// Get root process
	int coords[2] = { 0, 0 };
	MPI_Cart_rank(grid, coords, &root);
	// Get neighbors
	MPI_Cart_shift(grid, 1, -1, &right_rank, &left_rank);
	MPI_Cart_shift(grid, 0, -1, &down_rank, &up_rank);

	//Give memory to blocks
	size = N / dims[0];
	double *blockA = new double[size*size];
	double *blockB = new double[size*size];
	double *blockC = new double[size*size] ();
	/*C = new double[N*N];
	for (int i = 0; i<size*size; i++)
		blockC[i] = 0;
*/
	// Create vector type to represent a block
	MPI_Datatype temptype;
	MPI_Type_vector(size, size, N, MPI_DOUBLE, &temptype);
	MPI_Type_create_resized(temptype, 0, size*size*sizeof(double), &blockType);
	MPI_Type_commit(&blockType);

	if (rank == root){
		A = GetRandMatrix(N);
		B = GetRandMatrix(N);
		C = new double[N * N];

		ShowMatrix(A, N);
		ShowMatrix(B, N);

		//Start parallel work
		timeP = MPI_Wtime();

		// Copy the first blocks of matrices to block arrays
		for (int i = 0; i < size; ++i)
			for (int j = 0; j < size; j++)
			{
				blockA[i * size + j] = A[i * N + j];
				blockB[i * size + j] = B[i * N + j];
			}
		// Initial Cannon's alignment (shifted rows in A)
		for (int i = 0; i < dims[0]; ++i)
			for (int j = 0; j < dims[1]; ++j)
				if ((i != 0) || (j != 0))
				{
					// Send block of A matrix with initial shift to left
					int dest, block_coords[2] = { i, j - i };
					if (block_coords[1] < 0) block_coords[1] += dims[1];
					MPI_Cart_rank(grid, block_coords, &dest);
					MPI_Send(&A[i * N * size + j * size], 1, blockType, dest, 0, grid);
				}
		// Initial Cannon's alignment (shifted columns in B)
		for (int i = 0; i < dims[0]; ++i)
			for (int j = 0; j < dims[1]; ++j)
				if ((i != 0) || (j != 0))
				{
					// Send block of B matrix with initial shift to up
					int dest, block_coords[2] = { i - j, j };
					if (block_coords[0] < 0) block_coords[0] += dims[0];
					MPI_Cart_rank(grid, block_coords, &dest);
					MPI_Send(&B[i * N * size + j * size], 1, blockType, dest, 1, grid);
				}
	}
	else
	{
		// Recieve initial blocks from root process
		MPI_Recv(blockA, size * size, MPI_DOUBLE, root, 0, grid, &status);
		MPI_Recv(blockB, size * size, MPI_DOUBLE, root, 1, grid, &status);
	}

	// Main loop
	MultiplayMatrix(blockA, blockB, blockC, size);
	for (int i = 1; i < dims[0]; ++i)
	{
		MPI_Sendrecv_replace(blockA, size * size, MPI_DOUBLE, left_rank, 0, right_rank, 0, grid, &status);
		MPI_Sendrecv_replace(blockB, size * size, MPI_DOUBLE, up_rank, 1, down_rank, 1, grid, &status);
		MultiplayMatrix(blockA, blockB, blockC, size);
	}
	if (rank == root){
		// Copy first BlockC to matrix C
		for (int i = 0; i < size; ++i)
			for (int j = 0; j < size; ++j)
				C[i * N + j] = blockC[i * size + j];
		// Collect blocks from processes
		for (int i = 0; i < dims[0]; ++i)
			for (int j = 0; j < dims[1]; ++j)
				if ((i != 0) || (j != 0))
				{
					int source, block_coords[2] = { i, j };
					MPI_Cart_rank(grid, block_coords, &source);
					MPI_Recv(&C[i * N * size + j * size], 1, blockType, source, 4, grid, &status);
				}
		printf("Time parallel = %.10f\n", MPI_Wtime() - timeP);
		ShowMatrix(C, N);

		double *C_Lin = new double[N*N];
		for (int i = 0; i<N*N; i++)
			C_Lin[i] = 0;
		timeL = MPI_Wtime();

		MultiplayMatrix(A, B, C_Lin, N);
		printf("Time linear = %.10f\n", MPI_Wtime() - timeL);

		ShowMatrix(C_Lin, N);

		// Check if results are equal
		if (CheckMatrixForEqual(C, C_Lin, N) == 1)
			cout << "Matrices are equal" << endl;
		else
			cout << "Matrices are not equal" << endl;
		delete C_Lin;
	}
	else
		// Send blocks of C matrix to root process
		MPI_Send(blockC, size * size, MPI_DOUBLE, root, 4, grid);
	MPI_Type_free(&blockType);
	MPI_Comm_free(&grid);

	MPI_Finalize();

	delete A;
	delete B;
	delete C;
	delete blockA;
	delete blockB;
	delete blockC;


	return 0;
}