#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TAGINIT 0
#define NB_SITE 6

// TODO
/*  !!!!! valeur a initialiser !!!!!*/
#define NB_SITE //nombre de sites (processus)
#define M //nombre de fingers

/*LES TAGS */
#define TAG_INIT
//... d'autres TAG maybe 
#define TAG_TERMINAISON


/******************************Fonctions Aléatoires****************************/

/******************************************************************************
 *             f retourne aléatoirement un identifiant CHORD unique           *
 *             parmis l'ensemble des pairs du système                         *
 ******************************************************************************/

// p : rank
void f(int p) {}
/******************************************************************************
 *            g retourne aléatoirement un identifiant unique pour une donnée  *
 *            (clés) parmis l'ensemble des données du système                 *
 ******************************************************************************/
void g(void) {}

/*************Initialisation de la DHT par le processus simulateur*************/

void simulateur(void)
{
  int i;
  /***************************************************************************
   * A noté : L'indice de ces tableaux représente le rank des processus      *
   * la premiere case de ces derniers contiendra -1 pour le rank 0 car c'est *
   * lui qui est le processus simulateur                                     *
   ***************************************************************************/
  int CHORD[NB_SITE][2]; // contient un ensemble de couple (MPI rank, id chord)
  int fingers[NB_SITE][M]; // contient des ensemble de M fingers pour chaque site
  int succ[NB_SITE]; // contient l'identifiant CHORD du successeur succ[i]

  /*initialisation des identifiants CHORD, soit le tableau CHORD */

  /*initialisation des fingers, soit le tableau fingers*/

  /*initialisation du successeur de chacun des sites*/

  // NE PAS OUBLIER DE METTRE LES PAIRS DANS L'ORDRE CROISANT ==>> ANNEAU !!!!
  // EN FONCTION DES ID CHORD ET NON PAS RANK
  
  /*envoi à chacun des pairs du système leurs variables*/
  //===>>>> MPI_Send à faire
  // tag à créer ???? a voir

  /*------------------Fin de l'initialisation du système------------------*/

  /* QUESTION 3 : le simulateur
   *  tire aleatoirement un id pair (verifier son existence)
   *                     une clé de donnée
   *  envoi d'un message à ce pair pour chercher le responsable de cette donnée
   *  Attente de la réponse de la pair le responsable de cette donnée
   *  Progration du message de terminaison à tous les processus
   **/
}
/*
p4 recoit de P0
{-1, id1, id2, id3, -1, -1, id6, id7, -1}
id1, id2, id3, id6, id7 <- pas ça 
id4

CHORD => contient tous les couples (rank, id) de chaque pair de anneau ou id correspond à l'id du pair 

fingers contient des sous tableaux de finger pour chaque pair

pour l'envoi des fingers il faudra créer un tableau chord_tmp dans lequel on mettre SEULEMENT les couples 
correspond au id_chord des pairs qui sont dans le sous tableau de finger du proc i
et donc chaque proc i aura l'information seulement des ses finger pair id et son rang mpi

p34, p44, p55

*/

/*-----------------------Fin de la fonction simulateur------------------------*/

/*************************************MAIN*************************************/

int main(int argc, char* argv[])
{
  int nb_proc, rang;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

  if (nb_proc != NB_SITE + 1)
  {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rang);

  if (rang == 0)
  {
    simulateur();
  }
  else
  {
    calcul_min(rang);
  }

  MPI_Finalize();
  return 0;
}