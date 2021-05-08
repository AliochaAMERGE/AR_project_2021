#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "./include/header.h"

#define NB_PROC 11     // inconnu des pairs
#define N NB_PROC - 1  // inconnu des pairs
#define M 6

/* TAGS */
#define TAG_INIT 0
#define TAG_IN 1        // message de type IN (retour)
#define TAG_OUT 2       // message de type OUT
#define TAG_COLLECTE 3  // pour la collecte des id chord

/*Variables Globales*/
MPI_Status status;
int id_chord;     // identifiant chord du processus courant
int rang;         // MPI_rank
int right, left;  // voisin droit et gauche
int initiateur;   // pour determiner si le processus est initiateur ou non
int size;         // taille de l'anneau (inconnu au départ)

int nb_IN = 0;  // nombre de messages IN recu

int k = 0;            // nombre d'étape lors de l'élection
int *id_chord_table;  // tableau des id chord

/* ***************************************************************************
 *             f retourne aléatoirement un identifiant CHORD unique           *
 *             parmis l'ensemble des pairs du système                         *
 **************************************************************************** */

int f(int *id_already_used, int p) {
  // id_already_used : contient les idChord pour les p-1 ranks, p : rank
  int alea_chord;
  int used;
  do {
    used = 1;
    alea_chord = (rand() % ((int)pow(2, M)));  // retire une valeur aléatoire

    for (int i = 0; i < p; i++) {
      if (id_already_used[i] == alea_chord) {
        used = 0;
      }
    }
  } while (!used);

  return alea_chord;
}

/* ***************************************************************************
 *                           processus initiateur                            *
 *         distribue les identifiants chord aux différents processus         *
 *****************************************************************************/

