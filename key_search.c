#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 11  // nombre de sites (processus) en comptant le processus initiateur
#define M 6   // nombre de fingers

// ! il faut que N > M

/*LES TAGS */
#define TAG_INIT 0
//... d'autres TAG maybe
#define TAG_TERMINAISON 1

/* les variables globales */
MPI_Status status;

/**********************Fonction d'appartenance à [a,b[************************/
/******************************************************************************
 *               app vérifie si la clé k appartient à                         *
 *               l'intervalle [a,b[                                           *
 *****************************************************************************/

int app(int k, int a, int b)
{
  if (a < b) return ((k >= a) && (k < b));
  return (((k >= 0) && (k < b)) || ((k >= a) && (k < N)));
}

/******************************Fonctions Aléatoires****************************/

/******************************************************************************
 *             f retourne aléatoirement un identifiant CHORD unique           *
 *             parmis l'ensemble des pairs du système                         *
 ******************************************************************************/

// p : rank
// id_already_used : contient les idChord pour les p-1 ranks, p : rank
int f(int** id_already_used, int p)
{
  srand(time(NULL));
  int alea_chord;
  // on a 2^M-1 valeurs possibles
  int i = 0;
  do
  {
    alea_chord = (rand() % pow((M, 2) - 1));  // retire une valeur aléatoire
    if (id_already_used[i] == alea_chord)
    {
      i = 0;  // repart du début
    }
    else
    {
      i++;
    }
  } while (i < p);

  id_already_used[p] = alea_chord;

  return alea_chord;
}
/******************************************************************************
 *            g retourne aléatoirement un identifiant unique pour une donnée  *
 *            (clés) parmis l'ensemble des données du système                 *
 ******************************************************************************/

int g(int value) { return value % (pow(2, M) - 1); }

/******************************Fonctions pour trier****************************/
int compare(const void* a, const void* b)
{
  int int_a = *((int*)a);
  int int_b = *((int*)b);

  if (int_a == int_b)
    return 0;
  else if (int_a < int_b)
    return -1;
  else
    return 1;
}

// qsort( array, element_count, element_size, *compare_function )

/*************Initialisation de la DHT par le processus simulateur*************/

void simulateur(void)
{
  int i;
  /***************************************************************************
   * A noté : L'indice de ces tableaux représente le rank des processus      *
   * la premiere case de ces derniers contiendra -1 pour le rank 0 car c'est *
   * lui qui est le processus simulateur                                     *
   ***************************************************************************/
  
  // contient un ensemble de ID_CHORD
  int id_chord[N - 1];
  // contient des ensemble de M fingers pour chaque site
  // un finger est composé de son ID_CHORD et de son MPI_RANK
  int fingers[N - 1][M][2];

  /*initialisation des identifiants CHORD, soit le tableau id_chord puis tri */

  for (size_t p = 0; p < N - 1; p++)
  {
    f(*id_chord, p);
  }
  qsort(*id_chord, N - 1, sizeof(int), compare);

  /*initialisation des fingers, soit le tableau fingers*/

  for (size_t p = 0; p < N - 1; p++)
  {
    /* initialisation du successeur du site p */
    fingers[p][0][0] = p+1; // ! faux
    fingers[p][0][1] = id_chord[(p + 1) % N]; // ! faux

    // chaque process se voit assigner M fingers (f0 = succ)
    // tel que les fingers sont compris entre succ et process p + M/2 (moitié du
    // tableau)
    for (size_t f = 1; f < M; f++)
    {
      // TODO check if it works
      // ! vraiment pas sur
      int fing = ((p + 1) + rand() % ((p + 1) + N - 1 / 2)) % N - 1;
      // renvoie un entier aléatoire entre p+1 et p + N/2 (en respectant l'ordre cyclique)
      fingers[p][f][0] = id_chord[fing];
     
    }
  }

  /* envoie des messages aux processus avec leurs id_chord et fingers */

  // NE PAS OUBLIER DE METTRE LES PAIRS DANS L'ORDRE CROISANT ==>> ANNEAU !!!!
  // ~fait EN FONCTION DES ID CHORD ET NON PAS RANK ~fait

  /*envoi à chacun des pairs du système leurs variables*/

  for (size_t p = 0; p < N - 1; p++)
  {
    // envoie l'ID_CHORD à chaque processus (avec son MPI_rank mais pas
    // forcément utile)
    MPI_Send(&id_chord[p], 2, MPI_INT, (p + 1), TAG_INIT, MPI_COMM_WORLD);
    // envoie les fingers
    MPI_Send(&id_chord[p], 2, MPI_INT, (p + 1), TAG_INIT, MPI_COMM_WORLD);
  }

  /*
  ! N site
    ? ils ont chacun M finger
      * chaque finger est composé de ID chord et rank
  */
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

CHORD => contient tous les couples (rank, id) de chaque pair de anneau ou id
correspond à l'id du pair

fingers contient des sous tableaux de finger pour chaque pair

pour l'envoi des fingers il faudra créer un tableau chord_tmp dans lequel on
mettre SEULEMENT les couples correspond au id_chord des pairs qui sont dans le
sous tableau de finger du proc i et donc chaque proc i aura l'information
seulement des ses finger pair id et son rang mpi

p34, p44, p55

*/

/*-----------------------Fin de la fonction simulateur------------------------*/

/*************************************MAIN*************************************/

int main(int argc, char* argv[])
{
  int nb_proc, rang;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

  if (nb_proc != N + 1)
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