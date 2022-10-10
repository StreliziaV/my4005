#include <mpi.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <chrono>


int main (int argc, char **argv){

    MPI_Init(&argc, &argv); 

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int num_elements; // number of elements to be sorted
    
    num_elements = atoi(argv[1]); // convert command line argument to num_elements

    int elements[num_elements]; // store elements
    int sorted_elements[num_elements]; // store sorted elements

    if (rank == 0) { // read inputs from file (master process)
        std::ifstream input(argv[2]);
        int element;
        int i = 0;
        while (input >> element) {
            elements[i] = element;
            i++;
        }
        std::cout << "actual number of elements:" << i << std::endl;
    }

    std::chrono::high_resolution_clock::time_point t1;
    std::chrono::high_resolution_clock::time_point t2;
    std::chrono::duration<double> time_span;
    if (rank == 0){ 
        t1 = std::chrono::high_resolution_clock::now(); // record time
    }

    /* TODO BEGIN
        Implement parallel odd even transposition sort
        Code in this block is not a necessary. 
        Replace it with your own code.
        Useful MPI documentation: https://rookiehpc.github.io/mpi/docs
    */

    int num_my_element = num_elements / world_size; // number of elements allocated to each process
    int my_element[num_my_element]; // store elements of each process
    MPI_Scatter(elements, num_my_element, MPI_INT, my_element, num_my_element, MPI_INT, 0, MPI_COMM_WORLD); // distribute elements to each process
    
    // std::cout << rank << ' ' << my_element[0] << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    
    bool sorted = false;
    bool sorted_array[world_size];
    if (num_my_element % 2 == 0 && world_size > 1) {
        while (!sorted) {
            sorted = true;
            int from_p = -1;
            if (rank == 0) {
                MPI_Recv(&from_p, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else if (rank == world_size - 1) {
                MPI_Send(my_element, 1, MPI_INT, rank - 1, rank - 1, MPI_COMM_WORLD);
            }
            else {
                MPI_Send(my_element, 1, MPI_INT, rank - 1, rank - 1, MPI_COMM_WORLD);
                MPI_Recv(&from_p, 1, MPI_INT, rank + 1, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            
            MPI_Barrier(MPI_COMM_WORLD);
            
            for (int i = 0; i < num_my_element; i = i + 2) {
                if (i - 1 < 0) continue;
                if (i >= num_my_element) continue;
                if (my_element[i - 1] > my_element[i]) {
                    sorted = false;
                    int temp_i = my_element[i - 1];
                    my_element[i - 1] = my_element[i];
                    my_element[i] = temp_i;
                }
            }
            int to_send;
            if (rank != world_size - 1) {
                if (my_element[num_my_element - 1] > from_p) {
                    to_send = my_element[num_my_element - 1];
                    my_element[num_my_element - 1] = from_p;
                    sorted = false;
                }
                else {
                    to_send = from_p;
                }
            }
            int from_p_1 = -1;
            if (rank == 0) {
                MPI_Send(&to_send, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            }
            else if (rank == world_size - 1) {
                MPI_Recv(&from_p_1, 1, MPI_INT, rank - 1, rank - 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_element[0] = from_p_1;
            }
            else {
                MPI_Send(&to_send, 1, MPI_INT, rank + 1, rank, MPI_COMM_WORLD);
                MPI_Recv(&from_p_1, 1, MPI_INT, rank - 1, rank - 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_element[0] = from_p_1;
            }

            MPI_Barrier(MPI_COMM_WORLD);
            
            for (int i = 1; i < num_my_element; i = i + 2) {
                if (i - 1 < 0) continue;
                if (i >= num_my_element) continue;
                if (my_element[i - 1] > my_element[i]) {
                    sorted = false;
                    int temp_i = my_element[i - 1];
                    my_element[i - 1] = my_element[i];
                    my_element[i] = temp_i;
                }
            }
            
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Gather(&sorted, 1, MPI_C_BOOL, sorted_array, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
            if (rank == 0) {
                for (int j = 0; j < world_size; j++) {
                    sorted = sorted_array[j] && sorted;
                }
            }
            MPI_Bcast(&sorted, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    // odd number in each process
    if (num_my_element % 2 == 1 && world_size > 1) {
        while (!sorted){
            sorted = true;
            int from_p = -1;
            if (rank == 0) {
                MPI_Recv(&from_p, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else if (rank == world_size - 1) {
                MPI_Send(my_element, 1, MPI_INT, rank - 1, rank - 1, MPI_COMM_WORLD);
            }
            else {
                MPI_Send(my_element, 1, MPI_INT, rank - 1, rank - 1, MPI_COMM_WORLD);
                MPI_Recv(&from_p, 1, MPI_INT, rank + 1, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            
            MPI_Barrier(MPI_COMM_WORLD);

            for (int i = 0; i < num_my_element; i = i + 2) {
                if (i - 1 < 0) continue;
                if (i >= num_my_element) continue;
                if (my_element[i - 1] > my_element[i]) {
                    sorted = false;
                    int temp_i = my_element[i - 1];
                    my_element[i - 1] = my_element[i];
                    my_element[i] = temp_i;
                }
            }
            int to_send;
            if (rank % 2 == 1 && rank != world_size - 1) {
                if (rank != world_size - 1) {
                    if (my_element[num_my_element - 1] > from_p) {
                        to_send = my_element[num_my_element - 1];
                        my_element[num_my_element - 1] = from_p;
                        sorted = false;
                    }
                    else {
                        to_send = from_p;
                    }
                }
                MPI_Send(&to_send, 1, MPI_INT, rank + 1, rank, MPI_COMM_WORLD);
            }
            int from_p_1;
            if (rank > 0 && rank % 2 == 0) {
                MPI_Recv(&from_p_1, 1, MPI_INT, rank - 1, rank - 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_element[0] = from_p_1;
            }

            MPI_Barrier(MPI_COMM_WORLD);

            for (int i = 1; i < num_my_element; i = i + 2) {
                if (i - 1 < 0) continue;
                if (i >= num_my_element) continue;
                if (my_element[i - 1] > my_element[i]) {
                    sorted = false;
                    int temp_i = my_element[i - 1];
                    my_element[i - 1] = my_element[i];
                    my_element[i] = temp_i;
                }
            }

            if (rank % 2 == 0 && rank != world_size - 1) {
                if (rank != world_size - 1) {
                    if (my_element[num_my_element - 1] > from_p) {
                        to_send = my_element[num_my_element - 1];
                        my_element[num_my_element - 1] = from_p;
                        sorted = false;
                    }
                    else {
                        to_send = from_p;
                    }
                }
                MPI_Send(&to_send, 1, MPI_INT, rank + 1, rank, MPI_COMM_WORLD);
            }
            if (rank % 2 == 1) {
                MPI_Recv(&from_p_1, 1, MPI_INT, rank - 1, rank - 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_element[0] = from_p_1;
            }

            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Gather(&sorted, 1, MPI_C_BOOL, sorted_array, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
            if (rank == 0) {
                for (int j = 0; j < world_size; j++) {
                    sorted = sorted_array[j] && sorted;
                }
            }
            MPI_Bcast(&sorted, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);

        }
        
    }
    

    MPI_Gather(my_element, num_my_element, MPI_INT, sorted_elements, num_my_element, MPI_INT, 0, MPI_COMM_WORLD); // collect result from each process
    
    /* TODO END */

    if (rank == 0){ // record time (only executed in master process)
        t2 = std::chrono::high_resolution_clock::now();  
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        std::cout << "Student ID: " << "119010369" << std::endl; // replace it with your student id
        std::cout << "Name: " << "Bodong Yan" << std::endl; // replace it with your name
        std::cout << "Assignment 1" << std::endl;
        std::cout << "Run Time: " << time_span.count() << " seconds" << std::endl;
        std::cout << "Input Size: " << num_elements << std::endl;
        std::cout << "Process Number: " << world_size << std::endl; 
    }

    if (rank == 0){ // write result to file (only executed in master process)
        std::ofstream output(argv[2]+std::string(".parallel.out"), std::ios_base::out);
        for (int i = 0; i < num_elements; i++) {
            output << sorted_elements[i] << std::endl;
        }
    }
    
    MPI_Finalize();
    
    return 0;
}