void simulateur(void) {
  // Réalisé par le processus 0 (exterieur à l'anneau)
  // distibue les identifiants chord à chaque noeuds
  id_chord_table = malloc(sizeof(int) * NB_PROC);
  id_chord_table[0] = -1;
  int initiateurs[N];
  int bool = 0, temp;

  do {
    for (int p = 1; p < NB_PROC; p++) {
      temp = rand() % 2;
      initiateurs[p - 1] = temp;
      if (temp == 1) {
        bool = 1;
      }
    }
  } while (!bool);  // permet d'eviter l'absence d'initiateur

  for (int p = 1; p < NB_PROC; p++) {
    id_chord_table[p] = f(id_chord_table, p);
    MPI_Send(&id_chord_table[p], 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
    left = ((p - 1) == 0) ? N : (p - 1);
    MPI_Send(&left, 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
    right = ((p + 1) > N) ? 1 : (p + 1);
    MPI_Send(&right, 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
    MPI_Send(&initiateurs[p - 1], 1, MPI_INT, p, TAG_INIT, MPI_COMM_WORLD);
  }
}

/* ***************************************************************************
 *      - initialisation des noeuds de l'anneau                              *
 *      - récupèration de leurs id_chord (par le processus initiateur)       *
 *      - initialisation des voisins gauches et droites                      *
 *      - determination du fait qu'il soit initiateur ou non                 *
 * ***************************************************************************/

void init(void) {
  MPI_Recv(&id_chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  MPI_Recv(&left, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  MPI_Recv(&right, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  MPI_Recv(&initiateur, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
  if (initiateur) {
    initier_etape();
  }
}

/* ***************************************************************************
 *                         lancement de l'étape k+1                          *
 *                envoie un message OUT à gauche et à droite                 *
 *****************************************************************************/

void initier_etape(void) {
  // k : etape
  // 2^k : distance
  if (initiateur) {
    nb_IN = 0;

    /* Le contenue du message :
     * message[0] = MPI_rank
     * message[1] = distance (2^k)
     */

    int message[2] = {id_chord, (int)pow(2, k)};
    // envoie d'un message de type OUT au voisin de droit et gauche
    MPI_Send(message, 2, MPI_INT, right, TAG_OUT, MPI_COMM_WORLD);
    MPI_Send(message, 2, MPI_INT, left, TAG_OUT, MPI_COMM_WORLD);
    k++;
  }
}

/* ****************************************************************************
 *               receive permet de gérer la réception                         *
 *               des messages en fonction de leurs tags                       *
 *****************************************************************************/
void receive(void) {
  /*
   * message[0] : id initiateur
   * message[1] : TTL (distance)
   */
  int message[2];

  for (;;) {
    // MPI_Probe() permet de savoir par avance le type et la taille du message
    // que nous recevrons sans le consommer

    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    switch (status.MPI_TAG) {
      case TAG_IN:
        // Attente d'un message de TAG_IN
        MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, TAG_IN, MPI_COMM_WORLD,
                 &status);
        if (message[0] != id_chord) {
          // si le message n'est pas revenu à bon port
          if (status.MPI_SOURCE == right) {
            // on transmet le message au voisin gauche
            MPI_Send(message, 2, MPI_INT, left, TAG_IN, MPI_COMM_WORLD);
          } else {
            // resp. à droite
            MPI_Send(message, 2, MPI_INT, right, TAG_IN, MPI_COMM_WORLD);
          }
        } else {
          nb_IN++;
          if (nb_IN == 2) {
            // si deux messages de type IN ont été receptionné
            // on passe à l'étape suivante (k + 1)
            initier_etape();
          }
        }
        break;

      case TAG_OUT:
        // Attente d'un message de TAG_OUT
        MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, TAG_OUT, MPI_COMM_WORLD,
                 &status);
        if (!(initiateur) || (message[0] > id_chord)) {
          // si non initiateur OU c'est quelqu'un de plus fort que moi
          initiateur = 0;
          if (message[1] > 1) {
            // si la distance pour les messages de type OUT n'est pas encore
            // arrivé au bout
            message[1] -= 1;  // diminue la distance de 1
            if (status.MPI_SOURCE == right) {
              // s'il s'agit d'un message venant du voisin de droite
              // transmet le message de type OUT au voisin de gauche
              MPI_Send(message, 2, MPI_INT, left, TAG_OUT, MPI_COMM_WORLD);
            } else {
              // transmet de le message de type OUT au voisin de droit
              MPI_Send(message, 2, MPI_INT, right, TAG_OUT, MPI_COMM_WORLD);
            }
          } else {  // la distance est atteinte
            // le type de message change en IN et fait demi tour
            message[1] = 0;
            if (status.MPI_SOURCE == right) {
              // s'il s'agit d'un message venant du voisin de droite
              // transmet le message de type IN au voisin de gauche
              MPI_Send(message, 2, MPI_INT, right, TAG_IN, MPI_COMM_WORLD);
            } else {
              // transmet de le message de type IN au voisin de droit
              MPI_Send(message, 2, MPI_INT, left, TAG_IN, MPI_COMM_WORLD);
            }
          }
        } else {
          if ((message[0] == id_chord)) {
            // j'ai gagné l'élection YOUHOU :)
            // Nous pouvons donc déterminé la taille de l'anneau en fonction du
            // nombre d'étapes
            size = (int)pow(2, k - 1) - message[1] + 2;

            // Le tableau est ordonné en fonction de MPI_Rank
            id_chord_table = malloc(size * sizeof(int));
            id_chord_table[0] = -1;  // simulateur
            // ajout de son id_chord dans le tableau à l'indice rang
            id_chord_table[rang] = id_chord;
            // envoi au voisin de droit le tableau pour qu'il puisse ajouter son
            // id_chord
            MPI_Send(id_chord_table, size, MPI_INT, right, TAG_COLLECTE,
                     MPI_COMM_WORLD);
            // attente du tableau rempli de tous les id_Chord
            MPI_Recv(id_chord_table, size, MPI_INT, left, TAG_COLLECTE,
                     MPI_COMM_WORLD, &status);
            // envoi du tableau rempli au voisin de droite
            MPI_Send(id_chord_table, size, MPI_INT, right, TAG_COLLECTE,
                     MPI_COMM_WORLD);
            // le message est revenu, tout le monde connait les id_chord et donc
            // sa table de fingers
            MPI_Recv(id_chord_table, size, MPI_INT, left, TAG_COLLECTE,
                     MPI_COMM_WORLD, &status);

            // calcul de ma table de finger
            fingers_table();
            return;
          }
        }

        break;

      case TAG_COLLECTE:

        /* Collecte des identifiants Chord pour redistribution */
        // récuperation de la taille du message
        MPI_Get_count(&status, MPI_INT, &size);
        id_chord_table = malloc(size * sizeof(int));
        // attente du tableau pour pouvoir indiquer son id_chord
        MPI_Recv(id_chord_table, size, MPI_INT, left, TAG_COLLECTE,
                 MPI_COMM_WORLD, &status);
        // ajout de son id_chord dans le tableau à l'indice rang
        id_chord_table[rang] = id_chord;

        // envoi le tableau modifié au voisin de droite
        MPI_Send(id_chord_table, size, MPI_INT, right, TAG_COLLECTE,
                 MPI_COMM_WORLD);

        // attente du tableau rempli d'id_chord
        MPI_Recv(id_chord_table, size, MPI_INT, left, TAG_COLLECTE,
                 MPI_COMM_WORLD, &status);
        // envoie le tableau rempli d'identifiant chord à tous pour diffusion
        MPI_Send(id_chord_table, size, MPI_INT, right, TAG_COLLECTE,
                 MPI_COMM_WORLD);
        // calcul de ma table de finger
        fingers_table();

        return;
        // sort de receive() puis se termine

      default:
        perror("Erreur dans le switch case : TAG inconnu !");
        break;
    }
  }
}

/****************** Fonction utilitaire pour le tri *************************/
int compare(const void *a, const void *b) {
  int int_a = *((int *)a);
  int int_b = *((int *)b);

  if (int_a == int_b)
    return 0;
  else if (int_a < int_b)
    return -1;
  else
    return 1;
}

/***************** Calcul local de la table de fingers **********************
 *                   puis affichage de cette dernieres                       *
 *****************************************************************************/

void fingers_table(void) {
  int fingers[M];
  int tmp_id_chord[size];
  int cle;
  int ok;

  // Remplissage du tableau temporaire d'identifiant chord
  for (int i = 0; i < size; i++) {
    tmp_id_chord[i] = id_chord_table[i];
  }
  // Trie dans l'ordre croissant des identifiants chord
  qsort(tmp_id_chord, size, sizeof(int), compare);

  /* calcul de la table de finger */

  for (int j = 0; j < M; j++) {
    // calcul de la cle
    cle = (id_chord + (int)pow(2, j)) % ((int)pow(2, M));
    ok = 0;  // utilitaire pour respecter l'ordre cyclique
    for (int p = 1; p < size; p++) {
      if (tmp_id_chord[p] >= cle) {
        // responsable de la cle trouvé
        fingers[j] = tmp_id_chord[p];
        ok = 1;
        break;
      }
    }
    if (!ok) {
      // si le responsable de la cle n'est pas trouve
      // alors il s'agit du premier
      fingers[j] = tmp_id_chord[1];
    }
  }
  /* affichage de la table de fingers */
  usleep(rang * 1000);  // pour garder un affichage cohérent
  usleep(rang * 1000);  // pour garder un affichage cohérent
  usleep(rang * 1000);  // pour garder un affichage cohérent
  printf("\n Fingers table de : %d\n", id_chord);
  for (int t = 0; t < M; t++) {
    printf(" %2d ", fingers[t]);
  }
  printf("\n");
}

/************************************ MAIN ************************************/

int main(int argc, char *argv[]) {
  int nb_proc;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  if (nb_proc != N + 1) {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &rang);

  srand(getpid());

  if (rang == 0) {
    simulateur();
    // rien d'autre à faire P0 se termine
  } else {
    // recupere ses fingers et id_chord
    init();
    // boucle sur la réception de message
    receive();
  }
  // free apres malloc
  free(id_chord_table);
  MPI_Finalize();
  return 0;
}