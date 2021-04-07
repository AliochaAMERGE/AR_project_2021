/***** a ne pas copier ****/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define NB_PROC 11
#define M 6

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

/*** fin a ne pas copier ***/

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
  /*Mettre dans l'ordre croissant les id_chord*/
  qsort(id_chord, NB_PROC - 1, sizeof(int), compare);

  /*Initialisation des fingers pour chaque processus*/
  // envoie successif des informations à la création de
  // chaque ensemble de finger
  for (int p = 1; p < NB_PROC; p++)
  {
    int fingers[M][2];  // tableau des M fingers

    // finger[0] = successeur, temp_rank est le rank mpi du successeur
    int temp_rank = (p + 1) % NB_PROC;
    temp_rank = temp_rank > 0 ? temp_rank : temp_rank + 1;
    fingers[0][0] = temp_rank; //mpi rank
    fingers[0][1] = id_chord[temp_rank]; // id chord

    for (int f = 1; f < M; f++)
    {
      // en defini M-1 finger, répartis entre p+1 et (p+(N/2)) % N (au plus la
      // moitié des voisins)

      fmax = f+(NB_PROC/2);

      // tirer la plage de valeurs des fingers :
      // soit p le site courant, nous voulons tirer une valeurs entre p et p+(N/2) en respectant l'ordre cyclique
      // nous prenons la valeurs de p, et de p+(N/2) appelons les n et nmax
      // tirons une valeurs aléatoire dans [n, nmax[ appelons la nrand
      // vérifions quel site s'occupe de nrand (avec app() par exemple) appelons le f -> on recupere son MPI_RANK & id_chord
      // ajoutons f dans notre liste de finger, ajoutons son MPI_rank
      // ? OK ?


      int temp_id = 0;
      fingers[0][0] = temp_id > 0 ? temp_id : temp_id + 1;

      fingers[f][0] = 0;
    }
    // MPI_Send();
  }
}
/*
tu es sur le site N
    : d id p tu veux son successeur(MPI_rank) tu
      fais(p + 1 % N) si id du finger < p alors tu fais + 1 quand tu l ajoutes
                                            N site

                                            p =
    N tu veux successeur tu fais(p + 1) % N->ça te donne 0 0 <
    p->+ 1 et tu obtiens successeur = 1

    c est pas beau mais ça résoud le probleme oui,
                                            il y a une autre solution
*/