#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "./include/header.h"

#define NB_PROC 11       // inconnu des pairs
#define N (NB_PROC - 1)  // inconnu des pairs
#define M 6

#define NIL -1

/* TAGS */
#define TAG_INIT 0       // initialisation de la DHT
#define TAG_INSERTION 1  // insertion du nouveau pair
#define TAG_RECHERCHE 2  // recherche d'un pair

/* les variables globales */
MPI_Status status;
int rang;
int id_chord;
// [0] : MPI_Rank ~ [1] : Id_chord
int fingers[M][2];
int inverse[N][N][2];
int* C;  // les données gérée par le site courant
// Ces dernieres ne sont pas gérée pour le moment

/*variables globales en plus pour l'insertion d'un pair*/
int id_chord_responsable = -1;
int rang_responsable = -1;

/* ******************* Fonction d'appartenance à ]a,b] ***********************
 *               app vérifie si la clé k appartient à                         *
 *               l'intervalle ]a,b]                                           *
 **************************************************************************** */

int app(int k, int a, int b) {
  if (a < b) return ((k > a) && (k <= b));
  return (((k >= 0) && (k <= b)) || ((k > a) && (k <= pow(2, M))));
}

/***************************** Fonctions de hachage **************************/

/* ***************************************************************************
 *             f retourne aléatoirement un identifiant CHORD unique           *
 *             parmis l'ensemble des pairs du système                         *
 **************************************************************************** */

// id_already_used : contient les idChord pour les p-1 ranks, p : rank
int f(int* id_already_used, int p) {
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
 *            g retourne aléatoirement un identifiant unique pour une donnée  *
 *            (clés) parmis l'ensemble des données du système                 *
 **************************************************************************** */

int g(int value) { return value % (int)(pow(2, M) - 1); }

/****************** Fonction utilitaire pour le tri *************************/

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

/*************************** Fonctions de recherche ***************************/

int find(int arg_id_chord) {
  for (int i = M - 1; i >= 0; i--) {
    if (app(arg_id_chord, fingers[i][1], id_chord)) {
      return fingers[i][0];
    }
  }
  return NIL;
}

int find_inverse(int arg_id_chord) {
  int temp = inverse[0][0];

  for (int i = 1; i <= N; i++) {
    if (app(arg_id_chord, id_chord, inverse[i][1])) {  // ???? a verifier demain
      if (app(arg_id_chord, inverse[i][1], temp)) {    // ???? a verifier demain
        temp = inverse[i][0];
      }
    }
  }
  return temp;
}

/************ Initialisation de la DHT par le processus simulateur ************/

void simulateur(void) {
  /* initialisation des variables */
  // tableau des identifiants CHORD {excluant le simulateur et le nouveau pair}
  int id_chord[N - 1];

  int inverse[N][N][2];

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      inverse[i][j][0] = -1;  // Rang_MPI
      inverse[i][j][1] = -1;  // id_chord
    }
  }

  /* Attribution des identifiants CHORD à chaque processus */
  /* Sauf le processus 0 qui correspond au simulateur */

  for (int p = 0; p < N - 1; p++) {
    id_chord[p] = f(id_chord, p);
  }

  /* Trie dans l'ordre croissant les id_chord afin de former l'anneau */
  qsort(id_chord, N - 1, sizeof(int), compare);
  printf("(rang, pair) : ");
  for (int p = 0; p < N - 1; p++) {
    // affichage des id_chord de chaque proche
    printf("(%d, %d)\t", p, id_chord[p]);
  }
  printf("\n");
  /* Envoi des identifiants chord au pairs du système */
  for (int p = 0; p < N - 1; p++) {
    MPI_Send(&id_chord[p], 1, MPI_INT, p + 1, TAG_INIT, MPI_COMM_WORLD);
  }

  /* ***************************************************
   * Initialisation des fingers pour chaque processus  *
   *************************************************** */

  /* envoie successif des informations à la création de *
   * chaque ensemble de finger   */

  // pour chaque pairs du systeme (simulateur exclu)
  for (int p = 0; p < N - 1; p++) {
    /* tableau des M fingrs */
    int fingers[M][2];

    int idChord = id_chord[p];

    printf("%2d > ", id_chord[p]);

    // pour chaque finger du pair p
    for (int j = 0; j < M; j++) {
      /* clé */
      int cle = (idChord + (int)pow(2, j)) % ((int)pow(2, M));
      int ok = 0;
      // recherche pair assicué au finger
      for (int i = 0; i < N - 1; i++) {
        if (id_chord[i] >= cle) {
          // table des fingers
          fingers[j][0] = i;            // MPI RANK
          fingers[j][1] = id_chord[i];  // ID_CHORD
          // table inverse
          inverse[i][p][0] = p;            // MPI RANK
          inverse[i][p][1] = id_chord[p];  // ID_CHORD

          ok = 1;
          break;
        }
      }  // fin recherche pair associé au finger
      if (!ok) {
        fingers[j][0] = 1;
        fingers[j][1] = id_chord[1];

        inverse[1][p][0] = p;
        inverse[1][p][1] = id_chord[p];
      }
      printf("%5d", fingers[j][1]);

    }  // fin for each finger du pair p
    printf("\n");

    // Envoie du tableau fingers au processus p
    MPI_Send(fingers, M * 2, MPI_INT, p + 1, TAG_INIT, MPI_COMM_WORLD);

  }  // fin for each processus

  // envoie des inverses à chaque pairs
  for (int p = 0; p < N - 1; p++) {
    MPI_Send(inverse[p + 1], (N)*2, MPI_INT, p + 1, TAG_INIT, MPI_COMM_WORLD);
  }

  /* création du nouveau pair */

  int nouveau_pair_id_chord = f(id_chord, nouveau_pair_id_chord);
  int nouveau_pair_rang = NB_PROC;
  int responsable_nouveau = 1 + rand() % (N - 1);

  /* Le contenu du message
   * messages[0] : id_chord du pair qui s'insère
   * messages[1] : id_chord du pair responsable
   * messages[2] : rang MPI du pair reponsable
   */
  int messages[3] = {nouveau_pair_rang, id_chord[responsable_nouveau],
                     responsable_nouveau};
  MPI_Send(messages, 3, MPI_INT, nouveau_pair_rang, TAG_INSERTION,
           MPI_COMM_WORLD);

}  // fin simulateur

