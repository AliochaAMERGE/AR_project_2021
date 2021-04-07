#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 11  // nombre de sites (processus) en comptant le processus initiateur
#define M 6   // nombre de fingers

// ! il faut que N > M

/*LES TAGS */
#define TAG_INIT 0
#define TAG_RECH 1
#define TAG_TERM 2

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
int f(int* id_already_used, int p)
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

int cycle_comparator(int a, int b)
{
  // ne pas utiliser avec qsort,
  // -> undefined behavior, may infinite loop
  if (a == b)
    return 0;
  else if (abs(b - a) < (pow(2, M)) / 2)
    return 1;
  else
    return -1;
}

cycle_comparator(a, b);

// qsort( array, element_count, element_size, *compare_function )

/*************Initialisation de la DHT par le processus simulateur*************/

void simulateur(void)
{
  /*initialisation des variables*/
  int id_chord[NB_PROC];  // tableau des identifiants CHORD
  //  int fingers[M][2]; //tableau des M fingers
  // int succ;

  /*Identifiants chord*/

  /*Attribution des identifiants CHORD à chaque processus*/
  /*Sauf le processus 0 qui correspond au simulateur*/
  id_chord[0] = -1;
  for (int p = 1; p < NB_PROC; p++)
  {
    f(&id_chord, p);
  }
  /*Trie dans l'ordre croissant les id_chord pour former l'anneau*/
  qsort(id_chord, NB_PROC - 1, sizeof(int), compare);

  /*Envoi des identifiants chord au pairs du système*/
  for (int p = 1; p < NB_PROC; p++)
  {
    MPI_Send(id_chord[p], 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
  }

  /* ***************************************************
   * Initialisation des fingers pour chaque processus  *
   *************************************************** */

  /* ****************************************************
   * envoie successif des informations à la création de *
   * chaque ensemble de finger                          *
   **************************************************** */
  // pour chaque pair du systeme
  for (int p = 1; p < NB_PROC; p++)
  {
    /* ***************************************************
     * tableau des M fingers                             *
     ************************************************** */
    int fingers[M][2];
    /* ***************************************************
     *  finger[0] = successeur, temp_rank                *
     *  est le rank mpi du successeur                    *
     *************************************************** */

    int idChord = id_chord[p];
    int rank;

    for (int j = 0; j < M; j++)  // pour chaque finger du pair p
    {
      /* ***************************************************
       *                clé                                *
       *************************************************** */
      int cle = (idChord + pow(2, j)) % pow(2, M);
      for (int i = 1; i < NB_PROC; i++)
      {
        if (id_chord[i] >= cle)
        {
          /* ***************************************************
           *                MPI Rank                           *
           *************************************************** */
          fingers[j][0] = rank;
          /* ***************************************************
           *                ID_CHORD                           *
           *************************************************** */
          fingers[j][1] = id_chord[rank];  // le responsable de la cle
          break;
        }
      }  // fin recherche pair associé au finger

    }  // fin for each finger du pair p

    // Envoie du tableau fingers au processus p
    MPI_Send(&fingers, M * 2, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);

  }  // fin for each processus

  /*------------------Fin de l'initialisation du système------------------*/

  /* QUESTION 3 : le simulateur
   *  tire aleatoirement un id pair (verifier son existence)
   *                     une clé de donnée
   *  envoi d'un message à ce pair pour chercher le responsable de cette donnée
   *  Attente de la réponse de la pair le responsable de cette donnée
   *  Progration du message de terminaison à tous les processus
   **/

  /*Tire aleatoirement un id pair existant*/
  srand(time(NULL));
  int alea_pair = 1 + rand() % (NB_PROC + 1);
  /*Tirage aleatoire d'une clé de donnée*/
  int alea_donnee = rand() % pow(2, M);

  /*Envoie d'un message à ce pair pour chercher le responsable de cette donnée
   */
  MPI_Send(alea_donnee, 1, MPI_INT, TAG_RECH, alea_pair, MPI_COMM_WORLD);

  /*Attente de la réponse de la pair le responsable de cette donnée*/
  int responsable;
  MPI_Recv(&responsable, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
           MPI_COMM_WORLD, &status);
  printf("Le responsable de la donnée %d est (<P%d>, <Pair_%d>)\n", alea_donnee,
         status.MPI_SOURCE, responsable);

  /*Progration du message de terminaison à tous les processus*/
  for (int p = 1; p < NB_PROC; p++)
  {
    // le contenu du message n'a pas d'importance, on ne recupere pas de valeurs
    // lors de la terminaison (pour le moment)
    MPI_Send(&p, 1, MPI_INT, p, TAG_TERM, MPI_COMM_WORLD);
  }

  printf("Fin du simulateur\n");
}  // fin simulateur

/*---------------------- Fin de la fonction simulateur -----------------------*/

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