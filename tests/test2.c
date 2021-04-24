#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NB_PROC 11
#define N (NB_PROC - 1)
#define M 6
#define TAG_INIT 0

MPI_Status status;
int id_chord;
// [0] : MPI_Rank ~ [1] : Id_chord
int fingers[M][2];

int compare(const void* a, const void* b) {
  int int_a = *((int*)a);
  int int_b = *((int*)b);

  if (int_a == int_b)
    return 0;
  else if (int_a < int_b)
    return -1;
  else
    return 1;
}

int f(int* id_already_used, int p) {
  srand(time(NULL));
  int alea_chord;
  // on a 2^M-1 valeurs possibles
  int i = 0;
  do {
    alea_chord = (rand() % ((int)pow(2, M)));  // retire une valeur aléatoire
    if (id_already_used[i] == alea_chord) {
      i = 0;  // repart du début
    } else {
      i++;
    }
  } while (i < p);

  // id_already_used[p] = alea_chord;

  return alea_chord;
}

void simulateur(void) {
  /*initialisation des variables*/
  int id_chord[NB_PROC];  // tableau des identifiants CHORD
  //  int fingers[M][2]; //tableau des M fingers
  // int succ;

  /*Identifiants chord*/

  /*Attribution des identifiants CHORD à chaque processus*/
  /*Sauf le processus 0 qui correspond au simulateur*/
  id_chord[0] = -1;
  for (int p = 1; p < NB_PROC; p++) {
    id_chord[p] = f(id_chord, p);
  }
  /*Trie dans l'ordre croissant les id_chord pour former l'anneau*/
  qsort(id_chord, NB_PROC - 1, sizeof(int), compare);

  /*Envoi des identifiants chord au pairs du système*/
  for (int p = 1; p < NB_PROC; p++) {
    printf("[%d] = %d \n", p, id_chord[p]);
    MPI_Send(&id_chord[p], 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
  }

  /* ***************************************************
   * Initialisation des fingers pour chaque processus  *
   *************************************************** */

  /* ****************************************************
   * envoie successif des informations à la création de *
   * chaque ensemble de finger                          *
   **************************************************** */
  // pour chaque pair du systeme
  for (int p = 1; p < NB_PROC; p++) {
    /* ***************************************************
     * tableau des M fingers                             *
     ************************************************** */
    int fingers[M][2];
    /* ***************************************************
     *  finger[0] = successeur, temp_rank                *
     *  est le rank mpi du successeur                    *
     *************************************************** */

    int idChord = id_chord[p];

    // pour chaque finger du pair p
    for (int j = 0; j < M; j++) {
      /* clé */
      int cle = (idChord + (int)pow(2, j)) % ((int)pow(2, M));
      for (int i = 1; i < NB_PROC; i++) {
        if (id_chord[i] >= cle) {
          // * MPI RANK
          fingers[j][0] = i;
          // * ID_CHORD
          // le responsable de la cle
          fingers[j][1] = id_chord[i];
          break;
        }
      }  // fin recherche pair associé au finger

    }  // fin for each finger du pair p

    // Envoie du tableau fingers au processus p
    MPI_Send(fingers, M * 2, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);

  }  // fin for each processus

  /*------------------Fin de l'initialisation du système------------------*/

  printf("Fin du simulateur\n");
}  // fin simulateur

void init() {
  MPI_Recv(&id_chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  MPI_Recv(fingers, M * 2, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  // printf("fin init\n");
}

int main(int argc, char* argv[]) {
  int nb_proc, rang;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  if (nb_proc != N + 1) {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &rang);

  if (rang == 0) {
    simulateur();

    /*Tire aleatoirement un id pair existant*/
    srand(time(NULL));
    int alea_pair = 1 + rand() % (NB_PROC);  // MPI_rank
    /*Tirage aleatoire d'une clé de donnée*/
    int alea_key = 1 + rand() % ((int)pow(2, M) - 1);

    printf("Recherche de : %d\n", alea_key);
    printf("Initiateur : %d\n", alea_pair);

  } else {
    init();

    MPI_finalize();
    return 0;
  }