/********** reception des id_chord et de la liste des fingers  ************/
/*                  sauf pour le noeud que nous insérons                  */

void init() {
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  switch (status.MPI_TAG) {
    case TAG_INSERTION:

      // initialisation du nouveau pair

      int message[3];
      MPI_Recv(&message, 3, MPI_INT, 0, TAG_INSERTION, MPI_COMM_WORLD, &status);
      id_chord = message[0];
      // id_chord et rang MPI de l'unique pair à qui il peut envoyer et recevoir
      // des messages initialement
      id_chord_responsable = message[1];
      rang_responsable = message[2];

      // Le nouveau pair recherche son "successeur"
      int msg_rech[2] = {id_chord, rang};
      MPI_Send(msg_rech, 2, MPI_INT, rang_responsable, TAG_RECHERCHE,
               MPI_COMM_WORLD);

      MPI_Recv(msg_rech, 2, MPI_INT, rang_responsable, TAG_RECHERCHE,
               MPI_COMM_WORLD, &status);
      id_chord_responsable = msg_rech[0];
      rang_responsable = msg_rech[1];

      // debut de la recherche des fingers

      break;
    default:  // par défaut il s'agit du tag TAG_INIT

      // initialisation des pairs deja present dans la DHT

      MPI_Recv(&id_chord, 1, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
      MPI_Recv(fingers, M * 2, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &status);
      MPI_Recv(inverse, (N * 2), MPI_INT, MPI_ANY_SOURCE, TAG_INIT,
               MPI_COMM_WORLD, &status);
      break;
  }
}

void receive(void) {
  int message[2];                 /* le contenu du message recu */
  int nouveau_pair_id_chord = -1; /* id_chord du nouveau pair */
  int nouveau_pair_rang = -1;     /* rang du nouveau pair */
  int next;                       /* pair suivant pour la recherche */

  while (1) {
    MPI_Recv(message, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);

    switch (status.MPI_TAG) {
      case TAG_RECHERCHE:
        // prévenir le noeud de sa responsabilité par rapport au nouveau pair
        nouveau_pair_id_chord = message[0];
        nouveau_pair_rang = message[1];

        if (nouveau_pair_id_chord > id_chord) {
          // recherche dans les fingers le plus grand finger inferieur à
          // l'identifiant chord du nouveau pair

          next = find(nouveau_pair_id_chord);
          if(next == NIL){
            next = fingers[0][0];
          }
        } else if (nouveau_pair_id_chord < id_chord) {
          // recherche dans les inverses le plus petit
          next = find_inverse(nouveau_pair_id_chord);
        }

        MPI_Send(message, un, truc, en, plus);

        // nous cherchons le successeur du nouveau noeud (le responsable de son
        // id_chord)
        break;

      default:
        break;
    }
  }
}

int main(int argc, char* argv[]) {
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
  } else {
    init();
    receive();
  }
  MPI_Finalize();
  return 0;
}
