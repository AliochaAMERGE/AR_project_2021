#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "./include/header.h"

#define NB_PROC 11     // inconnu des pairs
#define N NB_PROC - 1  // inconnu des pairs
#define M 6

/* TAGS */


/*Variables Globales*/
MPI_Status status;
int id_chord;     // identifiant chord du processus courant
int rang;         // MPI_rank